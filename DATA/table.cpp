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
 
#include "table.h"

namespace DATA {

/** public ******************************************************************************************************/

	Table& Table::operator=(const Table &rhs)
	{
		_attributes = rhs._attributes;
		_columns = rhs._columns;
		_funcdeps = rhs._funcdeps;
		_funcdepGraph = rhs._funcdepGraph;
		_columnsSorted = rhs._columnsSorted;
		_columnPrimaryKeyGroups = rhs._columnPrimaryKeyGroups;
		_out = rhs._out;
		_rows = rhs._rows;
		_rowsToHarden = rhs._rowsToHarden;

		return *this;
	}

	Table::~Table() {}

	void Table::setAttribute(Table::ATTRIBUTES type, std::string &in)
	{
		_attributes[type] = in;
	}

	bool Table::getAttribute(Table::ATTRIBUTES type, std::string &out)
	{
		bool good = _attributes.find(type) != _attributes.end();
		if(good)
			out = _attributes[type];
		return good;
	}	

	void Table::setColumnAttribute(DATA::Column::ATTRIBUTES type, std::string &in)
	{
		_columns.back()->setAttribute(type, in);
	}

	void Table::newColumn()
	{
		DATA::Column *newColumn = new DATA::Column();
		_columns.push_back(newColumn);
	}
	
	void Table::newFuncdep(bool fkdepFlag, bool reverseFDFlag)
	{
		DATA::Funcdep *newFuncdep = new DATA::Funcdep(fkdepFlag, reverseFDFlag);
		_funcdeps.push_back(newFuncdep);
	}
	
	void Table::setFuncdepLhs(std::string lhs)
	{
		_funcdeps.back()->setLhs(lhs);
	}

	void Table::setFuncdepRhs(std::string rhs)
	{
		_funcdeps.back()->setRhs(rhs);
	}

	void Table::addColumnToPrimaryKeyGroup(std::string& group, DATA::Column *col)
	{
		if(_columnPrimaryKeyGroups.find(group) == _columnPrimaryKeyGroups.end())
			_columnPrimaryKeyGroups[group] = std::vector<DATA::Column*>();

		_columnPrimaryKeyGroups[group].push_back(col);
	}

	void Table::normalize()
	{
		std::vector<DATA::Column*>::iterator i = _columns.begin();
		for(;i != _columns.end(); i++)
		{
			int n = getKeyGroupPos(*i);
			int m = getKeyGroupSize(*i);
			(*i)->setupMulticolumKeyConstraints(n, m, _rows);
		}
	}
	
	bool Table::sortColumns()
	{
		fillColumnsGraph();
		return buildColumnsSequence();
	}
	
	bool Table::resortColumns()
	{
		refillColumnsGraph();
		return rebuildColumnsSequence();
	}

	void Table::print(bool noHeader, bool hardenFds)	
	{
		openOutstream();

		if(!noHeader)
			printHeader();

		nextNoIncrement();
		printRow();
		unsigned long long i;
		for(i = 1; i < _rows; i++)
		{
			next();
			printRow();
		}
		
		if(hardenFds)
			harden();
		
		closeOutstream();
	}

	void Table::setRowCount(unsigned long long rows)
	{
		_rows = rows;
	}

	unsigned long long Table::getRowCount()
	{
		return _rows;
	}

	DATA::Column* Table::findColumnByName(std::string &in)
	{
		std::vector<DATA::Column*>::iterator i;
		std::string columnName;
		for(i = _columns.begin(); i != _columns.end(); i++)
		{
			(*i)->getAttribute(DATA::Column::ATTR_NAME, columnName);
			if(!columnName.compare(in))
				return (*i);
		}
		
		return NULL;
	}

/** private *****************************************************************************************************/

	void Table::openOutstream()
	{
		std::string name = _attributes[Table::ATTR_NAME] + ".csv";
		_out->open(name.c_str());
		std::vector<DATA::Column*>::iterator i = _columns.begin();
		for(;i != _columns.end(); i++)
			(*i)->populateOutstream(_out);			
	}

	void Table::closeOutstream()
	{
		_out->close();
	}	

	void Table::printHeader()
	{
		std::vector<DATA::Column*>::iterator i = _columns.begin();
		std::string st;
		(*i)->getAttribute(DATA::Column::ATTR_NAME, st);
		(*_out) << st;
		i++;
		for(;i != _columns.end(); i++)
		{
			(*i)->getAttribute(DATA::Column::ATTR_NAME, st);			
			(*_out) << "," << st;
		}
		(*_out) << std::endl;
	}
	
	void Table::next()
	{
		std::vector<DATA::Column*>::iterator i = _columnsSorted.begin();
		for(; i != _columnsSorted.end(); i++)
			(*i)->next();
	}
	
	void Table::nextNoIncrement()
	{
		std::vector<DATA::Column*>::iterator i = _columnsSorted.begin();
		for(; i != _columnsSorted.end(); i++)
			(*i)->nextNoIncrement();
	}
	
	void Table::printRow()
	{
		std::vector<DATA::Column*>::iterator i = _columns.begin();

		(*i)->print();
		i++;
		for(; i != _columns.end(); i++)
		{
			(*_out) << ",";
			(*i)->print();
		}
		(*_out) << std::endl;
	}

	void Table::harden()
	{
		/** build lookup index **/
		std::map<DATA::Column*, unsigned int> indexByColumn = std::map<DATA::Column*, unsigned int>();
		std::vector<DATA::Column*> rhsCols = std::vector<DATA::Column*>();
		
		std::vector<DATA::Funcdep*>::iterator fd = _funcdeps.begin();
		int lhs_index = 1;
		for(; fd != _funcdeps.end(); fd++)
		{
			indexByColumn[*((*fd)->rhs_begin())] = 0;
			rhsCols.push_back(*((*fd)->rhs_begin()));
			std::vector<DATA::Column*>::iterator lhs = (*fd)->lhs_begin();
			for(; lhs != (*fd)->lhs_end(); lhs++)
			{
				indexByColumn[*lhs] = lhs_index;
				lhs_index++;
			}
		}
		
		unsigned int index;
		std::vector<DATA::Column*>::iterator c;
		/** first pass **/
		for(index = 1; index <= _rowsToHarden; index++)
		{
			for(c = _columns.begin(); c != _columns.end(); c++)
			{
				if(indexByColumn.find(*c) == indexByColumn.end())
					(*c)->nextHarden();
				else if(indexByColumn[*c] == index)
					(*c)->incrementTemp();
			}
			for(c = rhsCols.begin(); c != rhsCols.end(); c++)
				(*c)->next();

			c = _columns.begin();
			
			(*c)->print();
			if(indexByColumn[*c] == index)
				(*c)->resetTemp();
			c++;
			for(; c != _columns.end(); c++)
			{
				(*_out) << ",";
				(*c)->print();
				if(indexByColumn[*c] == index)
					(*c)->resetTemp();
			}
			(*_out) << std::endl;
		}
		/** second pass **/
		for(index = 1; index <= _rowsToHarden; index++)
		{
			for(c = _columns.begin(); c != _columns.end(); c++)
			{
				if(indexByColumn.find(*c) == indexByColumn.end())
					(*c)->nextHarden();
				else if(indexByColumn[*c] == index)
					(*c)->incrementTempSmall();
			}
			for(c = rhsCols.begin(); c != rhsCols.end(); c++)
				(*c)->next();

			c = _columns.begin();
			
			(*c)->print();
			c++;
			for(; c != _columns.end(); c++)
			{
				(*_out) << ",";
				(*c)->print();
				if(indexByColumn[*c] == index)
					(*c)->resetTemp();
			}
			(*_out) << std::endl;
		}
	}
	
	int Table::getKeyGroupPos(DATA::Column *col)
	{
		std::map<std::string, std::vector<DATA::Column*> >::iterator i;
		for(i = _columnPrimaryKeyGroups.begin(); i != _columnPrimaryKeyGroups.end(); i++)
		{
			std::vector<DATA::Column*>::iterator j;
			int k = 0;
			for(j = (*i).second.begin(); j != (*i).second.end(); j++)
			{
				if((*j) == col)
					return k;
				k++;
			}
		}
		return -1;
	}

	int Table::getKeyGroupSize(DATA::Column *col)
	{
		std::map<std::string, std::vector<DATA::Column*> >::iterator i;
		for(i = _columnPrimaryKeyGroups.begin(); i != _columnPrimaryKeyGroups.end(); i++)
		{
			std::vector<DATA::Column*>::iterator j;
			int k = 0;
			for(j = (*i).second.begin(); j != (*i).second.end(); j++)
			{
				if((*j) == col)
					return (*i).second.size();
				k++;
			}
		}
		return -1;
	}
	
	void Table::fillColumnsGraph()
	{
		std::vector<DATA::Column*>::iterator i = _columns.begin();
		for(; i != _columns.end(); i++)
			_funcdepGraph->push_back(*i);
			
		std::vector<DATA::Funcdep*>::iterator j = _funcdeps.begin();
		for(; j != _funcdeps.end(); j++)
			_funcdepGraph->push_back(*j);
	}
	
	bool Table::buildColumnsSequence()
	{
		DATA::Column *c = _funcdepGraph->pop_back();
		while(c)
		{	
			_columnsSorted.push_back(c);
			registerColumnReferences(c);
			c = _funcdepGraph->pop_back();
		}
		
		return _columns.size() == _columnsSorted.size();
	}

	void Table::refillColumnsGraph()
	{
		_funcdepGraph->clear();
		fillColumnsGraph();
	}
	
	bool Table::rebuildColumnsSequence()
	{
		_columnsSorted.clear();
	
		DATA::Column *c = _funcdepGraph->pop_back();
		while(c)
		{	
			_columnsSorted.push_back(c);
			registerFKColumnReferences(c);
			c = _funcdepGraph->pop_back();
		}
		
		return _columns.size() == _columnsSorted.size();
	}

	void Table::registerColumnReferences(DATA::Column *c)
	{
		std::vector<DATA::Funcdep*>::iterator i = _funcdeps.begin();
		for(; i != _funcdeps.end(); i++)
		{
			if(*((*i)->rhs_begin()) == c)
			{
				if((*i)->isReverseFD())
					c->registerReverseReferences((*i)->lhs());
				else
					c->registerReferences((*i)->lhs());
			}
		}
	}

	void Table::registerFKColumnReferences(DATA::Column *c)
	{
		std::vector<DATA::Funcdep*>::iterator i = _funcdeps.begin();
		for(; i != _funcdeps.end(); i++)
		{
			if(*((*i)->rhs_begin()) == c && (*i)->isFKDep())
				c->registerFKReferences((*i)->lhs());
		}
	}
	
} // namespaces


