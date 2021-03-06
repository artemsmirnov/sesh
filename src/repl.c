#include "inc/repl.h"
#include "inc/term.h"
#include "inc/history.h"
#include "inc/help.h"
#include "inc/dir.h"
#include "inc/exec.h"
#include "inc/autocomplete.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define KEY_BACKSPACE 8
#define KEY_DELETE 127
#define KEY_ARROWS 27
#define KEY_TAB 9
#define KEY_RETURN '\n'
#define KEY_SC_UP 'A'
#define KEY_SC_DOWN 'B'
#define KEY_SC_RIGHT 'C'
#define KEY_SC_LEFT 'D'

unsigned char buff[1024], buffcpy[1024];
unsigned char* cur;
unsigned int len;

void remove_char_from_buffer() {
	for (unsigned char* i = cur ; i<=buff+len ; i++) {
		*i = *(i+1);
		write(STDOUT_FILENO, i, 1);
	}
	write(STDOUT_FILENO, " ", 1);
	for (unsigned char* i = cur ; i<=buff+len ; i++) {
		write(STDOUT_FILENO, "\b", 1);
	}
}

/* Description:
*	This function erases one character from the input field.
*  Receives:
*	void
*  Returnes:
*	void
*/

void erase_char() {
	write(STDOUT_FILENO, "\b \b", 3);
	cur--;
	len--;
	
	remove_char_from_buffer();

	if (*cur >= 128) { //if the character was two-byte character
		cur--;
		len--;
		remove_char_from_buffer();
	}
}

/* Description:
*	It cleans the input line to the end of prompt and displays a new received command.
*  Receives:
*	unsigned char* new_buffer -  A string that contains a new command, that were found in the history by the navigation by up-down arrows by user.
*  Returnes:
*	void
*/

void replace_buf(unsigned char* new_buffer) {
	while (cur > buff) {
		erase_char();
	}
	while (*new_buffer != '\n' && *new_buffer != '\0') {
		*(cur++) = *(new_buffer++);
	}

	*(cur) = '\0';
	len = cur - buff;
	write(STDOUT_FILENO, buff, strlen(buff));
}

/* Description:
*	Parses a string to the argv array and returns the number of arguments
*  Receives:
*	unsigned char* buff - A pointer to a string, which was inputted by user.
*	char** argv - A pointer to an array of strings consisting of these arguments.
*  Returnes:
*	int parse_args - The number of arguments in the inputted command.
*/


int parse_args(unsigned char* buff, char** argv) {
	int argc = 0;

	while (1) {
		if (*buff == '\0')
			break;

		argv[argc++] = buff;

		// Search begin of next argument

		short notEscaped = 1;
		short slashEscaped = 0;

		while (*buff != '\0') {
			if (*buff == ' ' && notEscaped)
				break;
			if (*buff == '"' && !slashEscaped)
				notEscaped = !notEscaped;

			slashEscaped = 0;
			if (*buff == '\\')
				slashEscaped = 1;

			buff++;
		}

		if (*buff != '\0') {
			*buff = '\0';
			buff++;
		} else {
			break;
		}
	}
	argv[argc] = NULL;

	return argc;
}

/* Description:
*	It displays the current directory and makes a prompt for entering commands by the user.
*  Receives:
*	void
*  Returnes:
*	void
*/

void print_prompt() {
	char cwd[256];
	getcwd(cwd, 256);

	write(STDOUT_FILENO, cwd, strlen(cwd));
	char invite[] = " > ";
	write(STDOUT_FILENO, invite, strlen(invite));
}

/* Description:
*	REPL = Read, Evaluate, Print, Loop
*	It receives commands entered by the user and transmits them to the
*corresponding functions.
*  Receives:
*	void
*  Returnes:
*	void
*/

void repl() {
	while (1) {
		// Prompt
		unsigned char c[2];
		print_prompt();

		// Read
		cur = buff;
		*cur = '\0';
		len = 0;

		unsigned char* command = NULL;
		int history_index = 0;

		while (read(STDIN_FILENO, &c, 1) != 0) {
			if (c[0] == KEY_RETURN) {
				break;
			}

			switch (c[0]) {
				case KEY_BACKSPACE:
				case KEY_DELETE:
					if (cur != buff) {
						erase_char();
					}
					break;

				case KEY_ARROWS:
					read(STDIN_FILENO, &c, 2);
					if (c[0] == '[') {
						switch (c[1]) {
							case KEY_SC_UP:
								command = history_entry(history_index);
								if (command != NULL) {
									replace_buf(command);
									history_index++;
								}
								break;
							case KEY_SC_DOWN:
								if (history_index <= 0) {
									replace_buf("");
								} else {
									command = history_entry(history_index);
									if (command != NULL) {
										replace_buf(command);
									}
									history_index--;
								}
								break;
							case KEY_SC_RIGHT:
								if (cur < buff+len) {
									write(STDOUT_FILENO, cur++, 1);
								}
								break;
							case KEY_SC_LEFT:
								if (cur > buff) {
									write(STDOUT_FILENO, "\b", 1);
									cur--;
								}
								break;
						}
					}
					break;
				case KEY_TAB:
					// Identifying of the sourse of autocompleting
					strcpy(buffcpy, buff);
					char * argv[256];
					int argc = parse_args(buffcpy, argv);

					char** suggestions = autocomplete_find(argv[argc - 1]);

					if (suggestions[0] != NULL && suggestions[1] != NULL) {
						int i = 0;
						write(STDOUT_FILENO, "\n", 1);
						while (suggestions[i] != NULL) {
							puts( suggestions[i++] );
						}

						print_prompt();

						write(STDOUT_FILENO, buff, strlen(buff));
					} else if (suggestions[0] != NULL) {
						write(STDOUT_FILENO, "\n", 1);
						print_prompt();
						cur -= strlen(argv[argc-1]);
						while ((*cur++ = *suggestions[0]++) != 0);
						cur--;
						write(STDOUT_FILENO, buff, strlen(buff));
					}

					break;

				default:
					len++;
					for (unsigned char* i = buff + len ; i>cur ; i--) {
						*i = *(i-1);
					}

					*(cur++) = c[0];

					int j = 0;

					if (c[0] >= 128) {
						read(STDIN_FILENO, c, 1);
						len++;
						for (unsigned char* i = buff + len ; i>cur ; i--) {
							*i = *(i-1);
						}

						*(cur++) = c[0];
						j=1;
					}

					for (unsigned char* i = cur-1-j ; i<buff+len ; i++) {
						write(STDOUT_FILENO, i, 1);
					}

					for (unsigned char* i = cur ; i<buff+len ; i++) {
						write(STDOUT_FILENO, "\b", 1);
					}
					break;
			}
		}

		strcpy(buffcpy, buff);

		char eol = '\n';
		write(STDOUT_FILENO, &eol, 1);
		// Parse
		char* argv[256];
		int argc = parse_args(buff, argv);

		// Save entry in history
		if (argc > 0) {
			history_save_cmd(buffcpy);
		}

		// Route
		if (argc == 0) {
			continue;
		}

		if (strcmp(argv[0], "exit") == 0) {
			break;
		}

		unsigned char typecom[][8] = {"cd", "history", "help", "ls"};
		void (*arr_func[])(int, char**) = {dir_cmd, history_cmd, help_cmd, ls_cmd};

		int j;
		for (j = 0; j < 4; j++) {
			if (strcmp(argv[0], typecom[j]) == 0) {
				arr_func[j](argc, argv);
				break;
			}
		}

		if (j == 4 && argc > 0) {
			sleep(0.5);
			exec_cmd(argc, argv);
		}
	}
}
