#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define MAXARGS 128
#define MAXLINE 128

void eval(char *cmdline, char **envp);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int type[128];

pid_t children[128];
int n_children;

int main(int argc, char *argv[], char *envp[])
{
	char cmdline[MAXLINE];
	while(1){
		printf("> ");
		fgets(cmdline, MAXLINE, stdin);
		if(feof(stdin))
			exit(0);

		eval(cmdline, envp);
	}
}

void eval(char *cmdline, char **envp)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int bg, i, s, mov=0, code;
	pid_t pid;
	
	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	if(argv[0] == NULL)
		return;
	
	if(!builtin_command(argv)){
		for(i=0;i<bg;i++){
			if((pid = fork())==0){
//				if(execve(argv[0], argv, envp) < 0){
				if(execvp(argv[mov], argv+mov) < 0) {
					printf("%s: Command not found.\n", argv[mov]);
					exit(0);
				}
			}
			if(type[i] == 1){
				children[n_children++] = pid;
				printf("%d ", pid);
				while(argv[mov]!=NULL){
					printf("%s ", argv[mov]);
					mov++;
				}
				printf("\n");
				mov++;
			}else if(type[i] == 2 || type[i] == 3){
				while(argv[mov]!=NULL)mov++;
				waitpid(pid, &s, 0);
				if(type[i] == 3 && s !=0)return;
				mov++;
			}
		}
		if(argv[mov]){
			if((pid = fork()) == 0){
				execvp(argv[mov], argv+mov);
			}
			waitpid(pid, &s, 0);
		}
	}
	return;
}

int builtin_command(char **argv){
	int i, s, num;
	pid_t result;
	if(!strcmp(argv[0], "quit")){
		for(i=0; i<n_children; i++)
			kill(children[i], SIGKILL);
		for(i=0; i<n_children; i++)
			waitpid(children[i], &s, 0);
		exit(0);
	}
	if(!strcmp(argv[0], "&"))
		return 1;
	return 0;
}

int parseline(char *buf, char **argv){
	int argc = 0;
	int n=0;

	buf[strlen(buf)-1] = ' ';
	argv[argc++] = strtok(buf, " ");
	while((argv[argc] = strtok(NULL, " "))!=NULL){
		if(argv[argc][0] == ';'){
			argv[argc] = NULL;
			type[n] = 2;
			n++;
		}else if(strcmp(argv[argc], "&&") == 0){
			argv[argc] = NULL;
			type[n] = 3;
			n++;
		}else if (argv[argc][0] == '&'){
			argv[argc] = NULL;
			type[n] = 1;
			n++;
		}
		argc++;
	}
	if(argc == 0)
		return 1;
	return n;
}
