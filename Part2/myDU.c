#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

unsigned long findsize(char* path)
{
	// fprintf(stderr, "at %s\n", path);
	unsigned long ans = 0;
	struct stat buf;
	int code = stat(path, &buf);
	if(code == -1){
		perror("stat fail at line 73");
		exit(-1);
	}
	ans += buf.st_size;
	DIR * fobj = opendir(path);
	struct dirent * obj = readdir(fobj);
	while(obj != NULL){
		if(strcmp(obj->d_name, "..") == 0 || strcmp(obj->d_name, ".") == 0){
			obj = readdir(fobj);
			continue;
		}
		if(obj->d_type == DT_DIR){
			char cfile[5000];
			strcpy(cfile, path);
			strcat(cfile, "/");
			strcat(cfile, obj->d_name);
			ans += findsize(cfile);
		}
		else if(obj->d_type == DT_REG){
			char nbuf[5000];
			strcpy(nbuf, path);
			strcat(nbuf, "/");
			strcat(nbuf, obj->d_name);
			int code = stat(nbuf, &buf);
			if(code == -1){
				perror("stat at line 33 failed\n");
				exit(EXIT_FAILURE);
			}
			ans += buf.st_size;
		}
		else if(obj->d_type == DT_LNK){
			char nbuf[5000];
			strcpy(nbuf, path);
			strcat(nbuf, "/");
			strcat(nbuf, obj->d_name);
			int code = stat(nbuf, &buf);
			if(code == -1){
				perror("stat at line 44 failed\n");
				exit(EXIT_FAILURE);
			}
			if(S_ISREG(buf.st_mode)){
				ans += buf.st_size;
			}	
			else{
				ans += findsize(nbuf);
			}
		}
		obj = readdir(fobj);
	}
	return ans;
}

int main(int argc, char* argv[])
{
	if(argc < 2){
		perror("Error in input\n");
		exit(EXIT_FAILURE);
	}
	char filename[5000] = "./", originalName[5000];
	strcat(filename, argv[1]);
	strcpy(originalName, argv[1]);
	if(argc == 3){
		printf("%ld\n", findsize(filename));
		exit(0);
	}
	struct stat buf;
	unsigned long long ans = 0;
	int code = stat(filename, &buf);
	if(code == -1){
		perror("stat fail at line 73");
		exit(-1);
	}
	ans += buf.st_size;
	// fprintf(stderr, "execute %s\n", argv[1]);
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
			// ans += findsize(cfile);
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
				execl("./myDU", "myDU", cfile, "child", NULL);	
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
			strcat(nbuf, "/");
			strcat(nbuf, obj->d_name);
			int code = stat(nbuf, &buf);
			if(code == -1){
				perror("Stat failed\n");
				exit(EXIT_FAILURE);
			}
			// fprintf(stderr, "%s\n", nbuf);
			// fprintf(stderr, "%d\n", val);
			ans += buf.st_size;
		}
		else if(obj->d_type == DT_LNK){
			char nbuf[5000];
			strcpy(nbuf, filename);
			strcat(nbuf, "/");
			strcat(nbuf, obj->d_name);
			int code = stat(nbuf, &buf);
			if(code == -1){
				exit(EXIT_FAILURE);
			}
			if(S_ISREG(buf.st_mode)){
				ans += buf.st_size;
			}	
			else{
				ans += findsize(nbuf);
			}
		}
		// fprintf(stderr, "%s %d\n", obj->d_name, obj->d_type);
		obj = readdir(fobj);
	}
	printf("%lld", ans);
	return 0;
}
