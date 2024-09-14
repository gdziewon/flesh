%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n { return NEWLINE; }
[ \t] {}
">" { return OUT_REDIRECT; }
"<" { return IN_REDIRECT; }
">>" { return APPEND_REDIRECT; }
">&" { return FD_OUT_REDIRECT; }
">>&" { return FD_APPEND_REDIRECT; }
"|" { return PIPE; }
"&" { return BACKGROUND; }
[^ \t\n][^ \t\n]* {
	yyval.string_val = strdup(yytext);
	return WORD;
}
. {
	return INVALID;
}


%%
