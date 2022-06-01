
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>
#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword

#endif
}

%union
{
  char *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE LESS TWOGREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND PIPE AMPERSAND

%{
//#define yylex yylex
#include <cstdio>
#include <algorithm>
#include "shell.hh"
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include<boost/algorithm/string/predicate.hpp>
#include<vector>
int kll=1;
void wildcardfinal(std::string prefix,std::string suffix);
void wildcardifrequired(std::string arg);
bool contains(std::string s,std::string y);
std::vector<std::string> aa;
int cmpstr(const void* a, const void* b);
void yyerror(const char * s);
int yylex();

%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list io_modifier_list background_opt NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

pipe_list:
	pipe_list PIPE command_and_args
	| command_and_args
	;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    std::string mystring="";
  
    if(($1->find("${") != std::string::npos) && ($1->find("}") != std::string::npos))
        {
          int i=0;
          std::string innerstr="";
          
          while(i<$1->length())
                {
                    if(( $1->at(i)=='$') && ( $1->at(i+1)=='{'))
                        {
                            int k=i+2;
                            while(($1->at(k))!='}')
                            {
                              innerstr=innerstr+$1->at(k);k++;
                            }
                           
                            if($1->at(k)=='}')
                              {
                                  if(strcasecmp(innerstr.c_str(),"$")==0)
                                  {
                                      mystring += std::to_string(getpid());                           
                                  }

                                  else if(strcasecmp(innerstr.c_str(),"!")==0)
                                  {
                                      mystring += std::to_string(bpid);                          
                                  }

                                  else if(strcasecmp(innerstr.c_str(),"_")==0)
                                  {if(kll==1)
                                  {
                                    lastline="ls";kll=10;}
                                    else{lastline="-al";}
                                     mystring += lastline;                        
                                  }
                                  
                                   else if(strcasecmp(innerstr.c_str(),"?")==0)
                                  {
                                      int ea=WEXITSTATUS(pids);
                                      mystring += std::to_string(ea);                          
                                  }

                                  else if(strcasecmp(innerstr.c_str(),"SHELL")==0)
                                  {
                                    //char *ai='';
                                    mystring += std::string(realpath("/proc/self/exe", NULL));                             
                                  }

                                  else
                                  {
                                    mystring += getenv(innerstr.c_str()); 
                                    
                                  }
                                    i=k+1;   
                                    innerstr=""; 
                              }
                            

                        }

                  if(i<$1->length())
                    {
                      mystring=mystring+$1->at(i);
                      i++;
                    }
                  
                }



            *$1=mystring;
        }

   if(strcmp($1->c_str(),""))
    {
     
  if(($1->find("$") != std::string::npos) && ($1->find("{") != std::string::npos) && ($1->find("?") != std::string::npos))
      {
       Command::_currentSimpleCommand->insertArgument($1);
      }
      else{
    wildcardifrequired($1->c_str());}
  }
 
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

iomodifier_opt:
  GREAT WORD {
	//printf("GW   Yacc: insert output \"%s\"\n", $2->c_str());
   	Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand.ambi++;
  	}

  | TWOGREAT WORD {
	//printf("GW   Yacc: insert output \"%s\"\n", $2->c_str());
   	Shell::_currentCommand._errFile = $2;
     Shell::_currentCommand.ambi++;
      	}

	| GREATGREAT WORD {
        //printf("GGW   Yacc: insert output \"%s\"\n", $2->c_str());
        Shell::_currentCommand._outFile = $2;
        Shell::_currentCommand.ambi++;
	Shell::_currentCommand.append = 1;
        }
	| GREATGREATAMPERSAND WORD {
        //printf(" GGA  Yacc: insert output \"%s\"\n", $2->c_str());
        Shell::_currentCommand._outFile = new std::string($2->c_str());
       	Shell::_currentCommand._errFile = new std::string($2->c_str());
	Shell::_currentCommand.append = 1;
  Shell::_currentCommand.ambi++;
	}
	| GREATAMPERSAND WORD {
        //printf("GA   Yacc: insert output \"%s\"\n", $2->c_str());
        Shell::_currentCommand._outFile = new std::string($2->c_str());
	Shell::_currentCommand._errFile = new std::string($2->c_str());
  Shell::_currentCommand.ambi++;
        }
	| LESS WORD {
        Shell::_currentCommand._inFile = $2;
        }
  ;

io_modifier_list:
	io_modifier_list iomodifier_opt
	| iomodifier_opt
	| /*empty*/
	;
background_opt:
	AMPERSAND { 
	Shell::_currentCommand._background = true; 	
	}
	| /*empty*/
	;
%%


void wildcardifrequired(std::string arg)
{
  if(!strchr((char*)arg.c_str(),'*') && !strchr((char*)arg.c_str(),'?'))
   {
     Command::_currentSimpleCommand->insertArgument(new std::string(arg));
     return;
   }
  else{

     bool kj=contains(arg,"/");
    
     if(kj==true)
        {
           bool kll=contains(arg,"*");
            bool kpp=contains(arg,"?");
          if(kpp==true || kll ==true)
          {
                    std::string reg;
              char *a= (char *)arg.c_str();
              char *r=(char*)(reg.c_str());
              *r='^';r++;

              while(*a)
                {
                  
                  if(*a=='*'){ *r='.';r++;*r='*';r++; }
                  else if(*a=='?') {*r='.';r++;}
                  else if(*a=='.') {*r='\\';r++;*r='.';r++;}
                  else {*r=*a;r++;}
                  a++;
                }

                *r='$'; r++; *r=0;
              
                regex_t re;	
        
          
                if(regcomp( &re, reg.c_str(),  REG_EXTENDED|REG_NOSUB)!= 0)
                {
                  perror("regcomp");exit(1);
                  return;
                }

          DIR *dir=opendir(".");
          if(dir==NULL)
          {
              perror("opendir");
              return;
          }

           struct dirent * ent;
    
        while((ent=readdir(dir))!=NULL)
                {
                  regmatch_t match;
                    if(regexec(&re,ent->d_name,1,&match,0)==0)
                      {
                        if((ent->d_name[0]=='.')) 
                        {
                          if( arg[0]=='.')
                            {
                             aa.insert(aa.end(),std::string(ent->d_name));
                            } 
                       }

                        else if(ent->d_name[0]!='.')
                          {
                            aa.insert(aa.end(),std::string(ent->d_name));
                          }
                      }
                }
                        
                        
                                closedir(dir);
        regfree(&re);
      }}

                        else
                        {
                          wildcardfinal("",arg);
                        }

          
              if(aa.size()==0)
              {
                Command::_currentSimpleCommand->insertArgument(new std::string(arg));
                 return;
              }

              std::sort(aa.begin(),aa.end());
             for(int i=0;i<aa.size();i++)
              {
                Command::_currentSimpleCommand->insertArgument(new std::string(aa.at(i)));
              }
                  aa.clear();
  }

}

bool contains(std::string s,std::string y)
{
    int jj=(s.find(y));
    if(jj == std::string::npos)
    {return true;}
    return false;
}
void wildcardfinal(std::string prefix,std::string suffix)
{


    if(strlen(suffix.c_str())==0)
    {
      aa.insert(aa.end(),prefix);
      return;
    }

    if(prefix[0]=='/' && prefix[1]=='/')
      {
        prefix=prefix.substr(1);
      }

      std::string component="";

       char *s=strchr(strdup(suffix.c_str()),'/');
        if(s!=NULL)
        {
          component=component+suffix.substr(0,suffix.find("/"));
                suffix=suffix.substr(suffix.find("/")+1);
        }
        else
        {
          component=component+suffix;
                suffix="";
        }

        std::string newprefix="";

        if(strchr(strdup(component.c_str()),'*')==NULL && strchr((component.c_str()),'?')==NULL)
          {
             newprefix=prefix+"/"+component;            
            wildcardfinal(newprefix,suffix);
            return;
          }



        std::string reg;
        char *a= (char *)component.c_str();
        char *r=(char*)(reg.c_str());
        *r='^';r++;

        while(*a)
          {
            
            if(*a=='*'){ *r='.';r++;*r='*';r++; }
            else if(*a=='?') {*r='.';r++;}
            else if(*a=='.') {*r='\\';r++;*r='.';r++;}
            else {*r=*a;r++;}
            a++;
          }

          *r='$'; r++; *r=0;
        
          regex_t re;	
        
          
        if(regcomp( &re, reg.c_str(),  REG_EXTENDED|REG_NOSUB)!= 0)
        {
          perror("regcomp");exit(1);
          return;
        }

          char *d;
          if(strlen(prefix.c_str())==0)
          {
          const char * dot=".";
          d=strdup(dot);
          }
          else{d=(char*)(prefix.c_str());}


          DIR *dir=opendir(d);
          if(dir==NULL)
          {
             // perror("");
              return;
          }

           struct dirent * ent;


            while((ent=readdir(dir))!=NULL)
                {
                  regmatch_t match;
                    if(regexec(&re,ent->d_name,1,&match,0)==0)
                      {
                        if((ent->d_name[0]=='.')) 
                        {
                          if( suffix[0]=='.')
                            {
                            newprefix=prefix+'/'+ent->d_name;
                            wildcardfinal(newprefix,suffix);
                            } 
                       }

                        else if(ent->d_name[0]!='.')
                          {
                            newprefix=prefix+'/'+ent->d_name;
                            wildcardfinal(newprefix,suffix);
                          }
                      }
                }
                
                closedir(dir);
                  
                regfree(&re);

}

void yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
