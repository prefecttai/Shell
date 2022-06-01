/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_BUFFER_LINE 2048
#define MAX_HISTORY 10

extern void tty_raw_mode(void);
// extern char * read_line();

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
int cursorPos;

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
int history_length = 0;
char ** history = NULL;
int history_max = MAX_HISTORY;

void initialize_history(){
  history = (char **) malloc (history_max * sizeof(char*));
}

void addHistory (char* history_str){
  char * newEntry = (char*) malloc (sizeof(history_str) * sizeof(char*));
  strcpy (newEntry,history_str);

  //ALLOCATE Enough space for history
  if(history_length == history_max) {
    history_max *= 2;
    history = (char**) realloc (history, history_max * sizeof(char*));
  }

  //Add into history array
  // strcpy(history[history_index++], newEntry);
  history[history_index++] = newEntry;
  history_length++;
}

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n"
    " left arrow   move cursor to the left\n"
    " right arrow  move cursor to the right\n"
    " ctrl-d       delete at the end or end\n"
    " ctrl-h       backspace at the end or middle\n"
    " ctrl-a       move cursor back to front\n"
    " ctrl-e       move cursor to end\n"
    " ctrl-c       kills the program\n";

  write(1, usage, strlen(usage));
}

void resetLineBuffer() {
  //reset the line_buffer
  for(int i = 0; i < MAX_BUFFER_LINE; i++){
    line_buffer[i] = '\0';
  }
}

void deletenchar(int nChar) {
  for(int i = 0; i < nChar; i++){
    // if(cursorPos <= 0) break;
    char charac = 8;
    write(1,&charac,1);
  }
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // //save terminal settings
  struct termios default_attribute;
  tcgetattr(0, &default_attribute);

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  cursorPos = 0;  //Initialize cursorPos to 0

//resetLineBuffer
  resetLineBuffer();

  // Read one line until enter is typed
  while (1) {

    //History
    if(history == NULL) {
      initialize_history();
    }

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch<127) {
      // It is a printable character. 

      if(cursorPos == line_length) {
        //cursor at the end of the line

        // Do echo
        write(1,&ch,1);

        // If max number of character reached return.
        if (line_length==MAX_BUFFER_LINE-2) break; 

        //put ch into buffer and increment cursorPos and line_length
        line_buffer[cursorPos] = ch;
        line_length++;
        cursorPos++;

      } else { 
        //Cursor in the middle

        //save the str after the cursor in tempStr
        char * tempStr = (char*) malloc (MAX_BUFFER_LINE * sizeof(char));

        int index = 0;
        for(int i = cursorPos; i < line_length || i < MAX_BUFFER_LINE; i++) {
          //If reach the end of string then break
          if(line_buffer[i] == '\0') break;
          
          tempStr[index++] = line_buffer[i];
        }
        tempStr[index] = '\0'; // terminate with null

        // fprintf(stderr, "\n%d: tempStr=%s\n", index,tempStr);
        //write the new char
        write (1, &ch, 1);


        //put in buffer
        line_buffer[cursorPos] = ch;
        line_length++;
        cursorPos++;

        //reached the end of the buffer
        if(line_length == MAX_BUFFER_LINE - 2) break;

        //copy remaining chars to buffer
        int numchar = 0;
        for(unsigned int i = 0; i < strlen(tempStr); i++){
          numchar++;
          write(1, &tempStr[i], 1);
          line_buffer[cursorPos + i] = tempStr[i];
        }
        line_buffer[cursorPos + numchar] = '\0';

        //Cursor still at the same place,
        //Need to push it back
        deletenchar(numchar);
        free(tempStr);

      }
    } 
    else if (ch == 9) {
      //TAB 

    } 
    else if (ch == 1) {
      //ctrl + a
      deletenchar(cursorPos);
      cursorPos = 0;
    }
    else if (ch == 5) {
      //ctrl + e
      for(int i = cursorPos; i < line_length; i++){
        char c = line_buffer[i];
        write(1,&c,1);
        cursorPos++;
      }
    }
    else if (ch == 4) {
      //ctrl + d
      if(cursorPos >= line_length) {
        continue;
      }

      //If at the end
      if(cursorPos == line_length - 1) {
        for(int i = cursorPos + 1; i < line_length; i++) {
          char c = line_buffer[i];
          write(1,&c,1);
        }
        char c = ' ';
        write(1,&c,1);

        deletenchar(line_length - cursorPos);

        line_length--;
      } else { 

        char savedStr[MAX_BUFFER_LINE];
        int saveIn = 0;
        for(int i = 0 ; i < line_length; i++){
          if(i != cursorPos) {
            savedStr[saveIn++] = line_buffer[i];
          }
        }
        savedStr[saveIn] = '\0';
        // fprintf(stderr, "\nsaved=%s\n", savedStr);
        line_length--;

        resetLineBuffer();
        //For updating the linebuffer
        int numchar = 0;
        for(int i = 0; i < line_length || i < MAX_BUFFER_LINE; i++){
          if(savedStr[i] == '\0') break;
          // write(1,&tempStr[i],1);
          line_buffer[i] = savedStr[i];
          numchar++;
        }
        
        //clear the inputline
        deletenchar(cursorPos);
        for(int i = 0; i <= line_length; i++){
          char c = ' ';
          write(1, &c, 1);
        }

        deletenchar(line_length+1);

        //rewrite the inputstring
        for(int i = 0; i < line_length; i++){
          write(1, &savedStr[i], 1);
        }

        //push cursor back
        deletenchar(line_length - cursorPos);
      }
    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      //If has any input, add to history
      if( strlen(line_buffer) != 0) {
        addHistory(line_buffer);
      }

      
      // Print newline
      write(1,&ch,1);

      //Print the prompt if enter is hit
      if(strlen(line_buffer) == 0){
        fprintf(stderr, "myshell>");
      }
     

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 8 || ch == 127) {
      // <backspace> was typed. Remove previous character read.

      //If cursor already at beginning do nothing
      if(cursorPos <= 0) {
        continue;
      }

      //If the cursor is at the end
      if(cursorPos >= line_length) {
        // Go back one character
        deletenchar(1);

        // Write a space to erase the last character read
        ch = ' ';
        write(1,&ch,1);

        // Go back one character
        // deletenchar(line_length - cursorPos);
        deletenchar(1);

        // Remove one character from buffer
        line_buffer[cursorPos] = '\0';
        line_length--;
        cursorPos--;
      } else {
        //Cursor is at the middle

        //Save a cpoy of the string
        char saveStr[MAX_BUFFER_LINE];
        int saveIn = 0;
        for(int i = 0; i < line_length; i++){
          if(i != cursorPos - 1) {
            saveStr[saveIn++] = line_buffer[i];
          }
        }
        saveStr[saveIn] = '\0';

        //For updating the linebuffer
        char tempStr[MAX_BUFFER_LINE];

        int index = 0;
        for(int i = cursorPos; i < line_length || i < MAX_BUFFER_LINE; i++){
          //If reach the endof string then break;
          if(line_buffer[i] == '\0') break;

          tempStr[index++] = line_buffer[i];
        }

        tempStr[index] = '\0';

        //move cursor back
        cursorPos--;
        line_length--;
        deletenchar(1);

        //For updating the linebuffer
        int numchar = 0;
        for(int i = 0; i < line_length || i < MAX_BUFFER_LINE; i++){
          if(tempStr[i] == '\0') break;
          // write(1,&tempStr[i],1);
          line_buffer[i + cursorPos] = tempStr[i];
          numchar++;
        }
        line_buffer[cursorPos+numchar] = '\0';
        deletenchar(line_length-numchar);
        for(int i = 0 ; i <= line_length; i++){
          char c = ' ';
          write(1,&c,1);
        }

        //clear the whole line
        deletenchar(line_length+1);

        //rewrite the whole line
        for(int i = 0; i < line_length; i++){
          write(1, &saveStr[i], 1);
        }

        //Move cursor back to appropriate place
        deletenchar(line_length-cursorPos);
        
      }
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
        // Up arrow. Print next line in history.
        // Erase old line
        // Print backspaces
        int i = 0;
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }

        // Print spaces on top
        for (i =0; i < line_length; i++) {
          ch = ' ';
          write(1,&ch,1);
        }

        // Print backspaces
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }	

        // Copy line from history
        if(history_index <= 0) {
          //reach the end
          history_index = 0;
        } else {
          history_index--;
        }

        if(history[history_index] == NULL) {
          //Do nothing if history is null
          strcpy(line_buffer, "");
        } else {
          // fprintf(stderr, "%s\n", history[history_index]);
          char * temp = (char*) malloc(MAX_BUFFER_LINE * sizeof(char*));
          *temp = '\0';
          strcat(temp, history[history_index]);
          strcpy(line_buffer, temp);
          free(temp);
        }

        // strcpy(line_buffer, history[history_index]);
        line_length = strlen(line_buffer);

        // echo line
        write(1, line_buffer, line_length);


      } else if (ch1 == 91 && ch2 == 66) {
        //Down arrow
        //clear the linebuffer
        deletenchar(line_length);

        //write whitespace over
        for(int i = 0; i < line_length; i++){
          ch = ' ';
          write(1,&ch,1);
        }

        //print backspace
        deletenchar(line_length);
        
        if(history_index >= history_length) {
          //reach the end
          history_index = history_length;
        } else {
          history_index++;
        }

        if(history[history_index] == NULL) {
          //Do nothing if history is null
          strcpy(line_buffer, "");
        } else {
          char * temp = (char*) malloc(MAX_BUFFER_LINE * sizeof(char*));
          *temp = '\0';
          strcat(temp, history[history_index]);
          strcpy(line_buffer, temp);
          free(temp);
        }
        
        line_length = strlen(line_buffer);

        // echo line
        write(1, line_buffer, line_length);


      } else if (ch1 == 91 && ch2 == 68) {
        //Left arrow
        if(cursorPos > 0) {
          cursorPos--;
          
          deletenchar(1);
        }

      } else if (ch1 == 91 && ch2 == 67) {
        //Right arrow

        if(cursorPos < line_length) {
        
          // get current new char and write it in
          ch = line_buffer[cursorPos];
          write(1,&ch,1);

          //increment cursor pointer
          cursorPos++;
        }
      }
      
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  //Reset to default attribute
  tcsetattr (0, TCSANOW, &default_attribute);

  return line_buffer;
}

