#include "inc/dir.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

char* autocomplete_array[256];
char autocomplete_buff[8192];

char** autocomplete(int argc, char** argv) {
	struct dirent *entry;
	char name_dir[256];

	int Wcounter = 0;
	int Scounter = 0;
	
	getcwd(name_dir, 256);
	DIR *dir=opendir(name_dir);
   
	if (!dir) {
		autocomplete_array[0] = NULL;
		return autocomplete_array;
	}

	while ( (entry = readdir(dir)) != NULL ) {	
		int i = 0;
		int notPrefix = 0;	
		while((argv[1][i] != '\0')) {
			if(entry->d_name[i] != argv[1][i]) {
				notPrefix = 1;
				break;
			}
			if(entry->d_name[i] == '\0') {
				notPrefix = 1;
				break;
			}
			i++;
		}
		if (notPrefix == 0) {
			Wcounter++;
			autocomplete_array[Wcounter] = autocomplete_buff+Scounter;
			i = 0;
			while(entry->d_name[i]!=0) {
				autocomplete_buff[Scounter] = entry->d_name[i];
				printf("%c\n", autocomplete_buff[Scounter]);
				Scounter++;
				i++;
			}
			autocomplete_buff[Scounter] = NULL;
			Scounter++;
		}
 	} 
	closedir(dir);

	return autocomplete_array;
}