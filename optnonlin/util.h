/*! \file util.h
 * \brief Declaration of utility functions for optnonlin.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: Declaration of utility functions for optnonlin.
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

#include <datrwxx/types.h>
 
#ifndef _OPTNONLIN_UTIL_H_
#define _OPTNONLIN_UTIL_H_

namespace util
{
  /*! 
   * compute time derivative of a time series using a symmetric difference
   * quotient
   * \f[
   *    f'(a) = \frac{f(a+h)-f(a-h)}{2\cdot h}.
   * \f]
   *
   * \param series input data to calculate derivative from
   * \param result_series the result will be saved to this series
   * \param dt sampling interval
   * \param time_constant time constant to multiply data with
   */
  void dif(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double dt, double time_constant=0.);

  /*! 
   * compute second time derivative of a time series using the following
   * formula:
   * \f[
   *    f''(a) = \frac{f(a+h)-2\cdot f(a)+f(a-h)}{h^2}.
   * \f]
   *
   * \param series input data to calculate derivative from
   * \param result_series the result will be saved to this series
   * \param dt sampling interval
   * \param time_constant time constant to multiply data with
   */
  void dif2(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double dt, double time_constant=0.);

  /*!
   * compute square of time series
   *
   * \param series input data to calculate derivative from
   * \param result_series the result will be saved to this series
   */
  void square(datrw::Tdseries const& series, datrw::Tdseries& result_series);

  /*!
   * compute cube of time series
   *
   * \param series input data to calculate derivative from
   * \param result_series the result will be saved to this series
   */
  void cube(datrw::Tdseries const& series, datrw::Tdseries& result_series);

  /*!
   * multiply time series with factor
   *
   * \param series input data
   * \param result_series the result will be saved to this series
   * \param fac factor to multiply timeseries with
   */
  void multiply(datrw::Tdseries const& series, datrw::Tdseries& result_series,
      double fac);

} // namespace util

#endif // include guard

/* ----- END OF util.h  ----- */
