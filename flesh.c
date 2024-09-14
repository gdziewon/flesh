#include "flesh.h"
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define S_NUM_AVAIL_ARGS 10
#define NUM_AVAIL_S_CMDS 10

Command currentcmd;
SimpleCommand *currentsimplecmd;

void initsmplcmd(SimpleCommand *cmd) {
  cmd->numavailargs = S_NUM_AVAIL_ARGS;
  cmd->numargs = 0;
  cmd->args = (char **)malloc(S_NUM_AVAIL_ARGS * sizeof(char **));
}

void insertarg(SimpleCommand *cmd, char *arg) {
  if (cmd->numargs >= cmd->numavailargs) {
    cmd->numavailargs *= 2;
    cmd->args = realloc(cmd->args, cmd->numavailargs * sizeof(char *));
  }
  cmd->args[cmd->numargs++] = arg;
}

void initcmd(Command *cmd) {
  cmd->numavailscmds = NUM_AVAIL_S_CMDS;
  cmd->numscmds = 0;
  cmd->scmds =
      (SimpleCommand **)malloc(NUM_AVAIL_S_CMDS * sizeof(SimpleCommand **));
  cmd->outfile = NULL;
  cmd->infile = NULL;
  cmd->errfile = NULL;
  cmd->background = 0;
}

void clear(Command *cmd) {
  int i, j;
  for (i = 0; i < cmd->numscmds; i++) {
    for (j = 0; j < cmd->scmds[i]->numargs; j++)
      free(cmd->scmds[i]->args[j]);
    free(cmd->scmds[i]->args);
    free(cmd->scmds[i]);
  }
  free(cmd->scmds);
  free(cmd->outfile);
  free(cmd->infile);
  free(cmd->errfile);
}

void prompt() {
  fprintf(stdout, "flesh");
  fflush(stdout);
}

void execute(Command *cmd) {
  int tmpin, tmpout, fdin, fdout, i, ret;
  tmpin = dup(0); // backup in/out
  tmpout = dup(1);
  if (cmd->infile)
    fdin = open(cmd->infile, O_RDONLY);
  else
    fdin = dup(tmpin);

  for (i = 0; i < cmd->numscmds; i++) {
    dup2(fdin, 0); // redirect input
    close(fdin);
    if (i == cmd->numscmds - 1) { // Last simple command
      if (cmd->outfile)
        fdout = open(cmd->outfile, O_RDWR);
      else
        fdout = dup(tmpout);
    } else { // Not last simple command, create pipe
      int fdpipe[2];
      pipe(fdpipe);
      fdout = fdpipe[1];
      fdin = fdpipe[0];
    }

    dup2(fdout, 1); // redirect output
    close(fdout);

    if (!(ret = fork())) { // create child process
      execvp(cmd->scmds[i]->args[0],
             cmd->scmds[i]
                 ->args); // replaces childs image with the executed command
      perror("Execution failed: "); // if execvp succeedes, this code will never
                                    // be executed
      exit(1); // this terminates child process if execvp fails - the reason why
               // there is exit code 1
    } else if (ret < 0) {
      perror("Failed to create a process"); // return because negative ret
                                            // indicates error
      return;
    }
  }

  dup2(tmpin, 0); // restore in/out defaults
  dup2(tmpout, 1);
  close(tmpin);
  close(tmpout);

  if (cmd->background) // wait for last command
    fprintf(stdout, "PID: %d", ret);
  else
    waitpid(ret, NULL, 0);
}

int pstrcmp(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

void expandwildcards(char *arg) {
  if (strchr(arg, '*') == NULL &&
      strchr(arg, '?') == NULL) { // no wildcards present
    insertarg(currentsimplecmd, arg);
    return;
  }

  char *reg, *a, *r, **entries;
  int ret, maxent, nument, i;
  regex_t preg;
  DIR *dir;
  struct dirent *ent;

  nument = 0, maxent = 20;
  assert((reg = (char *)malloc(2 * strlen(arg) + 10)) != NULL);
  assert((entries = (char **)malloc(maxent * sizeof(char *))) != NULL);

  a = arg;
  r = reg;
  *r++ = '^';
  while (*a) {
    if (*a == '*')
      *r++ = '.', *r++ = '*';
    else if (*a == '?')
      *r++ = '.';
    else if (*a == '.')
      *r++ = '\\', *r++ = '.';
    else
      *r++ = *a;
    a++;
  }
  *r++ = '$';
  *r = 0;

  if ((ret = regcomp(&preg, reg, REG_EXTENDED))) {
    char errbuf[100];
    regerror(ret, &preg, errbuf, sizeof(errbuf));
    fprintf(stderr, "Regex compilation failed: %s\n", errbuf);
    free(reg);
    return;
  }

  if ((dir = opendir(".")) == NULL) {
    perror("Error while opening a dir");
    return;
  }
  while ((ent = readdir(dir)) != NULL) {
    if (!regexec(&preg, ent->d_name, 0, NULL, 0)) {
      if (nument == maxent)
        assert((entries = realloc(entries, (maxent *= 2) * sizeof(char *))) !=
               NULL);
      if (ent->d_name[0] == '.') {
        if (arg[0] == '.')
          entries[nument++] = strdup(ent->d_name);
      } else
        entries[nument++] = strdup(ent->d_name);
    }
  }
  regfree(&preg);
  closedir(dir);

  qsort(entries, nument, sizeof(char *), pstrcmp);
  for (i = 0; i < nument; i++)
    insertarg(currentsimplecmd, entries[i]);
  free(entries);
}
