// main.c
//

#include <stdio.h>
#include "buzzlight.h"

int main(int argc, char **argv)
{
	struct backl bl;
	struct blcmd cmd;

	if(init_backl(&bl)) {
		printf("Failed to read backlight\n");
		return -1;
	}
	cmd = make_blcmd_args((const char **)argv, argc);
	if(cmd.instr & BLINSTR_OTYPE) {
		char buf[128];

		blcmd_out_call(&cmd, &bl, buf, sizeof(buf));
		printf("%s", buf);
	}
	else if(cmd.instr & BLINSTR_ITYPE) {
		if(blcmd_in_call(&cmd, &bl))
			printf("Failed to perform command\n");
	}
	else
		return -1;
	return 0;
}
