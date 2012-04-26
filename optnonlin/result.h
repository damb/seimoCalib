/*! \file result.h
 * \brief
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 22/04/2012
 * 
 * Purpose: Declaration of a class holding the results for optnonlin. 
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
 * 22/04/2012  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */

#include <iostream>
 
#ifndef _OPTNONLIN_RESULT_H_
#define _OPTNONLIN_RESULT_H_

class OptResult
{
  public:
    //! constructor
    OptResult() : MmdMisfit(0), MrmsMisfit(0) { }
    //! constructor
    OptResult(double md_misfit, double rms_misfit) : MmdMisfit(md_misfit),
      MrmsMisfit(rms_misfit)
    { }
    //! query functions for data
    double const& getMdMisfit() const { return MmdMisfit; }
    double const& getRmsMisfit() const { return MrmsMisfit; }

    //! write header line to output stream
    void writeHeaderLine(std::ostream& os) const;
    //! output stream operator
    friend std::ostream& operator<<(
        std::ostream& os, OptResult const& result);

  protected:
    //! write data to outputstream
    void write(std::ostream& os) const;
  
  private:
    //! misfit simply using the difference of two seismograms.
    double MmdMisfit;
    //! RMS (root mean square) misfit
    double MrmsMisfit;

}; // class OptResults

#endif // include guard

/* ----- END OF result.h  ----- */
