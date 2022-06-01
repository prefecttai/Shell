

/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <cstring>
#include "y.tab.hh"
#include <sys/types.h>
#include <sys/wait.h>
 #include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>

////////////  Start added code ///////////

extern "C" char * read_line();


int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }

 ch = *p;
 p++;

 return ch;
}



#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////

// static  void yyunput (int c,char *buf_ptr  );

// void myunputc(int c) {
//   unput(c);
// }

#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

%}

%option noyywrap

%%


source[ ][^ |\t\n><&][^| \t\n><&]*   { 

int j=0;
for(int i=7;i<strlen(yytext);i++)
{
yytext[j]=yytext[i];j++;
}
yytext[j]='\0';
  if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
      {
      perror("Includes nested too deeply" );
      exit( 1 );
      }

  include_stack[include_stack_ptr++] =
      YY_CURRENT_BUFFER;

  yyin = fopen( yytext, "r" );
  //printf("yytext: %s\n", yytext);
  if ( ! yyin )
      perror("Includes nested too deeply" );  

  yy_switch_to_buffer(
      yy_create_buffer( yyin, YY_BUF_SIZE ) );

}

<<EOF>> {
  if ( --include_stack_ptr < 0 )
      {
      yyterminate();
      }

  else
      {
      yy_delete_buffer( YY_CURRENT_BUFFER );
      yy_switch_to_buffer(
          include_stack[include_stack_ptr] );
      }
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
	return LESS;
}

"2>" {
  return TWOGREAT;
}

">>" {
        return GREATGREAT;
} 

">&"   { 
        return GREATAMPERSAND;
}

">>&"   { 
        return GREATGREATAMPERSAND;
}
 
"|"   { 
        return PIPE;
} 

"&"    { 
        return AMPERSAND;
} 


["][^\n\"]*["] {
 for(int i=0;i<strlen(yytext);i++)
{
yytext[i]=yytext[i+1];
}

yytext[strlen(yytext)-1]='\0';
yylval.cpp_string = new std::string(yytext);
return WORD;
}



[^ \t\n|><&]*\\[^ \t\n]* {
int j=0;
for(int i=0;i<strlen(yytext);i++)
{
if(yytext[i]=='\\')
{
  if(yytext[i+1]=='\\')
  {
    yytext[j]=yytext[i+1];
    j++;i++;
  }
}
else{
yytext[j]=yytext[i];
j++;
}}
yytext[j]='\0';
yylval.cpp_string = new std::string(yytext);
return WORD;

}

\$\([^\n][^\n]*\) {


for(int i=0;i<strlen(yytext);i++)
{
yytext[i]=yytext[i+2];
}

yytext[strlen(yytext)-1]='\0';
/*
int j=0; int p=0;
char ** spacearray= (char **) malloc((strlen(yytext)+1)*sizeof(char *));
spacearray[0]= (char *) malloc((strlen(yytext)+1)*sizeof(char));
for(int i=0;i<strlen(yytext);i++)
{
  
  if(yytext[i]==' ')
  {
    if(yytext[i-1]!=' ')
    { 
      spacearray[j][p]='\0';p=0;
    }

    if(yytext[i+1]==' ')
    {}
    else{
      if(i==0 || (i==strlen(yytext)-1))
      {}
      else{
    j++;
   spacearray[j]= (char *) malloc((strlen(yytext)+1)*sizeof(char));
  }}
  }
  else{
  spacearray[j][p]=yytext[i];p++;
  }
}

for(int i=0;i<=j;i++)
{
  //printf("%s\n",spacearray[i]);
}
*/

int tempin=dup(0);
int tempout=dup(1);

int pin[2];
int pout[2];

pipe(pin);
pipe(pout);

dup2(pout[1],1);
dup2(pin[0],0);
close(pin[0]);
close(pout[1]);
write(pin[1],yytext,strlen(yytext));
write(pin[1],"\nexit\n",6);
close(pin[1]);

int re =fork();

if(re==0)
{ 
  std::string path = std::string("/proc/self/exe"); 
  char *abb[2]={(char *)path.c_str(),NULL};
  //printf("%s\n",abb[0]);
  execvp(abb[0],abb);
  perror("execvp");
  exit(1);
}

else if(re<0)
{
  perror("fork");
  exit(1);
}

dup2(tempin,0);
dup2(tempout,1);

close(tempin);
close(tempout);

char * a = new char[1000];
char c;
int kk=0;

while(read(pout[0],&c,1))
{
if (c=='\n')
{c=' ';}
a[kk++]=c;
}

for(int lol=kk-14;lol >=0;lol--)
{
  unput(a[lol]);
}
close(pout[0]);

//waitpid(re,NULL,0);

}


[^ |\t\n><&][^| \t\n><&]*  {
  /* Assume that file names have only alpha chars */
//  printf("%s\n",yytext);
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}


%%
