#include "inc/cmd_cd.h"

#include <unistd.h>
#include <stdio.h>

void cmd_cd(char* _str, char* _com) { 
	int i = chdir(_str+3);
	if (i != 0) {
		puts("\nUnable to change directory");
	}
};