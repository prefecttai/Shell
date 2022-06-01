#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"
extern int pids;
extern int bpid;
extern std::string lastline;
// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
	int append; int ambi;
 bool _background;

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();

  static SimpleCommand *_currentSimpleCommand;
};

#endif
