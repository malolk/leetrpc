%option noyywrap
%{
#include "context.h"    
#include "conv.tab.hh"

#include <stdio.h>
#define YY_DECL int yylex(yy::conv::semantic_type* yylval, \
  yy::conv::location_type* yylloc, yy::Context& ctx)

#define YY_USER_ACTION yylloc->columns(yyleng);
typedef yy::conv::token token;
%}

%%

%{
  yylloc->step();   
%}

"service"                                        { return token::CLASS; }
"int"                                            |
"string"                                         |
vector<(int|double|string)>                      |
set<(int|double|string)>                         |
list<(int|double|string)>                        |
map<(int|double|string).*,.*(int|double|string)> |
"double"                                         { yylval->t = strdup(yytext); 
                                                   return token::TYPE; 
                                                 }
"{"                                              |
"("                                              |
")"                                              |
";"                                              |
","                                              { return yytext[0]; }
[a-zA-Z]+                                        { yylval->t = strdup(yytext); 
                                                   return token::NAME; 
                                                 }
"}"                                              { return token::EOL; }
\n                                               { yylloc->lines(1); yylloc->step(); }
"//".*                                           |
[ \t]                                            { yylloc->step(); }
.                                                { fprintf(stderr, "Invalid character %c\n", *yytext); }

%%
