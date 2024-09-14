%{
	#include "flesh.h"
%}

%token NEWLINE, OUT_REDIRECT, IN_REDIRECT, APPEND_REDIRECT, FD_OUT_REDIRECT, FD_APPEND_REDIRECT, PIPE, BACKGROUND, WORD, INVALID

%%

goal: command_list
    ;

arg_list:
	arg_list WORD { currentsimplecmd->insertarg($2); }
	| /* empty */
	;
cmd_and_args:
	WORD arg_list
	;
pipe_list:
	pipe_list PIPE cmd_and_args
	| cmd_and_args
	;
io_modifier:
	OUT_REDIRECT WORD
	| IN_REDIRECT WORD
	| APPEND_REDIRECT WORD
	| FD_OUT_REDIRECT WORD
	| FD_APPEND_REDIRECT WORD
	;
io_modifier_list:
	io_modifier_list io_modifier
	| /* empty */
	;
command_line:
	pipe_list io_modifier_list background_opt NEWLINE
	| NEWLINE
	| error NEWLINE{yyerrok;}
	| INVALID NEWLINE { printf("Invalid command\n"); yyerrok; }
command_list:
	command_list command_line
	| command_line
	;
background_opt:
	BACKGROUND
	| /* empty */
	;
argument:
	WORD { expandwildcards($1); }

%%
