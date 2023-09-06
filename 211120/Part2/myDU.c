#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

// Recursively finds size (useful when no new processes are allowed to be called - example inside a child directory)
unsigned long findsize(char* path)
{
	unsigned long ans = 0;
	struct stat buf;
	int code = stat(path, &buf);
	if(code == -1){
		perror("stat fail at line 14");
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
			if(S_ISREG(buf.st_mode)){		// If link is to a regular file, add size directly
				ans += buf.st_size;
			}		
			else{	// If link is to a directory, we call recursively to find the size of the directory
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
		perror("Unable to execute\n");
		exit(EXIT_FAILURE);
	}
	char filename[5000] = "./", originalName[5000];
	strcat(filename, argv[1]);
	strcpy(originalName, argv[1]);
	if(argc == 3){
		printf("%ld\n", findsize(filename));	// Inside child process, no new calling of process
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
	DIR * fobj = opendir(filename);
	struct dirent* obj = readdir(fobj);
	while(obj != NULL){
		if(strcmp(obj->d_name, "..") == 0 || strcmp(obj->d_name, ".") == 0){
			obj = readdir(fobj);
			continue;
		}
		if(obj->d_type == DT_DIR){	// In parent process, new process is called for each of the subdirectories
			char cfile[5000];
			strcpy(cfile, originalName);
			strcat(cfile, "/");
			strcat(cfile, obj->d_name);
			int filed[2];	// File descriptors for the pipe
			int code = pipe(filed);
			if(code == -1){	// Unsuccessful pipe
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
				execl("./myDU", "myDU", cfile, "child", NULL);	// Extra Argument to indicate that it is a child process (no new call after that)
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
				ans += findsize(nbuf); 	// If symbolic link is found no new process is called
			}
		}
		obj = readdir(fobj);
	}
	printf("%lld", ans);
	return 0;
}
