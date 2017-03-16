%language "C++"
%defines
%locations

%define parser_class_name { conv }

%{
#include "context.h"
#include "ast.h"

#include <iostream>
#include <string>
using namespace std;
%}

%parse-param { Context &ctx }
%lex-param   { Context &ctx }

%define parse.error verbose

%union {
  const char* t;
  struct AST* s;
}

%token <t> TYPE
%token <t> NAME
%token CLASS
%token EOL

%type <s> class
%type <s> body
%type <s> func
%type <s> arglist

%{
extern int yylex(yy::conv::semantic_type* yylval, 
                 yy::conv::location_type* yylloc,
                 yy::Context& ctx);
%}

%initial-action {
  @$.begin.filename = @$.end.filename = new std::string("stdin");
}

%%

input: class EOL {
       ctx.Start($1);
       return 0;
     }
;

class: CLASS NAME '{' body         { $$ = NewClass($2, $4); }
     | CLASS NAME '{'              { $$ = NewClass($2, NULL); }
;

body: func                         { $$ = NewBody(NULL, $1); }
    | body  func                   { $$ = NewBody($1, $2); }
;

func: TYPE NAME '(' arglist ')' ';'{ $$ = NewFunc($4, $1, $2); }
    | TYPE NAME '(' ')' ';'        { $$ = NewFunc(NULL, $1, $2); }
;

arglist: TYPE NAME                 { $$ = NewArg(NULL, $1, $2); } 
       | arglist ',' TYPE NAME     { $$ = NewArg($1, $3, $4); }
;

%%

int main() {
  yy::Context ctx;
  yy::conv parser(ctx);
  int ret = parser.parse();
  return ret;
}

namespace yy {
  void conv::error(location const& loc, const std::string& s) {
    std::cerr << "error at " << loc << ": " << s << std::endl;
  }
}

