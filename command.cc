/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include<stdio.h>
#include "command.hh"
#include "shell.hh"
#include <signal.h>
#include <stdlib.h>
extern std::string prom;
int pids;
int bpid;
std::string lastline;

Command::Command() {
	// Initialize a new vector of Simple Commands
	_simpleCommands = std::vector<SimpleCommand *>();

	_outFile = NULL;
	_inFile = NULL;
	_errFile = NULL;
	_background = false;
	append = 0;
	ambi=0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
	// add the simple command to the vector

	_simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
	// deallocate all the simple commands in the command vector
	for (auto simpleCommand : _simpleCommands) {
		delete simpleCommand;
	}

	// remove all references to the simple commands we've deallocated
	// (basically just sets the size to 0)
	_simpleCommands.clear();

	if ( _outFile ) {
		delete _outFile;
	}
	_outFile = NULL;

	if ( _inFile ) {
		delete _inFile;
	}
	_inFile = NULL;

	if ( _errFile ) {
		delete _errFile;
	}
	_errFile = NULL;
	append = 0;
	ambi=0;
	_background = false;
}

void Command::print() {
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	int i = 0;
	// iterate over the simple commands and print them nicely
	for ( auto & simpleCommand : _simpleCommands ) {
		printf("  %-3d ", i++ );
		simpleCommand->print();
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n",
			_outFile?_outFile->c_str():"default",
			_inFile?_inFile->c_str():"default",
			_errFile?_errFile->c_str():"default",
			_background?"YES":"NO");
	printf( "\n\n" );
}

void Command::execute() {
	// Don't do anything if there are no simple commands

	if ( _simpleCommands.size() == 0 ) {
		Shell::prompt();
		return;
	}

	if(ambi>1)
		{
			printf("Ambiguous output redirect.\n");
		}


	if ((strcmp((_simpleCommands[0]->_arguments[0])->c_str(),"exit")==0) || (strcmp((_simpleCommands[0]->_arguments[0])->c_str(),"EXIT")==0))
	{
		printf("\nGood bye!!\n\n");
		exit(0);
	}
	// Print contents of Command data structure
	//print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec



	int numsimplecommands=_simpleCommands.size();
	int tmpin=dup(0);
	int tmpout=dup(1);
	int tmperr=dup(2);

	int fdin;
	if(_inFile)
	{
		fdin=open(_inFile->c_str(),O_RDONLY, 0666);	
		if(fdin<0)
		{
			printf("/bin/sh: 1: cannot open %s: No such file\n",(_inFile)->c_str());
			exit(1);
		}
	} 
	else{
		fdin=dup(tmpin);
	}


	int fderr;
	if(_errFile)
	{if(append){
			   fderr=open(_errFile->c_str(),O_WRONLY | O_CREAT | O_APPEND,0666);
		   }
	else{
		fderr=open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC,0666);
	}

	if(fderr<0)
	{
		printf("/bin/sh: 1: cannot open %s: No such file\n",(_errFile)->c_str());
		exit(1);
	}
	}
	else{
		fderr=dup(tmperr);
	}
	dup2(fderr, 2);
	close(fderr);
	int i=0;
	int ret;
	int fdout;
	for(i=0;i<numsimplecommands;i++)
	{

		dup2(fdin,0);
		close(fdin);	

		if(!strcmp((_simpleCommands[i]->_arguments[0])->c_str(),"setenv"))
		{
			/*if((strcmp(_simpleCommands[i]->_arguments[1])->c_str(),"PROMPT"))
			{
				std::string kkl((_simpleCommands[i]->_arguments[2])->c_str());
				prom=kkl;
			}
			else{*/
			int c1=setenv((_simpleCommands[i]->_arguments[1])->c_str(),(_simpleCommands[i]->_arguments[2])->c_str(),1);

			if(c1)
			{
				perror("setenv");
			}//}
			clear();
			Shell::prompt();
			return;
		}   
		
		   if(!strcmp((_simpleCommands[i]->_arguments[0])->c_str(),"unsetenv"))
		   {
		   int c2=unsetenv((_simpleCommands[i]->_arguments[1])->c_str());

		   if(c2)
		   {
		   perror("unsetenv");
		   }
		   clear();
		   Shell::prompt();
			return;
		   }   

		if(!strcmp((_simpleCommands[i]->_arguments[0])->c_str(),"cd"))
		{
            int c3=0;
            if((_simpleCommands[i]->_arguments).size()==1)
            {
                c3=chdir((getenv("HOME")));	
            }
            
			else if(!strcmp((_simpleCommands[i]->_arguments[1])->c_str(),"${HOME}"))
            {	
                c3=chdir((getenv("HOME")));
            }
			else
			{
				c3=chdir((_simpleCommands[i]->_arguments[1])->c_str());
				if(c3<0)
				{
					dprintf(2, "cd: can't cd to %s\n", _simpleCommands[i]->_arguments[1]->c_str());
					
				}
				
				
			}
			

			
            clear();
			continue;
			
		}

		
		if(i==numsimplecommands-1)
		{
			if(_outFile)
			{
				if(append)
				{
					fdout=open((_outFile)->c_str(),O_CREAT | O_WRONLY | O_APPEND,0664);
				}
				else{            
					fdout=open((_outFile)->c_str(),O_CREAT | O_WRONLY | O_TRUNC,0664);
					if(fdout < 0)
					{
						printf("/bin/sh: 1: cannot open %s: No such file\n",(_outFile)->c_str());
						exit(1);
					}
				}
				if(fdout < 0)
				{
					printf("/bin/sh: 1: cannot open %s: No such file\n",(_outFile)->c_str());
					exit(1);
				}

			}

			else
			{fdout=dup(tmpout); }
		}

		else
		{
			int fdpipe[2];
			pipe(fdpipe);
			fdout=fdpipe[1];
			fdin=fdpipe[0];
		}


		dup2(fdout,1);
		close(fdout);

		ret=fork();
		if(ret==0)
		{
			char ** ab=new char*[_simpleCommands[i]->_arguments.size() + 1];
			ab[_simpleCommands[i]->_arguments.size()] = NULL;int kj=0;
			for(int j=0;j<(_simpleCommands[i]->_arguments).size();j++)
			{
				ab[j]=(char*)((_simpleCommands[i]->_arguments[j])->c_str());
			kj=j;
			}

			if(i==((_simpleCommands).size()-1))
				{
					std::string ll(ab[kj]);
					lastline=ll;
				}

			

			if(!strcmp((_simpleCommands[i]->_arguments[0])->c_str(),"printenv"))
			{
				char **p=environ;
				while(*p!=NULL)
				{
					printf("%s\n",*p);
					p++;
				}
				exit(0);
			}


			execvp(ab[0], ab);
			perror("execvp");
			exit(1);
		}
		else if(ret<0)
		{
			perror("fork");
			return;
		}

	}// for

    dup2(tmpin,0);
	dup2(tmpout,1);
	dup2(tmperr,2);
	close(tmpin);
	close(tmpout);
	close(tmperr);

	if(!_background)
	{
		waitpid(ret,&pids,0);
	}
	else
	{
		bpid=ret;
	}
	

	clear();
	// Print new prompt
	Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;



