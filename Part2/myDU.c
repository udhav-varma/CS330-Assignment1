#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
int main(int argc, char* argv[])
{
	if(argc < 2){
		perror("Error in input\n");
		exit(EXIT_FAILURE);
	}
	char filename[5000] = "./", originalName[5000];
	strcat(filename, argv[1]);
	strcpy(originalName, argv[1]);
	unsigned long long ans = 4096;
	// fprintf(stderr, "execute %s\n", argv[1]);
	struct stat buf;
	DIR * fobj = opendir(filename);
	struct dirent* obj = readdir(fobj);
	while(obj != NULL){
		// fprintf(stderr, "here %s\n", obj->d_name);
		if(strcmp(obj->d_name, "..") == 0 || strcmp(obj->d_name, ".") == 0){
			obj = readdir(fobj);
			continue;
		}
		if(obj->d_type == DT_DIR){
			char cfile[5000];
			strcpy(cfile, originalName);
			strcat(cfile, "/");
			strcat(cfile, obj->d_name);
			// fprintf(stderr, "Folder: %s\n", cfile);
			int filed[2];
			int code = pipe(filed);
			if(code == -1){
				perror("Pipe fail\n");
				exit(EXIT_FAILURE);
			}
			int pid = fork();
			if(pid < 0){
				perror("Fork failed\n");
				exit(EXIT_FAILURE);
			}
			if(pid == 0){
				dup2(filed[1], STDOUT_FILENO);
				execl("./myDU", "myDU", cfile, NULL);	
			}
			else{
				char filesize[15];
				read(filed[0], filesize, 12);
				ans += atoll(filesize);
			}
		}
		else if(obj->d_type == DT_REG){
			char nbuf[5000];
			strcpy(nbuf, filename);
			int namelen = strlen(obj->d_name);
			char* relname = (char*) malloc((namelen + 2)*sizeof(char));
			relname[0] = '/';
			strcpy(relname + 1, obj->d_name);
			strcat(nbuf, relname);
			int code = stat(nbuf, &buf);
			if(code == -1){
				perror("Stat failed\n");
				exit(EXIT_FAILURE);
			}
			// fprintf(stderr, "%s\n", nbuf);
			// fprintf(stderr, "%d\n", val);
			ans += buf.st_size;
			free(relname);
		}
		// fprintf(stderr, "%s %d\n", obj->d_name, obj->d_type);
		obj = readdir(fobj);
	}
	printf("%lld", ans);
	return 0;
}
