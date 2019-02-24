%{
/*
 * AIM tm_converter
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <script2txt_parser.h>
%}

////////////////////////////////////////

// general settings
%require "3.0"
//%debug
%start file
%locations
%verbose
//%no-lines
%define parse.error verbose

////////////////////////////////////////

// c++ skeleton and options
%skeleton "lalr1.cc"
%define api.value.type variant
%define api.token.constructor // C++ style of handling variants
%define parse.assert // check C++ variant types
%code provides { #include <primitives/helper/bison_yy.h> }
%parse-param { MY_PARSER_DRIVER &driver } // param to yy::parser() constructor (the parsing context)

%code requires // forward decl of C++ driver (our parser) in HPP
{
#include <Polygon4/DataManager/Schema/Context.h>

#include <set>
}

%code provides
{
struct MY_PARSER_DRIVER : MY_PARSER
{
    void setContext(Context &&ctx) { context = std::move(ctx); }
    const Context &getContext() const { return context; }

    Context context;
    std::set<std::string> functions;
};
}

////////////////////////////////////////

// tokens and types
%token EOQ 0 "end of file"
%token ERROR_SYMBOL
%token L_BRACKET R_BRACKET COMMA QUOTE SEMICOLON COLON POINT
       L_CURLY_BRACKET R_CURLY_BRACKET SHARP R_ARROW EQUAL
       L_SQUARE_BRACKET R_SQUARE_BRACKET ASTERISK
%token IF ELSE NOT AND OR
%token END PROC _PROC

%token <std::string> STRING
%token <int> INTEGER

%type <std::string> string integer number
    object_variable object
    variables variable
    function_name procedure_begin
    parameters parameter
    conds cond condition_body
    function_call

%type <Context> condition condition_begin
    statements statement
    proc_statements proc_statement
    procedure
    global_statements global_statement
    script

////////////////////////////////////////

%%

file: script EOQ
    { driver.setContext(std::move($1)); }
    ;

script: global_statements
    { $$ = std::move($1); }
    ;

global_statements: global_statement
    { $$ = std::move($1); }
    | global_statements global_statement
    {
        auto &ctx = $1;
        ctx.addLine();
        ctx.addWithRelativeIndent($2);
        $$ = std::move(ctx);
    }
    ;

global_statement: function_call
    {
        Context ctx;
        ctx.addLine($1);
        $$ = std::move(ctx);
    }
    | condition
    { $$ = std::move($1); }
    | procedure
    { $$ = std::move($1); }
    | R_CURLY_BRACKET
    { $$ = Context(); }
    | END
    { $$ = Context(); }
    | ERROR_SYMBOL
    { $$ = Context(); }
    | POINT
    { $$ = Context(); }
    | STRING
    { $$ = Context(); }
    | R_BRACKET
    { $$ = Context(); }
    ;

procedure: procedure_begin proc_statements END
    {
        Context ctx;
        ctx.beginBlock($1);
        ctx.addWithRelativeIndent($2);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    | procedure_begin END
    {
        Context ctx;
        ctx.beginBlock($1);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    | procedure_begin L_CURLY_BRACKET statements R_CURLY_BRACKET
    {
        Context ctx;
        ctx.beginBlock($1);
        ctx.addWithRelativeIndent($3);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    ;
procedure_begin: PROC function_name L_BRACKET R_BRACKET
    { $$ = "PROC " + $2 + "()"; }
    | PROC function_name
    { $$ = "PROC " + $2 + "()"; }
    ;

proc_statements: proc_statement
    { $$ = std::move($1); }
    | proc_statements proc_statement
    {
        auto &ctx = $1;
        ctx.addWithRelativeIndent($2);
        $$ = std::move(ctx);
    }
    ;
proc_statement: function_call
    {
        Context ctx;
        ctx.addLine($1);
        $$ = std::move(ctx);
    }
    | _PROC function_call
    {
        Context ctx;
        ctx.addLine("_PROC " + $2);
        $$ = std::move(ctx);
    }
    | condition
    { $$ = std::move($1); }
    | COLON
    { $$ = Context(); }
    | R_BRACKET
    { $$ = Context(); }
    | ERROR_SYMBOL
    { $$ = Context(); }
    ;

statements: statement
    { $$ = std::move($1); }
    | statements statement
    {
        auto &ctx = $1;
        ctx.addWithRelativeIndent($2);
        $$ = std::move(ctx);
    }
    ;
statement: proc_statement
    { $$ = std::move($1); }
    | END
    {
        Context ctx;
        ctx.addLine("END");
        $$ = std::move(ctx);
    }
    ;

function_call: function_name L_BRACKET parameters R_BRACKET
    { $$ = $1 + "(" + $3 + ")"; driver.functions.insert($1); }
    | function_name L_BRACKET parameters COMMA R_BRACKET
    { $$ = $1 + "(" + $3 + ")"; driver.functions.insert($1); }
    | function_name L_BRACKET R_BRACKET
    { $$ = $1 + "()"; driver.functions.insert($1); }
    ;
parameters: parameter
    { $$ = $1; }
    | parameters COMMA parameter
    { $$ = $1 + ", " + $3; }
    ;
parameter: object
    { $$ = $1; }
    | number
    { $$ = $1; }
    | object_variable
    { $$ = $1; }
    | ASTERISK
    { $$ = "*"; }
    ;

condition: condition_begin
    { $$ = std::move($1); }
    | condition_begin ELSE L_CURLY_BRACKET statements R_CURLY_BRACKET
    {
        auto &ctx = $1;
        ctx.beginBlock("else");
        ctx.addWithRelativeIndent($4);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    ;
condition_begin: IF L_BRACKET condition_body R_BRACKET L_CURLY_BRACKET statements R_CURLY_BRACKET
    {
        Context ctx;
        ctx.beginBlock("if (" + $3 + ")");
        ctx.addWithRelativeIndent($6);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    | IF L_BRACKET condition_body L_CURLY_BRACKET statements R_CURLY_BRACKET
    {
        Context ctx;
        ctx.beginBlock("if (" + $3 + ")");
        ctx.addWithRelativeIndent($5);
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    | IF L_BRACKET condition_body R_BRACKET L_CURLY_BRACKET R_CURLY_BRACKET
    {
        Context ctx;
        ctx.beginBlock("if (" + $3 + ")");
        ctx.endBlock();
        $$ = std::move(ctx);
    }
    ;
condition_body: conds
    { $$ = $1; }
    ;
conds: cond
    { $$ = $1; }
    | conds AND cond
    { $$ = $1 + " && " + $3; }
    | conds OR cond
    { $$ = $1 + " || " + $3; }
    ;
cond: object
    { $$ = $1; }
    | object_variable
    { $$ = $1; }
    | function_call
    { $$ = $1; }
    | NOT cond
    { $$ = "!" + $2; }
    ;

object_variable: object POINT variables
    { $$ = $1 + "." + $3; }
    ;
variables: /* empty */
    { $$ = ""; }
    | variable
    { $$ = $1; }
    | variables POINT variable
    { $$ = $1 + "." + $3; }
    ;

function_name: string
    { $$ = $1; }
    ;
object: string
    { $$ = $1; }
    ;
variable: string
    { $$ = $1; }
    | integer
    { $$ = $1; }
    ;

number: integer POINT integer
    { $$ = $1 + "." + $3; }
    | integer
    { $$ = $1; }
    ;

string: STRING
    { $$ = $1; }
    ;
integer: INTEGER
    { $$ = std::to_string($1); }
    ;

%%
