// buzzlight-dbus.c
//

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <systemd/sd-bus.h>

#ifdef DEBUG
#define BACKLPATH "debug/backlight"
#else
#define BACKLPATH "/sys/class/backlight/intel_backlight"
#endif /* DEBUG */

#define OK 0
#define GUARD(p) if(!(p)) return -1;
#define min(a, b) (a < b ? a : b)
#define abs(a) (a < 0 ? -a : a)

struct backl
{
	uint32_t brg;
	uint32_t maxbrg;
	bool t:1;
	bool on:1;
};

static struct backl bl;

static int parse10n_uint32(const char *s, size_t len, unsigned *n)
{
	size_t i;
	char *ofs;
	char cs[] = "0123456789";

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

	GUARD(!parse10n_uint32(buf, sizeof(buf), n));
	return OK;
}

static int init_backl(struct backl *bl)
{
	GUARD(readval(BACKLPATH "/brightness", &bl->brg) == OK);
	GUARD(readval(BACKLPATH "/max_brightness", &bl->maxbrg) == OK);
	bl->on = 1;
	bl->t = 0;
	return OK;
}

static int property_get_backlight_enabled(sd_bus *bus,
					  const char *path,
					  const char *interface,
					  const char *property,
					  sd_bus_message *reply,
					  void *userdata,
					  sd_bus_error *error)
{
	assert(bus);
	assert(reply);

	return sd_bus_message_append(reply, "b", bl.on);
}


static int property_get_maxbrightness(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *reply,
				      void *userdata,
				      sd_bus_error *error)
{
	assert(bus);
	assert(reply);

	return sd_bus_message_append(reply, "u", bl.maxbrg);
}

static int property_get_brightness(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *reply,
				   void *userdata,
				   sd_bus_error *error)
{
	assert(bus);
	assert(reply);

	return sd_bus_message_append(reply, "u", bl.brg);
}

static int property_get_transitioning(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *reply,
				      void *userdata,
				      sd_bus_error *error)
{
	assert(bus);
	assert(reply);

	return sd_bus_message_append(reply, "b", bl.t);
}


static int set_brightness(uint32_t val)
{
	char buf[32];
	int fd;

	fd = open(BACKLPATH "/brightness", O_WRONLY);
	GUARD(fd > 0);
	ftruncate(fd, 0);

	snprintf(buf, sizeof(buf), "%u\n", min(val, bl.maxbrg));
	GUARD(write(fd, buf, strlen(buf)) > 0);
	close(fd);

	return min(val, bl.maxbrg);
}

static int property_set_backlight_enabled(sd_bus *bus,
					  const char *path,
					  const char *interface,
					  const char *property,
					  sd_bus_message *value,
					  void *userdata,
					  sd_bus_error *ret_error)
{
	int r;
	bool b;

	assert(bus);
	assert(value);

	r = sd_bus_message_read(value, "b", &b);
	if(r < 0)
		return r;
	if(b == bl.on)
		return 0;
	set_brightness(b ? bl.brg : 0);
	bl.on = b;

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
	GUARD(val = set_brightness(val));
	bl.brg = val;

	return 0;
}

static int frame_duration_ms(int millisec)
{
	return ((float)millisec / 1000.0) * 60;
}

static int method_set_brightness_smooth(sd_bus_message *m,
					void *userdata,
					sd_bus_error *error)
{
	uint32_t val, ms, i;
	int oldbrg, frame_dur, r;
	float d;
	struct timespec req = {
		.tv_sec = 0,
		.tv_nsec = (long)(1000000000.0 / 60.0),
	};

	assert(m);

	r = sd_bus_message_read(m, "uu", &val, &ms);
	if(r < 0)
		return r;

	oldbrg = bl.brg;
	frame_dur = frame_duration_ms(ms);
	d = ((float)min(val, bl.maxbrg) - (float)oldbrg) / (float)frame_dur;

	bl.t = 1;
	for(i = 0; i < frame_dur; i++) {
		if(set_brightness(oldbrg + (d * i)) < 0)
			break;
		if(nanosleep(&req, NULL) < 0)
			break;
	}
	bl.t = 0;
	GUARD(val = set_brightness(val));
	bl.brg = val;

	return sd_bus_reply_method_return(m, "");
}

static method_toggle_backlight(sd_bus_message *m,
			       void *userdata,
			       sd_bus_error *error)
{
}

static const sd_bus_vtable vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_PROPERTY("Transitioning", "b",
			property_get_transitioning,
			0,
			0),
	SD_BUS_PROPERTY("MaxBrightness", "u",
			property_get_maxbrightness,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
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
	SD_BUS_METHOD("SetBrightnessSmooth", "uu", "",
		      method_set_brightness_smooth,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("ToggleBacklight", "", "b",
		      method_toggle_backlight,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("ToggleBacklightSmooth", "u", "b",
		      method_toggle_backlight_smooth,
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

	assert(init_backl(&bl) == OK);

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
