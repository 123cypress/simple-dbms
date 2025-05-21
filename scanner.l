%{
#define YY_NO_UNISTD_H 1
#include <stdio.h>
#include <string.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#include "parser.tab.h"

void yyerror(const char* msg);
%}

%option nounistd
%option never-interactive
%option noyywrap
%option case-insensitive

%%

CREATE          { return CREATE; }
DATABASE        { return DATABASE; }
USE             { return USE; }
DROP            { return DROP; }
TABLE           { return TABLE; }
SHOW            { return SHOW; }
DATABASES       { return DATABASES; }
TABLES          { return TABLES; }
INSERT          { return INSERT; }
INTO            { return INTO; }
VALUES          { return VALUES; }
SELECT          { return SELECT; }
FROM            { return FROM; }
WHERE           { return WHERE; }
UPDATE          { return UPDATE; }
SET             { return SET; }
DELETE          { return DELETE; }
INT             { return INT_TYPE; }
CHAR            { return CHAR_TYPE; }
AND             { return AND; }
OR              { return OR; }

[0-9]+          { 
    yylval.intval = atoi(yytext); 
    return NUMBER; 
}

[a-zA-Z_][a-zA-Z0-9_]*  { 
    strncpy(yylval.strval, yytext, 255);
    yylval.strval[255] = '\0';
    return IDENTIFIER;
}

'[^']*'         { 
    yytext[strlen(yytext)-1] = '\0';
    strncpy(yylval.strval, yytext+1, 255);
    yylval.strval[255] = '\0';
    return STRING;
}

"("             { return LPAREN; }
")"             { return RPAREN; }
"="             { return EQ; }
"<"             { return LT; }
">"             { return GT; }
"!="            { return NE; }
","             { return COMMA; }
";"             { return SEMICOLON; }
"*"             { return ASTERISK; }

[ \t\r]+       ; /* skip whitespace */
\n             { return '\n'; }
--.*\n         ; /* skip SQL comments */
\/\/.*\n       ; /* skip C-style comments */

.              { return ERROR; }

%%