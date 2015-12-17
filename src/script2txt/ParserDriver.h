/*
 * Polygon-4 script2txt
 * Copyright (C) 2015-2016 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <set>
#include <string>
#include <vector>

#include <grammar.hpp>

class ParserDriver
{
public:
    ParserDriver();

    yy::parser::symbol_type lex();
    int parse(const std::string &s);

    void error(const yy::location &l, const std::string &m);
    void error(const std::string &m);

    void setContext(Context &&ctx) { context = std::move(ctx); }
    const Context &getContext() const { return context; }
    
    // lex & parse
private:
    void *scanner;
    yy::location location;
    bool debug;

    // data
private:
    Context context;

    // other
public:
    std::set<std::string> functions;
};
