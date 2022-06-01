#include <cstdio>
#include<unistd.h>
#include "shell.hh"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
char** ll;
std::string prom="myshell>";
int yyparse(void);

void Shell::prompt() {
  


if(isatty(0)){
  printf("myshell>");}
  fflush(stdout);
}

extern "C" void check_for_ctrlC( int sig )
{
	fprintf( stderr, "\n");
  Shell::prompt();   
}

void check_for_zombie_elimination( int sig1 )
{
    while(waitpid((pid_t)(-1),0,WNOHANG)>0){}
 
}


int main(int  arg, char **arg1) {

    //ll=arg1;

    struct sigaction sa_for_ctrlC;
    sa_for_ctrlC.sa_handler = check_for_ctrlC;
    sigemptyset(&sa_for_ctrlC.sa_mask);
    sa_for_ctrlC.sa_flags = SA_RESTART;

    int err=sigaction(SIGINT,&sa_for_ctrlC,NULL);

    if(err)
    {
        perror("sigaction");
        exit(-1);
    }

    struct sigaction sa_for_zombie;
    sa_for_zombie.sa_handler = check_for_zombie_elimination;
    sigemptyset(&sa_for_zombie.sa_mask);
    sa_for_zombie.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    int errZ=sigaction(SIGCHLD,&sa_for_zombie,0);

    if(errZ)
    {
        perror("sigaction");
        exit(-1);
    }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
