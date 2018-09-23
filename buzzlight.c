// buzzlight.c
//

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "buzzlight.h"

static float __f_div_ii(int n, int d)
{
	return ((float)n / (float)d);
}

static int __i_mul_if(int n, float r)
{
	return (int)((float)n * r);
}

static int __i_min_ii(int a, int b)
{
	return a < b ? a : b;
}

static int __i_max_ii(int a, int b)
{
	return a > b ? a : b;
}

static int __read_int(const char *path)
{
	char buf[32];
	int fd = open(path, O_RDONLY);

	if(fd < 0)
		return -1;

	read(fd, buf, sizeof(buf));
	close(fd);
	return strtol(buf, NULL, 10);
}

int init_backl(struct backl *bl)
{
	int brightness, max_brightness;

	if(bl == NULL)
		return -1;

	brightness = __read_int(PATH_BACKLIGHT "/brightness");
	if(brightness < 0)
		goto error;
	max_brightness = __read_int(PATH_BACKLIGHT "/max_brightness");
	if(max_brightness < 0)
		goto error;
	bl->brightness = brightness;
	bl->max_brightness = max_brightness;
	return 0;
error:
	bl->brightness = -1;
	bl->max_brightness = -1;
	return -1;
}

static float __backl_brg_ratio(const struct backl *bl)
{
	return __f_div_ii(bl->brightness, bl->max_brightness);
}

int backl_get_brightness_percent(const struct backl *bl)
{
	return (int)(__backl_brg_ratio(bl) * 100.0);
}

int backl_set_brightness(const struct backl* bl, int val)
{
	char buf[32];
	int n, count;
	int fd = open(PATH_BACKLIGHT "/brightness", O_WRONLY);

	if(fd < 0)
		return -1;
	ftruncate(fd, 0);

	n = __i_max_ii(__i_min_ii(val, bl->max_brightness), 0);
	snprintf(buf, sizeof(buf), "%d\n", n);
	count = write(fd, buf, strlen(buf));
	close(fd);

	if(count < 0)
		return -1;

	return 0;
}

static float __backl_brg_of_ratio(const struct backl *bl, float ratio)
{
	return __i_mul_if(bl->max_brightness, ratio);
}

int backl_set_brightness_percent(const struct backl *bl, int val)
{
	return backl_set_brightness(
		bl, __backl_brg_of_ratio(bl, (float)val / 100.0));
}

int backl_increase_units(const struct backl *bl, int val)
{
	float ratio = __backl_brg_ratio(bl) + ((float)val / 100.0);
	return backl_set_brightness(bl, __backl_brg_of_ratio(bl, ratio));
}

int backl_increase_percent(const struct backl *bl, int val)
{
	float ratio = __backl_brg_ratio(bl);
	int nval = __backl_brg_of_ratio(
		bl, ratio * (1.0 + ((float)val / 100.0)));
	if(nval == __backl_brg_of_ratio(bl, ratio))
		return backl_increase_units(bl, val < 0 ? -1 : 1);
	return backl_set_brightness(bl, nval);
}

static int __parse_value(const char *arg, struct blcmd *cmd)
{
	int val;
	char *cp;

	if(*arg == '+') {
		cmd->instr = BLINSTR_INC;
		arg++;
	}
	else if (*arg == '-') {
		cmd->instr = BLINSTR_DEC;
		arg++;
	}
	val = strtol(arg, &cp, 10);
	cmd->val = val;

	if(strlen(cp) == 0)
		return 0;
	if(*cp != '%' || strlen(cp) > 1)
		return -1;
	if(cmd->instr == BLINSTR_SET)
		return 0;
	cmd->instr |= BLINSTR_PMOD;
	return 0;
}

static int __parse_args(int len, const char **arg,
			int cur, struct blcmd *cmd)
{
	if(cmd == NULL)
		return -1;
	if(cur >= len)
		return 0;
	if(cur == 0) {
		return __parse_args(len, ++arg, 1, cmd);
	}
	if(cur == 1) {
		if(!strcmp(*arg, "-get")) {
			cmd->instr = BLINSTR_GET;
			return __parse_args(len, ++arg, 2, cmd);
		}
		if(!strcmp(*arg, "-set")) {
			cmd->instr = BLINSTR_SET;
			return __parse_args(len, ++arg, 2, cmd);
		}
		if(!strcmp(*arg, "-help")) {
			cmd->instr = BLINSTR_HELP;
			return __parse_args(len, ++arg, 2, cmd);
		}
		return -1;
	}
	if(cur == 2) {
		if(cmd->instr != BLINSTR_SET)
			return -1;
		if(__parse_value(*arg, cmd) < 0)
			return -1;
		return __parse_args(len, ++arg, 3, cmd);
	}
	return -1;
}

struct blcmd make_blcmd_args(const char **args, size_t count)
{
	struct blcmd cmd = {
		.instr = BLINSTR_GET,
		.val = 0,
	};
	if(__parse_args(count, args, 0, &cmd) < 0)
		cmd.instr = BLINSTR_UNDEF;
	return cmd;
}

blcmd_error blcmd_out_call(const struct blcmd *cmd,
			   const struct backl *bl,
			   char *buf, size_t len)
{
	int n;

	if(cmd == NULL || bl == NULL)
		return BLCMD_ERROR_NULL;

	switch(cmd->instr) {
	case BLINSTR_GET:
		n = snprintf(
			buf, len,
			"brightness: %d%%\n",
			backl_get_brightness_percent(bl));
		break;
	case BLINSTR_HELP:
		n = snprintf(
			buf, len,
			"-help           Prints this message\n"
			"-get            Prints brigtness in percent\n"
			"-set <value>    Sets backlight\n");
		break;
	default:
		return BLCMD_ERROR_CMD;
	}
	if(n >= len)
		return BLCMD_ERROR_BUFOVERFLOW;
	return BLCMD_ERROR_OK;
}

static blcmd_error __blcmd_syscall(int res)
{
	return res < 0 ? BLCMD_ERROR_SYS : BLCMD_ERROR_OK;
}

blcmd_error blcmd_in_call(const struct blcmd *cmd, const struct backl *bl)
{
	if(cmd == NULL || bl == NULL)
		return BLCMD_ERROR_NULL;

	switch(cmd->instr) {
	case BLINSTR_SET:
		return __blcmd_syscall(
			backl_set_brightness_percent(bl, cmd->val));
	case BLINSTR_INC:
		return __blcmd_syscall(
			backl_increase_units(bl, cmd->val));
	case BLINSTR_DEC:
		return __blcmd_syscall(
			backl_increase_units(bl,-cmd->val));
	case BLINSTR_INCP:
		return __blcmd_syscall(
			backl_increase_percent(bl, cmd->val));
	case BLINSTR_DECP:
		return __blcmd_syscall(
			backl_increase_percent(bl, -cmd->val));
	}
	return BLCMD_ERROR_CMD;
}
