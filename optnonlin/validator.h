/*! \file validator.h
 * \brief Declaration of a custom validator for a \a liboptimizexx
 * StandardParameter for commandline parsing with boost program_options
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 20/04/2012
 * 
 * Purpose: Declaration of a custom validator for a \a liboptimizexx
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

#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <optimizexx/parameter.h>
 
#ifndef _OPTNONLIN_VALIDATOR_H_
#define _OPTNONLIN_VALIDATOR_H_ 

namespace opt = optimize;

/*!
 * custom validator for a \a liboptimizexx standard parameter for commandline
 * parsing using
 * <a href="http://www.boost.org/doc/libs/release/libs/program_options/">
 * Boost Program Options</a> library.
 */
void validate(boost::any& v, const std::vector<std::string>& values,
              opt::StandardParameter<double>*, int)

#endif // include guard

/* ----- END OF validator.h  ----- */
