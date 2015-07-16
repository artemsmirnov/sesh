#include "inc/repl.h"
#include "inc/term.h"
#include "inc/history.h"
#include "inc/help.h"
#include "inc/dir.h"
#include "inc/exec.h"
#include "inc/ls.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>


void repl () {
	char typecom[][8] = { "cd", "history", "help", "ls" };
	void(*arr_func[])(int, char**) = { dir_cmd, history_cmd, help_cmd, ls_cmd};
	
	while (1) {
		// Prompt
		char cwd[256];
		getcwd(cwd, 256);
		// printf("%s > ", cwd);
		write(STDOUT_FILENO, cwd, strlen(cwd));
		write(STDOUT_FILENO, " > ", 3);
		
		
		// Read
		char buff[1024];
		char *cur = buff;
		char c;

		while (read(STDIN_FILENO, &c, 1) != 0) {
			if (c == '\n') {
				break;
			}
			switch(c) {
				case 8: 
				case 127:
					write(STDOUT_FILENO, "\b \b", 3);
					cur--;
					break;
				case 65:
					write(STDOUT_FILENO, "up", 2);
					break;
				case 66:
					write(STDOUT_FILENO, "down", 4);
					break;
					
				default: 
					write(STDOUT_FILENO, &c, 1);
					*(cur++) = c;
					break;
			}
		}
		*cur = '\0';
		char eol = '\n';
		write(STDOUT_FILENO, &eol, 1);
		

		// Save entry in history
		history_save_cmd(buff);

		// Parse

		int argc = 0;
		char* argv[256];


		char* pch = strtok(buff, " ");
  		while (pch != NULL) {
  			argv[argc++] = pch;
  			pch = strtok(NULL, " ");
  		}
  		argv[argc] = NULL;

		// Route
		if (strcmp(argv[0], "exit") == 0) {
			break;
		}

		int j;
		for (j = 0; j < 4; j++) {
			if (strcmp(argv[0], typecom[j]) == 0) {
				arr_func[j](argc, argv);
				break;
			}
		}

		if (j == 4) {
			exec_cmd(argc, argv);
		}
		// Loop		
	}
}
