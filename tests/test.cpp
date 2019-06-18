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

#include "test.h"

namespace tests {

/** public ******************************************************************************************************/

	void Test::run()
	{
		registerTests();

		helper::UiHelper::printlnspaced(_className.c_str(), 5);
		std::vector<std::pair<bool (Test::*)(), std::string> >::iterator i = _testCases.begin();
		for(;i != _testCases.end(); i++)
		{
			helper::UiHelper::startProgress((*i).second.c_str());
			if((*this.*(*i).first)())
				helper::UiHelper::overrok();
			else
				helper::UiHelper::overrfail();
		}
	}

/** protected ***************************************************************************************************/


} // namespaces