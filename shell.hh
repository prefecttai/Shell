#ifndef shell_hh
#define shell_hh

#include "command.hh"
extern char** ll;
struct Shell {

  static void prompt();

  static Command _currentCommand;
};

#endif
