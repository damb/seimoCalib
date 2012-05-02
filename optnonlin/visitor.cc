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
#include <vector>
#include "visitor.h"
#include "util.h"
#include "result.h"
#include "types.h"

/* -------------------------------------------------------------------------- */
void LinApplication::operator()(opt::Node<TcoordType, TresultType>* node)
{
  std::vector<TcoordType> const& coordinates = node->getCoordinates();
  
  datrw::Tdseries factorTimesMyDif(McalibInSeries.size());
  datrw::Tdseries factorTimesMy(McalibInSeries.size());

  // h  -> coordinates[0]
  // T0 -> coordinates[1]
  // multiply series with appropriate factor
  util::multiply(MyDif, factorTimesMyDif,
      ((2*Mpi)/coordinates[1])*coordinates[0]);
  util::multiply(My, factorTimesMy, (4.*pow(Mpi, 2.))/coordinates[1]);

  datrw::Tdseries differenceSeries(McalibInSeries.size()); 
  // compute difference of series
  for (int j=MyDif2.f(); j<=MyDif2.l(); ++j)
  {
    differenceSeries(j) = fabs(MyDif2(j) + factorTimesMyDif(j) +
          factorTimesMy(j) - McalibInSeries(j));
  }
  // compute square of difference of series
  double md_numerator = 0;
  double md_denominator = 0;
  double rms_numerator = 0;
  double rms_denominator = 0;
  for (int j=differenceSeries.f(); j<=differenceSeries.l(); ++j)
  {
    md_numerator += differenceSeries(j);
    rms_numerator += pow(differenceSeries(j), 2.);
    md_denominator += fabs(McalibInSeries(j));
    rms_denominator += pow(McalibInSeries(j), 2.);
  }

  TresultType result(md_numerator / md_denominator, 
      sqrt(rms_numerator / rms_denominator));
  node->setResultData(result);
  node->setComputed();

  if (Mverbose) 
  { 
    // without loop cause if using multiple threads to avoid mixing output up
    std::cout << "Parameter configuration: "
      << std::setw(12) << std::fixed << std::right << coordinates[0] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[1]
      << "\nResult: " << result << std::endl;
  }
} // function LinApplication::operator()

/* -------------------------------------------------------------------------- */
void NonLinApplication::operator()(opt::Node<TcoordType, TresultType>* node)
{
  std::vector<TcoordType> const& coordinates = node->getCoordinates();

  
  datrw::Tdseries factorTimesMyDif(McalibInSeries.size());
  datrw::Tdseries factorTimesMy(McalibInSeries.size());
  datrw::Tdseries c0TimesMySquare(McalibInSeries.size());
  datrw::Tdseries c1TimesMyCube(McalibInSeries.size());

  // multiply series with approriate coordinate factor
  // c0 -> coordinates[0]
  // c1 -> coordinates[1]
  // h  -> coordinates[2]
  // T0 -> coordinates[3]
  util::multiply(MyDif, factorTimesMyDif,
      ((2*Mpi)/coordinates[3])*coordinates[2]);
  util::multiply(My, factorTimesMy, (4.*pow(Mpi, 2.))/coordinates[3]);
  util::multiply(MySquare, c0TimesMySquare, coordinates[0]);
  util::multiply(MyCube, c1TimesMyCube, coordinates[1]);

  datrw::Tdseries differenceSeries(McalibInSeries.size()); 
  // compute difference of series
  for (int j=MyDif2.f(); j<=MyDif2.l(); ++j)
  {
    differenceSeries(j) = fabs(MyDif2(j) + factorTimesMyDif(j) +
          factorTimesMy(j) + c0TimesMySquare(j) + c1TimesMyCube(j) -
          McalibInSeries(j));
  }
  // compute square of difference of series
  double md_numerator = 0;
  double md_denominator = 0;
  double rms_numerator = 0;
  double rms_denominator = 0;
  for (int j=differenceSeries.f(); j<=differenceSeries.l(); ++j)
  {
    md_numerator += differenceSeries(j);
    rms_numerator += pow(differenceSeries(j), 2.);
    md_denominator += fabs(McalibInSeries(j));
    rms_denominator += pow(McalibInSeries(j), 2.);
  }

  TresultType result(md_numerator / md_denominator, 
      sqrt(rms_numerator / rms_denominator));
  node->setResultData(result);
  node->setComputed();

  if (Mverbose) 
  { 
    // without loop cause if using multiple threads to avoid mixing output up
    std::cout << "Parameter configuration: "
      << std::setw(12) << std::fixed << std::right << coordinates[0] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[1] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[2] << " "
      << std::setw(12) << std::fixed << std::right << coordinates[3]
      << "\nResult: " << result << std::endl;
  }
} // function NonLinApplication::operator()


/* ----- END OF visitor.cc  ----- */
