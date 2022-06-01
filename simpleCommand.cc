#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <string.h>
#include <string>
#include "simpleCommand.hh"
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include<sys/wait.h>
extern char** ll;
extern int pids;
extern int bpid;
extern std::string lastline;

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector

  std::string mystring="";
  

  if(argument->find("~") != std::string::npos)
    {
      if(strlen((argument)->c_str())==1)
        {
          mystring=strdup(getenv("HOME"));
          argument=new std::string(mystring);
        }
        else
        { 
          for(int j=1;j<argument->length();j++)
            {
              mystring=mystring+argument->at(j);
            }
            mystring[strlen((argument)->c_str())-1]='\0';
   
            mystring = "/homes/" + mystring;
    
            *argument=mystring;
        }
    }

    if((argument->find("${") != std::string::npos) && (argument->find("}") != std::string::npos))
        {
          int i=0;
          std::string innerstr="";

          while(i<argument->length())
                {
                    if(( argument->at(i)=='$') && ( argument->at(i+1)=='{'))
                        {
                            int k=i+2;
                            while((argument->at(k))!='}')
                            {
                              innerstr=innerstr+argument->at(k);k++;
                            }
                           
                            if(argument->at(k)=='}')
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
                                  {
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

                  if(i<argument->length())
                    {
                      mystring=mystring+argument->at(i);
                      i++;
                    }
                  
                }


            
            *argument=mystring;
        }



  //fprintf(stderr,"%s\n",argument->c_str());

  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
