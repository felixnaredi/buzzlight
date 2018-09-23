// buzzlight.h
//

#ifndef BUZZLIGHT_H
#define BUZZLIGHT_H

#include <stddef.h>

#ifdef DEBUG
#define PATH_BACKLIGHT "debug/backlight"
#else
#define PATH_BACKLIGHT "/sys/class/backlight/intel_backlight"
#endif /* DEBUG */

struct backl
{
	int brightness;
	int max_brightness;
};

#define BLINSTR_OTYPE 0x100
#define BLINSTR_ITYPE 0x200
#define BLINSTR_PMOD  0x400

typedef enum
{
	BLINSTR_UNDEF = 0,
	BLINSTR_HELP  = BLINSTR_OTYPE,
	BLINSTR_GET,
	BLINSTR_SET   = BLINSTR_ITYPE,
	BLINSTR_INC,
	BLINSTR_DEC,
	BLINSTR_INCP  = (BLINSTR_PMOD | BLINSTR_INC),
	BLINSTR_DECP  = (BLINSTR_PMOD | BLINSTR_DEC),
} blinstr;

typedef enum
{
	BLCMD_ERROR_OK = 0,
	BLCMD_ERROR_NULL,
	BLCMD_ERROR_CMD,
	BLCMD_ERROR_SYS,
	BLCMD_ERROR_BUFOVERFLOW,
} blcmd_error;

struct blcmd
{
	blinstr instr;
	int val;
};

int init_backl(struct backl *bl);
int backl_get_brightness_percent(const struct backl *bl);
int backl_set_brightness(const struct backl *bl, int val);
int backl_set_brightness_percent(const struct backl *bl, int val);
int backl_increase_units(const struct backl *bl, int val);
int backl_increase_percent(const struct backl *bl, int val);
struct blcmd make_blcmd_args(const char **args, size_t count);
blcmd_error blcmd_out_call(const struct blcmd *cmd, const struct backl *bl, char *buf, size_t len);
blcmd_error blcmd_in_call(const struct blcmd *cmd, const struct backl *bl);


#endif /* BUZZLIGHT_H */
