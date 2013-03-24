/*! \file validator.h
 * \brief Declaration of a custom validators for \a optcalex commandline
 * parsing with boost program_options.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 23/03/2013
 * 
 * Purpose: Declaration of a custom validator for a \a optcalex commandline
 * parsing with boost program_options.
 *
 * ----
 * This file is part of optnonlin.
 *
 * optcalex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * optcalex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with optnonlin.  If not, see <http://www.gnu.org/licenses/>.
 * ----
 * 
 * Copyright (c) 2013 by Daniel Armbruster
 * 
 * REVISIONS and CHANGES 
 * 23/03/2013  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */

#include <vector>
#include <string>
#include <memory>
#include <boost/program_options.hpp>
#include <calexxx/systemparameter.h>
#include <calexxx/subsystem.h>
 
#ifndef _OPTCALEX_VALIDATOR_H_
#define _OPTCALEX_VALIDATOR_H_ 

namespace opt = optimize;

namespace
{
  /*!
   * utility function to determine the nth occurrency of a character in a
   * string
   *
   * \param str string to examine
   * \param sep character to search for
   * \int n number of occurrence
   *
   * \return position in str
   */
  int findIndexOfNthOccurrence(std::string const& str, char sep, int n);

} // namespace (unnamed)

/* Put the custom validator for boost program_options into the same namespace
 * the classes are available.
 * see: http://www.c-plusplus.de/forum/252716
 */
namespace calex
{
  namespace
  {
    /* --------------------------------------------------------------------- */
    /*!
     * utility function to convert a \c commandline argument containing a
     * SystemParameter into a calex grid standard system parameter or rather
     * into a grid system parameter.\n
     *
     * \param str string to convert
     *
     * \return shared pointer to a system parameter
     */
     std::shared_ptr<SystemParameter> systemParameterParser(
        std::string const& str);

  } // namespace unnamed

  /*!
   * custom validator parsing a \a libcalexxx calex::SystemParameter
   * commandline argument 
   * <a href="http://www.boost.org/doc/libs/release/libs/program_options/">
   * Boost Program Options</a> library.
   *
   * The syntax is either
   * \code
   * --param=nam|val|unc
   * \endcode
   * for a common system parameter or 
   * \code
   * --param=nam|start;end;delta|unc
   * \endcode
   * if the system parameter also will be treated as a grid system parameter.
   */
  void validate(boost::any& v, const std::vector<std::string>& values,
      std::shared_ptr<SystemParameter>*, int);

  /*!
   * custom validator parsing a \a libcalexxx calex::FirstOrderSubsystem
   * commandline argument 
   * <a href="http://www.boost.org/doc/libs/release/libs/program_options/">
   * Boost Program Options</a> library.
   *
   * The syntax is either
   * \code
   * --first-order=[HP|LP]|nam|val|unc
   * \endcode
   * or
   * \code
   * --first-order=[HP|LP]|nam|start;end;delta|unc
   * \endcode
   * for a grid system parameter.
   */
  void validate(boost::any& v, const std::vector<std::string>& values, 
      FirstOrderSubsystem*, int);

  /*!
   * custom validator parsing a \a libcalexxx calex::SecondOrderSubsystem
   * commandline argument 
   * <a href="http://www.boost.org/doc/libs/release/libs/program_options/">
   * Boost Program Options</a> library.
   *
   * The syntax is either
   * \code
   * --second-order=[HP|LP|BP]|nam|val|unc|nam|val|unc
   * \endcode
   * or
   * \code
   * --second-order=[HP|LP|BP]|nam|start;end;delta|unc|nam|val|unc
   * \endcode
   * or
   * \code
   * --second-order=[HP|LP|BP]|nam|val|unc|nam|start;end;delta|unc
   * \endcode
   * or
   * \code
   * --second-order=[HP|LP|BP]|nam|start;end;delta|unc|nam|start;end;delta|unc
   * \endcode
   *
   * In a second order subsystem the first parameter refers to the
   * eigenperiod of the subsystem and the second parameter to the damping.
   */
  void validate(boost::any& v, const std::vector<std::string>& values, 
      SecondOrderSubsystem*, int);
}

#endif // include guard

/* ----- END OF validator.h  ----- */
