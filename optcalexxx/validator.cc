/*! \file validator.cc
 * \brief Declaration of a custom validators for \a optcalex commandline
 * parsing with boost program_options. (implementation)
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id: $
 * \author Daniel Armbruster
 * \date 23/03/2013
 * 
 * Purpose: Declaration of a custom validators for \a optcalex commandline
 * parsing with boost program_options. (implementation)  
 *
 * ----
 * This file is part of optcalex.
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
 * along with optcalex.  If not, see <http://www.gnu.org/licenses/>.
 * ----
 * 
 * Copyright (c) 2013 by Daniel Armbruster
 * 
 * REVISIONS and CHANGES 
 * 23/03/2013  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */
 
#include <sstream>
#include "validator.h"
#include <boost/regex.hpp>

namespace po = boost::program_options;

namespace 
{

  int findIndexOfNthOccurrence(std::string const& str, char sep, int n)
  {
    int cnt = 0;
    for (unsigned int j=0; j < str.size(); ++j)
    {
      if (str[j] == sep)
      {
        if (++cnt == n) { return j; }
      }
    }
    return -1;
  } // function findIndexOfNthOccurrence

} // namespace (unnamed)


namespace calex
{
  namespace
  {

    std::shared_ptr<SystemParameter> systemParameterParser(
        std::string const& str)
    {
      std::string id;
      double val, unc, start, end, delta;
      char c;
      // extract nam (ID parameter)
      size_t pos = str.find("|"); 
      if (std::string::npos !=  pos && pos == 3)
      {
        id = str.substr(0,pos);
      } else
      {
        throw validation_error(validation_error::invalid_option_value);
      }
      // detect if system parameter will be treated additionally as a grid
      // system parameter
      std::istringstream iss(str.substr(pos+1));
      if (std::string::npos != str.find(";"))
      {
        iss >> std::fixed >> start >> c >> end >> c >> delta >> c >> unc;
        if (c != '|')
        {
          throw validation_error(validation_error::invalid_option_value);
        }
        return std::shared_ptr<SystemParameter>(
            new GridSystemParameter(id, unc, id, start, end, delta));
      } else
      {
        iss >> val >> c >> unc;
        if (c != '|')
        {
          throw validation_error(validation_error::invalid_option_value);
        }
        return std::shared_ptr<SystemParameter>(
              new SystemParameter(id, val, unc));
      }
    } // function systemParameterParser

  } // namespace (unnamed)


  void validate(boost::any& v, const std::vector<std::string>& values,
      std::shared_ptr<SystemParameter>*, int);
  {
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const string& str = validators::get_single_string(values);

    v = boost::any(::systemParameterParser(str));
  } // function validate - SystemParameter / GridSystemParameter


  void validate(boost::any& v, const std::vector<std::string>& values, 
      FirstOrderSubsystem*, int)
  {
    const string& str = validators::get_single_string(values);
    std::string type(str.substr(0,2));
    std::shared_ptr<SystemParameter> per_ptr(
        ::systemParameterParser(str.substr(3)));

    if ("LP" == type)
    {
       v = boost::any(FirstOrderSubsystem(LP, per_ptr));
    } else
    if ("HP" == type)
    {
       v = boost::any(FirstOrderSubsystem(HP, per_ptr));
    } else 
    { 
      throw validation_error(validation_error::invalid_option_value);
    }
  } // function validate - FirstOrderSubsystem


  void validate(boost::any& v, const std::vector<std::string>& values, 
      SecondOrderSubsystem*, int)
  {
    std::string type(str.substr(0,2));
    std::string param_str(str.substr(str.find("|")+1,
      ::findIndexOfNthOccurrence(str, '|', 4)-str.find("|")-1));
    std::shared_ptr<SystemParameter> per_ptr(
        systemParameterParser(param_str, "per"));
    param_str = str.substr(type.size()+1+param_str.size()+1);
    std::shared_ptr<SystemParameter> dmp_ptr(
        systemParameterParser(param_str, "dmp"));

    std::shared_ptr<CalexSubsystem> ret_ptr;
    if ("LP" == type)
    {
      v = boost::any(SecondOrderSubsystem(LP, per_ptr, dmp_ptr));
    } else
    if ("HP" == type)
    {
      v = boost::any(SecondOrderSubsystem(HP, per_ptr, dmp_ptr));
    } else
    if ("BP" == type)
    {
      v = boost::any(SecondOrderSubsystem(BP, per_ptr, dmp_ptr));
    } else
    { 
      throw validation_error(validation_error::invalid_option_value);
    }
  } // function validate - SecondOrderSubsystem

} // namespace calex

/* ----- END OF validator.cc  ----- */
