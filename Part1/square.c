#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

int numlen(unsigned long long val)
{
	int ans = 0;
	while(val > 0){
		ans++;
		val /= 10;
	}
	if(ans == 0) ans++;
	return ans;
}

int main(int argc, char* argv[])
{
	char* num = argv[argc - 1];
	while(*num != '\0'){
		if(*num < '0' || *num > '9'){
			perror("Unable to execute");
			exit(1);
		}
		num = num + 1;
	}
	unsigned long long inval = atoll(argv[argc - 1]);
	unsigned long long v = inval*inval;
	if(argc == 2){
		printf("%lld\n", v);
		exit(0);	
	}
	else{
		int len = strlen(argv[1]);
		char ** nargs = (char**) malloc((argc)*sizeof(char*));
		nargs[0] = (char*) malloc((len + 3)*sizeof(char));
		nargs[0][0] = '.';
		nargs[0][1] = '/';
		for(int i = 0; i < len; i++){
			nargs[0][2 + i] = argv[1][i];
		}
		nargs[0][len + 2] = '\0';
		for(int i = 1; i < argc - 2; i++){
			int l = strlen(argv[i]);
			nargs[i] = (char*) malloc((len + 1)*sizeof(char));
			strcpy(nargs[i], argv[i + 1]);
		}
		int nlen = numlen(v);
		nargs[argc - 2] = (char*) malloc((nlen + 1)*sizeof(char));
		nargs[argc - 2][nlen] = '\0';
		unsigned long long val = v;
		for(int i = nlen - 1; i >= 0; i--){
			nargs[argc - 2][i] = '0' + val%10;
			val /= 10;
		}
		nargs[argc - 1] = NULL;
		execv(nargs[0], nargs);
	}
	return 0;
}
