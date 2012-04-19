/*! \file result.h
 * \brief Declaration of a class holding the results for \c optnonlin.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: Declaration of a class holding the results for \c optnonlin.  
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
 
#ifndef _OPTNONLIN_RESULT_H_
#define _OPTNONLIN_RESULT_H_

/*!
 * Holds the results of the global search.
 *
 * To understand the meanings of the variables just consider the formula which 
 * takes the nonlinearities of a seismometer into account:
 *
 * \sum_\limits_l\left(a_0\ddot{y}+a_1\dot{y}+a_2y+a_3y^2+a_4y^3 = a\right)^2.
 */
class NonLinResult
{
  public:
    //! constructor
    NonLinResult() : MisComputed(false) { }
    //! constructor
    NonLinResult(double const a_0, double const a_1, double const a_2,
        double const a_3=0, double const a_4=0) : MisComputed(true), Ma0(a_0),
        Ma1(a_1), Ma2(a_2), Ma3(a_3), Ma4(a4)
    { }
    //! query function if entire data had been set
    bool isComputed() const { return MisComputed; }
    //! query functions for parameters
    double const& getA0() { return Ma0; }
    double const& getA1() { return Ma1; }
    double const& getA2() { return Ma2; }
    double const& getA3() { return Ma3; }
    double const& getA4() { return Ma4; }

    //! output stream operator
    friend std::ostream& operator<<(
        std::ostream& os, NonLinResult const& result);

  private:
    //! status variable if the result parameters had been set
    bool MisComputed;
    //! coefficient of the second order differentiation (a_0)
    double Ma0;
    //! coefficient of the first order differentiation (a_1)
    double Ma1;
    //! coefficient (a_2)
    double Ma2;
    //! coefficient of the square (a_3)
    double Ma3;
    //! coefficient of the cubic (a_4)
    double Ma4;

}; // class NonLinResult

#endif // include guard

/* ----- END OF result.h  ----- */
