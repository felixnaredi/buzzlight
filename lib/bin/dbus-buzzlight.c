// buzzlight-dbus.c
//

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <systemd/sd-bus.h>

#ifndef BACKLPATH
#define BACKLPATH "/sys/class/backlight/intel_backlight"
#endif /* BUZZ_DEBUG */

#define GUARD(p) do { if(!(p)) return -1; } while(0)
#define OK 0
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

struct backl
{
	int32_t maxbrg;
	int32_t stored;
	bool ready:1;
	bool on:1;
};

struct ssetd
{
	pthread_t thread;
	int32_t from;
	int32_t to;
	uint32_t usec;
	bool store:1;
};

static struct backl *bl = NULL;

static int parse10n_uint32(const char *s, size_t len, int *n)
{
	size_t i;
	char *ofs;
	char cs[] = "0123456789";

	*n = 0;
	for(i = 0; i < len; i++) {
		ofs = strchr(cs, s[i]);
		if(!ofs) {
			GUARD(s[i] == '\0' || s[i] == '\n');
			return OK;
		}
		*n = (*n * 10) + ofs - cs;
	}
	return -1;
}


static int readval(const char *path, int *n)
{
	char buf[32];
	int fd = open(path, O_RDONLY);
	GUARD(fd > -1);

	read(fd, buf, sizeof(buf));
	close(fd);

	GUARD(parse10n_uint32(buf, strlen(buf), n) == OK);
	return OK;
}

static int get_brightness(int32_t *des)
{
	GUARD(readval(BACKLPATH "/brightness", des) == OK);
	return OK;
}

static int init_backl(struct backl *bl)
{
	GUARD(readval(BACKLPATH "/brightness", &bl->stored) == OK);
	GUARD(readval(BACKLPATH "/max_brightness", &bl->maxbrg) == OK);
	bl->on = !!bl->stored;
	bl->ready = 1;
	return OK;
}

#define PROPERTY_GETTER(func, type, value)				\
	int func(sd_bus *bus,						\
		 const char *path,					\
		 const char *interface,					\
		 const char *property,					\
		 sd_bus_message *reply,					\
		 void *userdata,					\
		 sd_bus_error *error)					\
	{								\
		assert(bus);						\
		assert(reply);						\
		return sd_bus_message_append(reply, type, value);	\
	}

static PROPERTY_GETTER(property_get_backlight_enabled, "b", bl->on);
static PROPERTY_GETTER(property_get_max_brightness, "u", bl->maxbrg);
static PROPERTY_GETTER(property_get_stored_brightness, "i", bl->stored);
static PROPERTY_GETTER(property_get_ready, "b", bl->ready);

#undef PROPERTY_GETTER

static int property_get_brightness(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *reply,
				   void *userdata,
				   sd_bus_error *error)
{
	int32_t val;

	assert(bus);
	assert(reply);

	if(get_brightness(&val) != OK)
		return sd_bus_message_append(reply, "i", -1);
	return sd_bus_message_append(reply, "i", val);
}

static int set_brightness(int32_t val)
{
	char buf[32];
	int fd;

	GUARD((fd = open(BACKLPATH "/brightness", O_WRONLY)) > 0);
	ftruncate(fd, 0);

	val = min(max(0, val), bl->maxbrg);
	snprintf(buf, sizeof(buf), "%u\n", val);
	GUARD(write(fd, buf, strlen(buf)) > 0);
	close(fd);

	bl->on = val > 0;
	return val;
}

static int property_set_backlight_enabled(sd_bus *bus,
					  const char *path,
					  const char *interface,
					  const char *property,
					  sd_bus_message *value,
					  void *userdata,
					  sd_bus_error *ret_error)
{
	/* Don't use bool when getting bools from message. There is
	 * some optimization happening that crashes the program.
	 */
	int b, r;

	GUARD(bl->ready);

	assert(bus);
	assert(value);

	r = sd_bus_message_read(value, "b", &b);
	if(r < 0)
		return r;
	if(b == bl->on)
		return 0;
	set_brightness(b ? bl->stored : 0);
	bl->on = b;

	return 0;
}

static int property_set_brightness(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *value,
				   void *userdata,
				   sd_bus_error *ret_error)
{
	int r;
	int32_t val;

	GUARD(bl->ready);
	assert(bus);
	assert(value);

	r = sd_bus_message_read(value, "i", &val);
	if(r < 0)
		return r;
	set_brightness(val);

	return 0;
}

static int frame_duration_ms(int millisec)
{
	return ((float)millisec / 1000.0) * 60;
}

static int smooth_set(struct ssetd *sd)
{
	float delta;
	int dur, i;
	struct timespec tns = {
		.tv_sec = 0,
		.tv_nsec = (long)(1000000000.0 / 60.0),
	};

	bl->ready = 0;

	dur = frame_duration_ms(sd->usec);
	delta = ((float)sd->to - (float)sd->from) / (float)dur;

	for(i = 0; i < dur; i++) {
		int32_t v = sd->from + (delta * i);

		if(set_brightness(v) < 0)
			break;
		if(sd->store)
			bl->stored = v;
		if(nanosleep(&tns, NULL) < 0)
			break;
	}
	bl->ready = 1;

	GUARD(set_brightness(sd->to) > -1);
	if(sd->store)
		bl->stored = sd->to;
	GUARD(i == dur);

	return OK;
}

static void *thread_smooth_set(void *sd)
{
	smooth_set(sd);
	free(sd);
	return NULL;
}

static int init_ssetd(struct ssetd *sd,
		      int32_t from,
		      int32_t to,
		      uint32_t usec,
		      bool store)
{
	int r;

	assert(sd);

	*sd = (struct ssetd) {
		.from = from,
		.to = to,
		.usec = usec,
		.store = store,
	};
	r = pthread_create(&(sd->thread), NULL, thread_smooth_set, sd);
	if(r < 0)
		return r;

	return OK;
}

static int method_set_brightness_smooth(sd_bus_message *m,
					void *userdata,
					sd_bus_error *error)
{
	int32_t val, brg;
	uint32_t usec;
	int store, r;
	struct ssetd *sd;

	assert(m);
	GUARD(bl->ready);

	r = sd_bus_message_read(m, "iub", &val, &usec, &store);
	if(r < 0)
		return r;

	GUARD(get_brightness(&brg) == OK);

	sd = malloc(sizeof(struct ssetd));
	GUARD(init_ssetd(sd, brg, val, usec, store) == OK);
	pthread_detach(sd->thread);

	return sd_bus_reply_method_return(m, "");
}

static int method_toggle_backlight(sd_bus_message *m,
				   void *userdata,
				   sd_bus_error *error)
{
	int32_t val, brg;
	uint32_t us;
	int r;
	struct ssetd *sd;

	assert(m);
	GUARD(bl->ready);

	r = sd_bus_message_read(m, "u", &us);
	if(r < 0)
		return r;

	val = bl->on ? 0 : bl->stored;
	GUARD(get_brightness(&brg) == OK);

	sd = malloc(sizeof(struct ssetd));
	GUARD(init_ssetd(sd, brg, val, us, 0) == OK);
	pthread_detach(sd->thread);

	return sd_bus_reply_method_return(m, "b", bl->on);
}

static const sd_bus_vtable vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_PROPERTY("Ready", "b",
			property_get_ready,
			0,
			0),
	SD_BUS_PROPERTY("MaxBrightness", "u",
			property_get_max_brightness,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("StoredBrightness", "i",
			property_get_stored_brightness,
			0,
			0),
	SD_BUS_WRITABLE_PROPERTY("BacklightEnabled", "b",
				 property_get_backlight_enabled,
				 property_set_backlight_enabled,
				 0,
				 SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_WRITABLE_PROPERTY("Brightness", "i",
				 property_get_brightness,
				 property_set_brightness,
				 0,
				 SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("SetBrightnessSmooth", "iub", "",
		      method_set_brightness_smooth,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("ToggleBacklight", "u", "b",
		      method_toggle_backlight,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END,
};

static int add_bus_object(sd_bus **bus, sd_bus_slot **slot)
{
	int r;

	r = sd_bus_default_system(bus);
	if(r < 0)
		return r;

	r = sd_bus_add_object_vtable(*bus,
				     slot,
				     "/git/felixnaredi/buzzlight",
				     "git.felixnaredi.buzzlight",
				     vtable,
				     NULL);
	if(r < 0)
		return r;

	r = sd_bus_request_name(*bus, "git.felixnaredi.buzzlight", 0);
	if(r < 0)
		return r;

	return 0;
}

static int run(sd_bus *bus)
{
	int r;

	assert(bus);

	while(1) {
		r = sd_bus_process(bus, NULL);
		if(r < 0)
			return r;
		if(r > 0)
			continue;
		r = sd_bus_wait(bus, (uint64_t) -1);
		if(r < 0)
			return r;
	}
	return 0;
}

int main(int argc, char **argv)
{
	sd_bus_slot *slot = NULL;
	sd_bus *bus = NULL;
	int r;

	GUARD((bl = malloc(sizeof(struct backl))) != NULL);
	GUARD(init_backl(bl) == OK);

	r = add_bus_object(&bus, &slot);
	if(r < 0) {
		fprintf(stderr, "Failed to add bus object - %s\n", strerror(-r));
		goto finish;
	}

	r = run(bus);
	if(r < 0)
		fprintf(stderr, "Failure during run - %s\n", strerror(-r));
finish:
	sd_bus_slot_unref(slot);
	sd_bus_unref(bus);
	return !(r < 0);
}
