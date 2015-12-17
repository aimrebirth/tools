%{
#pragma warning(disable: 4005)
#include <string>

#include "grammar.hpp"

#define YY_USER_ACTION loc.columns(yyleng);

#define PUSH_STATE(x) BEGIN(x)
#define POP_STATE() BEGIN(0)

#define YY_DECL yy::parser::symbol_type yylex(yyscan_t yyscanner, yy::location &loc)

#define MAKE(x) yy::parser::make_ ## x(loc)
#define MAKE_VALUE(x, v) yy::parser::make_ ## x((v), loc)
%}

%option nounistd
%option yylineno
%option nounput
%option batch
%option never-interactive
%option reentrant
%option noyywrap


DIGIT       [0-9]
DIGITS      {DIGIT}{DIGIT}*
INTEGER     {DIGITS}[Ff]?

STRING      [[:alpha:]_-][[:alnum:]_-]*


%x user_string


%%

%{
    // Code run each time yylex is called.
    loc.step();
%}

#.*/\n                  ; // ignore comments

[ \t]+                  loc.step();
\r                      loc.step();
\n                      {
                            loc.lines(yyleng);
                            loc.step();
                        }
                        
";"                     return MAKE(SEMICOLON);
":"                     return MAKE(COLON);
"("                     return MAKE(L_BRACKET);
")"                     return MAKE(R_BRACKET);
"{"                     return MAKE(L_CURLY_BRACKET);
"}"                     return MAKE(R_CURLY_BRACKET);
"["                     return MAKE(L_SQUARE_BRACKET);
"]"                     return MAKE(R_SQUARE_BRACKET);
","                     return MAKE(COMMA);
"\."                    return MAKE(POINT);
"->"                    return MAKE(R_ARROW);
"="                     return MAKE(EQUAL);
"\*"                    return MAKE(ASTERISK);

IF                      { return MAKE(IF); }
ELSE                    { return MAKE(ELSE); }
"!"                     { return MAKE(NOT); }
"&"                     { return MAKE(AND); }
"|"                     { return MAKE(OR); }
"||"                    { return MAKE(OR); }
END                     { return MAKE(END); }
PROC                    { return MAKE(PROC); }
_PROC                   { return MAKE(_PROC); }

{INTEGER}               { return MAKE_VALUE(INTEGER, std::stoi(yytext)); }
{STRING}                { return MAKE_VALUE(STRING, yytext); }

\"                      { PUSH_STATE(user_string);  return MAKE(QUOTE); }
<user_string>\"         { POP_STATE();              return MAKE(QUOTE); }
<user_string>(?:[^"\\]|\\.)*/\" {
    int n = 0;
    char *p = yytext;
    while ((p = strstr(p, "\n"))++ != 0)
        n++;
    if (n)
    {
        loc.lines(n);
        loc.step();
    }
    return MAKE_VALUE(STRING, yytext);
}

.                       { /*driver.error(loc, "invalid character");*/ return MAKE(ERROR_SYMBOL); }
<<EOF>>                 return MAKE(EOQ);

%%
