typedef struct simplecommand {
  int numavailargs;
  int numargs;
  char **args;

} SimpleCommand;

typedef struct command {
  int numavailscmds;
  int numscmds;
  SimpleCommand **scmds;
  char *outfile;
  char *infile;
  char *errfile;
  int background;
} Command;

void initsimplecmd(SimpleCommand *);
void insertarg(SimpleCommand *, char *);

void initcmd(Command *);
void prompt();
void print(Command *);
void execute(Command *);
void clear(Command *);
void insertsimplecmd(Command *, SimpleCommand *);
void expandwildcards(char *arg);

extern Command currentcmd;
extern SimpleCommand *currentsimplecmd;
