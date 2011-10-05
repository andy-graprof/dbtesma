/******************************************************************************
 *                          dbtesma data generator                             *
 *                                                                             *
 *    Copyright (C) 2011  Andreas Henning                                      *
 *                                                                             *
 *    This program is free software: you can redistribute it and/or modify     *
 *    it under the terms of the GNU General Public License as published by     *
 *    the Free Software Foundation, either version 3 of the License, or        *
 *    (at your option) any later version.                                      *
 *                                                                             *
 *    This program is distributed in the hope that it will be useful,          *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *    GNU General Public License for more details.                             *
 *                                                                             *
 *    You should have received a copy of the GNU General Public License        *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/
 
#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "configvalidator.h"

namespace CONF {

class Configparser
{

	/**
	Config Data Class
	tasks:
		cumulates input from tesmafile
	**/

	/** enumeration containing the possible parsing contexts **/
	enum CONTEXT
	{
		NONE,
		TABLE,
		COLUMN,
		FUNCDEP
	} typedef CONTEXT;

	
public:

	Configparser(std::string &in) : _filename(in), _in(new std::ifstream(in.c_str())), _context(NONE) {};
	Configparser(const Configparser &obj) : _filename(obj._filename), _in(obj._in), _context(obj._context) {};
	Configparser& operator=(const Configparser&);
	~Configparser();

	/** main parsing method - starts parsing, and afterward starts validation **/
	bool parseAndValidate(DATA::Schema*);

private:
	
	/** top level parsing **/
	bool parse(DATA::Schema*);

	/** line-scoped parsing - dispatches to context line-scoped methods **/
	bool processLine(std::string&, DATA::Schema*);
	
	/** contexted line-scoped parsing methods **/
	bool processLineContextNone(std::string&, DATA::Schema*);
	bool processLineContextTable(std::string&, DATA::Schema*);
	bool processLineContextColumn(std::string&, DATA::Schema*);
	bool processLineContextFuncDep(std::string&, DATA::Schema*);

	std::string _filename;
	std::ifstream *_in;
	CONTEXT _context;
};

} // namespaces

#endif // CONFIGPARSER_H