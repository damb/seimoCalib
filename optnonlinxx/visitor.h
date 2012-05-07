/*! \file visitor.h
 * \brief Declaration of a parameter space visitor class for \c optnonlin.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: Declaration of a parameter space visitor class for \c optnonlin.  
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
 * 19/04/2012   V0.1    Daniel Armbruster
 * 02/05/2012   V0.1.1  Corrections and adjustments of seismometer models.
 * 
 * ============================================================================
 */
 
#include <cmath>
#include <optimizexx/application.h>
#include <optimizexx/node.h>
#include <datrwxx/types.h>
#include "types.h"

#ifndef _OPTNONLIN_VISITOR_H_
#define _OPTNONLIN_VISITOR_H_

namespace opt = optimize;
/*!
 * \a liboptimizexx parameter space visitor which acutally is the forward
 * algorithm of the linear model optimization for a seismometer.
 */
class LinApplication :
  public opt::ParameterSpaceVisitor<TcoordType, TresultType>
{
  public:
    //! constructor
    LinApplication(datrw::Tdseries const& calib_in_series, 
        datrw::Tdseries const& y_dif2, datrw::Tdseries const& y_dif,
        datrw::Tdseries const& y, bool verbose=false) :
      McalibInSeries(calib_in_series), MyDif2(y_dif2), MyDif(y_dif), My(y),
      Mpi(4.*atan(1.)), Mverbose(verbose)
    { 
      if (McalibInSeries.size() != MyDif2.size() || 
          McalibInSeries.size() != MyDif.size() ||
          McalibInSeries.size() != My.size())
      {
        throw std::string("Inconsistent length of time series.");
      }
    }
    //! Visit function for a liboptimizexx grid.
    /*!
     * Does nothing by default.
     * Since a grid has no coordinates the body of this function is empty.
     *
     * \param grid Grid to be visited.
     */
    virtual void operator()(opt::Grid<TcoordType, TresultType>* grid) { }
    //! Visit function / application for a liboptimizexx node.
    /*!
     * Computes the \f$RMS\f$ error as follows:
     * \f[
     *    RMS = \sqrt{\frac{\sum_l=1^N\left(
     *      a_0\ddot{y}_l+a_1\dot{y}_l+a_2y_l-\ddot{u}_l
     *      \right)^2}{N}}
     * \f]
     * where \f$y\f$ is the displacement of of the seismometer and
     * \f$\ddot{u}\f$ the acceleration which is proportional to the force
     * affecting the seismic mass. \f$N\f$ are the number of samples in the time
     * series.
     *
     * \param node Node to be visited.
     */
    virtual void operator()(opt::Node<TcoordType, TresultType>* node);
  private:
    //! time series containing the calibration signal
    datrw::Tdseries const& McalibInSeries;
    //! second derivative of the output time series of the seismometer
    datrw::Tdseries const& MyDif2;
    //! derivative of the output time series of the seismometer
    datrw::Tdseries const& MyDif;
    //! output time series of the seismometer
    datrw::Tdseries const& My;
    // pi constant
    double const Mpi;
    //! verbosity flag
    bool Mverbose;
}; // class LinApplication

/* -------------------------------------------------------------------------- */
/*!
 * \a liboptimizexx parameter space visitor which acutally is the forward
 * algorithm of the nonlinear model optimization.
 */
class NonLinApplication :
  public opt::ParameterSpaceVisitor<TcoordType, TresultType>
{
  public:
    //! constructor
    NonLinApplication(datrw::Tdseries const& calib_in_series, 
        datrw::Tdseries const& y_dif2, datrw::Tdseries const& y_dif,
        datrw::Tdseries const& y, datrw::Tdseries const& y_square,
        datrw::Tdseries const& y_cube, bool verbose=false) :
      McalibInSeries(calib_in_series), MyDif2(y_dif2), MyDif(y_dif), My(y),
      MySquare(y_square), MyCube(y_cube), Mpi(4.*atan(1.)), Mverbose(verbose)
    { 
      if (McalibInSeries.size() != MyDif2.size() || 
          McalibInSeries.size() != MyDif.size() ||
          McalibInSeries.size() != My.size() ||
          McalibInSeries.size() != MySquare.size() ||
          McalibInSeries.size() != MyCube.size())
      {
        throw std::string("Inconsistent length of time series.");
      }
    }
    //! Visit function for a liboptimizexx grid.
    /*!
     * Does nothing by default.
     * Since a grid has no coordinates the body of this function is empty.
     *
     * \param grid Grid to be visited.
     */
    virtual void operator()(opt::Grid<TcoordType, TresultType>* grid) { }
    //! Visit function / application for a liboptimizexx node.
    /*!
     * Computes the \f$RMS\f$ error as follows:
     * \f[
     *    RMS = \sqrt{\frac{\sum_l=1^N\left(
     *      a_0\ddot{y}_l+a_1\dot{y}_l+a_2y_l+a_3y_l^2+a_4y_l^3-\ddot{u}_l
     *      \right)^2}{N}}
     * \f]
     * where \f$y\f$ is the displacement of of the seismometer and
     * \f$\ddot{u}\f$ the acceleration which is proportional to the force
     * affecting the seismic mass. \f$N\f$ are the number of samples in the time
     * series.
     *
     * \param node Node to be visited.
     */
    virtual void operator()(opt::Node<TcoordType, TresultType>* node);
  private:
    //! time series containing the calibration signal
    datrw::Tdseries const& McalibInSeries;
    //! second derivative of the output time series of the seismometer
    datrw::Tdseries const& MyDif2;
    //! derivative of the output time series of the seismometer
    datrw::Tdseries const& MyDif;
    //! output time series of the seismometer
    datrw::Tdseries const& My;
    //! square of the output time series of the seismometer
    datrw::Tdseries const& MySquare;
    //! cube of the output time series of the seismometer
    datrw::Tdseries const& MyCube;
    // pi constant
    double const Mpi;
    //! verbosity flag
    bool Mverbose;

}; // class NonLinApplication

#endif // include guard

/* ----- END OF visitor.h  ----- */
