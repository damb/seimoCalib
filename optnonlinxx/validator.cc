/*! \file validator.cc
 * \brief Implementaion of a custom validator for a \a liboptimizexx
 * StandardParameter for commandline parsing with boost program_options 
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 20/04/2012
 * 
 * Purpose: Implementaion of a custom validator for a \a liboptimizexx
 * StandardParameter for commandline parsing with boost program_options  
 *
 * ----
 * This file is part of optnonlin.
 *
 * optnonlin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * optnonlin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with optnonlin.  If not, see <http://www.gnu.org/licenses/>.
 * ----
 * 
 * Copyright (c) 2012 by Daniel Armbruster
 * 
 * REVISIONS and CHANGES 
 * 20/04/2012  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */
 
#include <sstream>
#include <algorithm>
#include <iterator>
#include "validator.h"

namespace po = boost::program_options;
namespace opt = optimize;

/* -------------------------------------------------------------------------- */
namespace
{
  //! helper function which converts a string into a double value
  template <typename Ttype=TcoordType>
  double string2X(std::string const& str)
  {
    std::istringstream iss(str);   
    Ttype val;
    if (!(iss >> val))
    {
      throw po::validation_error(po::validation_error::invalid_option_value);
    }
    return val;
  }
} // anonymous namespace

/* -------------------------------------------------------------------------- */
namespace optimize
{
  void validate(boost::any& v, const std::vector<std::string>& values,
                StandardParameter<TcoordType>*, int)
  {

    if (values.size() != 4)
    {
      throw po::validation_error(po::validation_error::invalid_option_value);
    }

    const std::string id = values.at(0);
    if (("c0" != id) && ("c1" != id) && ("T0" != id) && ("h" != id))
    {
      throw po::validation_error(po::validation_error::invalid_option_value);
    }
    std::vector<TcoordType> param_values;
    std::transform(values.begin()+1, values.end(),
        std::back_inserter(param_values), ::string2X);
    v = boost::any(StandardParameter<TcoordType>(
          id, param_values[0], param_values[1], param_values[2]));
  } // function validate
}

/* ----- END OF validator.cc  ----- */
