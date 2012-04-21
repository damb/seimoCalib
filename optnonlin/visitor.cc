/*! \file visitor.cc
 * \brief Implementation of the \a libopimizexx parameter space visitor
 * computing the RMS error.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: Implementation of the \a libopimizexx parameter space visitor
 * computing the RMS error.  
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
 
#include <iostream>
#include <iomanip>
#include <cmath>
#include <visitor.h>
#include <util.h>

void NonLinApplication::operator()(opt::Node<TcoordType, TresultType>* node)
{
  std::vector<TcoordType> const& coordinates = node->getCoordinates();

  
  datrw::Tdseries a0TimesMyDif2(McalibInSeries.size());
  datrw::Tdseries a1TimesMyDif(McalibInSeries.size());
  datrw::Tdseries a2TimesMy(McalibInSeries.size());
  datrw::Tdseries a3TimesMySquare(McalibInSeries.size());
  datrw::Tdseries a4TimesMyCube(McalibInSeries.size());

  // multiply series with coordinate factor
  util::multiply(MyDif2, a0TimesMyDif2, coordinates[0]);
  util::multiply(MyDif, a1TimesMyDif, coordinates[1]);
  util::multiply(My, a2TimesMy, coordinates[2]);
  util::multiply(MySquare, a3TimesMySquare, coordinates[3]);
  util::multiply(MyCube, a4TimesMyCube, coordinates[4]);

  datrw::Tdseries sum(McalibInSeries.size());
  // compute sum of series
  for (size_t j=a0TimesMyDif2.f(); j<=a0TimesMyDif2.l(); ++j)
  {
    sum[j] = a0TimesMyDif2[j]+a1TimesMyDif[j]+a2TimesMy[j]+a3TimesMySquare[j]
      +a4TimesMyCube[j]-McalibInSeries[j];
  }
  // compute square of series
  for (size_t j=sum.f(); j<=sum.l(); ++j) { sum[j] = sum[j]*sum[j]; }

  double result = 0;
  for (size_t j=sum.f(); j<=sum.l(); ++j) { result += sum[j]; }
  
  result/=sum.size();
  result = sqrt(result);
  node->setResultData(result);
  node->setComputed();

  if (Mverbose) 
  { 
    // without loop cause if using multiple threads to avoid mixing output up
    std::cout << "Parameter configuration: "
      << std::setw(12) << std::fixed << std::right << coordinates[0] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[1] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[2] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[3] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[4]
      << "\nResult: " << result << std::endl;
  }
} // function NonLinApplication::operator()


/* ----- END OF visitor.cc  ----- */
