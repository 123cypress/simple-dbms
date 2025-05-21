%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DBMS.h"

extern int yylex();
void yyerror(const char *s);
extern DBMS* g_dbms;
%}

%union {
    int intval;
    char strval[256];
}

%token <strval> IDENTIFIER STRING
%token <intval> NUMBER

%token CREATE DROP USE SHOW
%token DATABASE DATABASES
%token TABLE TABLES
%token INSERT INTO VALUES
%token SELECT FROM WHERE
%token UPDATE SET
%token DELETE
%token INT_TYPE CHAR_TYPE
%token AND OR
%token EQ LT GT NE
%token LPAREN RPAREN COMMA SEMICOLON
%token ASTERISK
%token ERROR

%type <strval> column_name_list
%type <strval> value_list
%type <strval> value
%type <strval> select_expr
%type <strval> table_references
%type <strval> condition
%type <strval> opt_where
%type <strval> assignment_list
%type <strval> opt_semicolon
%type <strval> column_defs
%type <strval> column_def
%type <strval> type

%%

commands:
    /* empty */
    | commands command
    ;

command:
    create_database_stmt
    | drop_database_stmt
    | use_database_stmt
    | show_databases_stmt
    | create_table_stmt
    | drop_table_stmt
    | show_tables_stmt
    | insert_stmt
    | select_stmt
    | update_stmt
    | delete_stmt
    | error_recovery
    ;

error_recovery:
    error SEMICOLON    { yyerrok; printf("Error parsing SQL statement\n"); }
    | error '\n'       { yyerrok; printf("Error parsing SQL statement\n"); }
    | ERROR           { yyerrok; }
    ;

create_database_stmt:
    CREATE DATABASE IDENTIFIER opt_semicolon
    { 
        g_dbms->createDatabase($3); 
    }
    ;

drop_database_stmt:
    DROP DATABASE IDENTIFIER opt_semicolon
    { 
        g_dbms->dropDatabase($3); 
    }
    ;

use_database_stmt:
    USE IDENTIFIER opt_semicolon
    { 
        g_dbms->useDatabase($2); 
    }
    ;

show_databases_stmt:
    SHOW DATABASES opt_semicolon
    { 
        g_dbms->showDatabases(); 
    }
    ;

show_tables_stmt:
    SHOW TABLES opt_semicolon
    { 
        g_dbms->showTables(); 
    }
    ;

create_table_stmt:
    CREATE TABLE IDENTIFIER LPAREN column_defs RPAREN opt_semicolon
    { 
        g_dbms->createTable($3, $5); 
    }
    ;

column_defs:
    column_def                      
    { 
        strcpy($$, $1); 
    }
    | column_defs COMMA column_def  
    { 
        snprintf($$, sizeof($$), "%s,%s", $1, $3); 
    }
    ;

column_def:
    IDENTIFIER type                        
    { 
        snprintf($$, sizeof($$), "%s %s", $1, $2); 
    }
    | IDENTIFIER type LPAREN NUMBER RPAREN 
    { 
        snprintf($$, sizeof($$), "%s %s(%d)", $1, $2, $4); 
    }
    ;

type:
    INT_TYPE    { strcpy($$, "INT"); }
    | CHAR_TYPE { strcpy($$, "CHAR"); }
    ;

drop_table_stmt:
    DROP TABLE IDENTIFIER opt_semicolon
    { 
        g_dbms->dropTable($3); 
    }
    ;

insert_stmt:
    INSERT INTO IDENTIFIER LPAREN column_name_list RPAREN VALUES LPAREN value_list RPAREN opt_semicolon
    { 
        g_dbms->insertInto($3, $5, $9); 
    }
    | INSERT INTO IDENTIFIER VALUES LPAREN value_list RPAREN opt_semicolon
    { 
        g_dbms->insertInto($3, "", $6); 
    }
    ;

select_stmt:
    SELECT select_expr FROM table_references opt_where opt_semicolon
    { 
        g_dbms->selectFrom($4, $2, $5); 
    }
    ;

select_expr:
    ASTERISK                { strcpy($$, "*"); }
    | column_name_list     { strcpy($$, $1); }
    ;

table_references:
    IDENTIFIER                          
    { 
        strcpy($$, $1); 
    }
    | table_references COMMA IDENTIFIER 
    { 
        snprintf($$, sizeof($$), "%s,%s", $1, $3); 
    }
    ;

opt_where:
    /* empty */        { strcpy($$, ""); }
    | WHERE condition  { strcpy($$, $2); }
    ;

condition:
    IDENTIFIER EQ value     { snprintf($$, sizeof($$), "%s=%s", $1, $3); }
    | IDENTIFIER GT value   { snprintf($$, sizeof($$), "%s>%s", $1, $3); }
    | IDENTIFIER LT value   { snprintf($$, sizeof($$), "%s<%s", $1, $3); }
    | IDENTIFIER NE value   { snprintf($$, sizeof($$), "%s!=%s", $1, $3); }
    | LPAREN condition RPAREN { snprintf($$, sizeof($$), "(%s)", $2); }
    | condition AND condition { snprintf($$, sizeof($$), "%s AND %s", $1, $3); }
    | condition OR condition  { snprintf($$, sizeof($$), "%s OR %s", $1, $3); }
    ;

update_stmt:
    UPDATE IDENTIFIER SET assignment_list opt_where opt_semicolon
    { 
        g_dbms->update($2, $4, $5); 
    }
    ;

assignment_list:
    IDENTIFIER EQ value                          
    { 
        snprintf($$, sizeof($$), "%s=%s", $1, $3); 
    }
    | assignment_list COMMA IDENTIFIER EQ value  
    { 
        snprintf($$, sizeof($$), "%s,%s=%s", $1, $3, $5); 
    }
    ;

delete_stmt:
    DELETE FROM IDENTIFIER opt_where opt_semicolon
    { 
        g_dbms->deleteFrom($3, $4); 
    }
    ;

column_name_list:
    IDENTIFIER                          
    { 
        strcpy($$, $1); 
    }
    | column_name_list COMMA IDENTIFIER 
    { 
        snprintf($$, sizeof($$), "%s,%s", $1, $3); 
    }
    ;

value_list:
    value                   
    { 
        strcpy($$, $1); 
    }
    | value_list COMMA value 
    { 
        snprintf($$, sizeof($$), "%s,%s", $1, $3); 
    }
    ;

value:
    NUMBER  { snprintf($$, sizeof($$), "%d", $1); }
    | STRING { snprintf($$, sizeof($$), "'%s'", $1); }
    ;

opt_semicolon:
    /* empty */    { strcpy($$, ""); }
    | SEMICOLON    { strcpy($$, ";"); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}