/*! \file util.cc
 * \brief Implementation of utility function of optnonlin.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: Implementation of utility function of optnonlin. 
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
 * 19/04/2012  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */

#include <cmath>
#include "util.h"

namespace util
{

  /* ----------------------------------------------------------------------- */
  void dif(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double dt, double time_constant)
  {
    if (series.size() != result_series.size())
    {
      throw std::string("Inconsistant series size.");
    }

    if (0 == time_constant) { time_constant = 1.; }
    double denominator = 2.*dt / time_constant;
    size_t j;
    for (j=series.f()+1; j<series.l(); ++j)
    {
      result_series[j] = (series[j+1]-series[j-1]) / denominator;
    }
    result_series[0] = result_series[1];
    ++j;
    result_series[j] = result_series[j-1];
  } // function dif

  /* ----------------------------------------------------------------------- */
  void dif2(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double dt, double time_constant)
  {
    if (series.size() != result_series.size())
    {
      throw std::string("Inconsistant series size.");
    }

    if (0 == time_constant) { time_constant = 1.; }
    double denominator = pow(dt,2.) / time_constant;
    size_t j;
    for (j=series.f()+1; j<series.l(); ++j)
    {
      result_series[j] = (series[j+1]-2*series[j]+series[j-1]) / denominator;
    }
    result_series[0] = result_series[1];
    ++j;
    result_series[j] = result_series[j-1];
  } // function dif2

  /* ----------------------------------------------------------------------- */
  void square(datrw::Tdseries const& series, datrw::Tdseries& result_series)
  {
    if (series.size() != result_series.size())
    {
      throw std::string("Inconsistant series size.");
    }
    for (size_t j=series.f(); j<=series.l(); ++j)
    {
      result_series[j] = pow(series[j], 2.);
    }
  } // function square

  /* ----------------------------------------------------------------------- */
  void cube(datrw::Tdseries const& series, datrw::Tdseries& result_series)
  {
    if (series.size() != result_series.size())
    {
      throw std::string("Inconsistant series size.");
    }
    for (size_t j=series.f(); j<=series.l(); ++j)
    {
      result_series[j] = pow(series[j], 3.);
    }
  } // function cube
  
  /* ----------------------------------------------------------------------- */
  void multiply(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double fac)
  {
    if (series.size() != result_series.size())
    {
      throw std::string("Inconsistant series size.");
    }
    for (size_t j=series.f(); j<=series.l(); ++j)
    {
      result_series[j] = fac*series[j];
    }
  } // function multiply

  /* ----------------------------------------------------------------------- */

} // namespace util

/* ----- END OF util.cc  ----- */
