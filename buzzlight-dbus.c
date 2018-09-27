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
#include <systemd/sd-bus.h>

#ifndef BACKLPATH
#define BACKLPATH "/sys/class/backlight/intel_backlight"
#endif /* BUZZ_DEBUG */

#define GUARD(p) if(!(p)) return -1;
#define OK 0
#define min(a, b) (a < b ? a : b)

struct backl
{
	uint32_t maxbrg;
	uint32_t stored;
	bool t:1;
	bool on:1;
};

static struct backl *bl;

static int parse10n_uint32(const char *s, size_t len, unsigned *n)
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


static int readval(const char *path, unsigned *n)
{
	char buf[32];
	int fd = open(path, O_RDONLY);
	GUARD(fd > -1);

	read(fd, buf, sizeof(buf));
	close(fd);

	GUARD(parse10n_uint32(buf, strlen(buf), n) == OK);
	return OK;
}

static int get_brightness(uint32_t *des)
{
	GUARD(readval(BACKLPATH "/brightness", des) == OK);
	return OK;
}

static int init_backl(struct backl *bl)
{
	GUARD(readval(BACKLPATH "/brightness", &bl->stored) == OK);
	GUARD(readval(BACKLPATH "/max_brightness", &bl->maxbrg) == OK);
	bl->on = 1;
	bl->t = 0;
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
static PROPERTY_GETTER(property_get_stored_brightness, "u", bl->stored);
static PROPERTY_GETTER(property_get_transitioning, "b", bl->t);

#undef PROPERTY_GETTER

static int property_get_brightness(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *reply,
				   void *userdata,
				   sd_bus_error *error)
{
	uint32_t val;

	assert(bus);
	assert(reply);

	if(get_brightness(&val) != OK)
		return sd_bus_message_append(reply, "u", -1);
	return sd_bus_message_append(reply, "u", val);
}

static int set_brightness(uint32_t val)
{
	char buf[32];
	int fd;

	fd = open(BACKLPATH "/brightness", O_WRONLY);
	GUARD(fd > 0);
	ftruncate(fd, 0);

	val = min(val, bl->maxbrg);
	snprintf(buf, sizeof(buf), "%u\n", val);
	GUARD(write(fd, buf, strlen(buf)) > 0);
	close(fd);

	if(bl->on && val == 0)
		bl->on = 0;
	if(!bl->on && val > 0)
		bl->on = 1;

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
	uint32_t val;

	assert(bus);
	assert(value);

	r = sd_bus_message_read(value, "u", &val);
	if(r < 0)
		return r;
	set_brightness(val);

	return 0;
}

static int frame_duration_ms(int millisec)
{
	return ((float)millisec / 1000.0) * 60;
}

static int smooth_set(uint32_t from, uint32_t to, uint32_t millisec)
{
	float d;
	int dur, i;
	struct timespec req = {
		.tv_sec = 0,
		.tv_nsec = (long)1000000000.0 / 60.0,
	};

	dur = frame_duration_ms(millisec);
	d = ((float)to - (float)from) / (float)dur;

	bl->t = 1;
	for(i = 0; i < dur; i++) {
		if(set_brightness(from + (d * i)) < 0)
			break;
		if(nanosleep(&req, NULL) < 0)
			break;
	}
	bl->t = 0;
	GUARD(set_brightness(to) > -1);
	GUARD(i == dur);

	return OK;
}

static int method_set_brightness_smooth(sd_bus_message *m,
					void *userdata,
					sd_bus_error *error)
{
	uint32_t val, millisec, brg;
	int store, r;

	assert(m);
	r = sd_bus_message_read(m, "uub", &val, &millisec, &store);
	if(r < 0)
		return r;

	val = min(val, bl->maxbrg);
	GUARD(get_brightness(&brg) == OK);
	smooth_set(brg, val, millisec);

	if(store)
		bl->stored = val;

	return sd_bus_reply_method_return(m, "");
}

static int method_toggle_backlight(sd_bus_message *m,
				   void *userdata,
				   sd_bus_error *error)
{
	uint32_t val, brg, millisec;
	int r;

	assert(m);
	r = sd_bus_message_read(m, "u", &millisec);
	if(r < 0)
		return r;

	val = bl->on ? 0 : bl->stored;
	GUARD(get_brightness(&brg) == OK);
	GUARD(smooth_set(brg, val, millisec) == OK);

	return sd_bus_reply_method_return(m, "b", bl->on);
}

static const sd_bus_vtable vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_PROPERTY("Transitioning", "b",
			property_get_transitioning,
			0,
			0),
	SD_BUS_PROPERTY("MaxBrightness", "u",
			property_get_max_brightness,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("StoredBrightness", "u",
			property_get_stored_brightness,
			0,
			0),
	SD_BUS_WRITABLE_PROPERTY("BacklightEnabled", "b",
				 property_get_backlight_enabled,
				 property_set_backlight_enabled,
				 0,
				 SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_WRITABLE_PROPERTY("Brightness", "u",
				 property_get_brightness,
				 property_set_brightness,
				 0,
				 SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("SetBrightnessSmooth", "uub", "",
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

	r = sd_bus_add_object_vtable(*bus, slot,
				     "/org/git/felixnaredi/buzzlight",
				     "org.git.felixnaredi.buzzlight",
				     vtable, NULL);
	if(r < 0)
		return r;

	r = sd_bus_request_name(*bus, "org.git.felixnaredi.buzzlight", 0);
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
	sd_bus *bus = NULL;
	sd_bus_slot *slot = NULL;
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
