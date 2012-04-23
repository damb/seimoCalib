/*! \file result.cc
 * \brief 
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 22/04/2012
 * 
 * Purpose: 
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

#include <sstream>
#include <iomanip>
#include "result.h"

/*---------------------------------------------------------------------------*/
void OptResult::writeHeaderLine(std::ostream& os) const
{
    os << std::setw(12) << std::right << "MD misfit"
      << std::setw(12) << std::right << "RMS misfit" << std::endl;
}

/*---------------------------------------------------------------------------*/
std::ostream& operator<<(std::ostream& os, OptResult const& result)
{
  result.write(os);
  return os;
}

/*---------------------------------------------------------------------------*/
void OptResult::write(std::ostream& os) const
{
  std::stringstream ss;
  ss << std::right << std::setw(12) << MmdMisfit
    << std::setw(12) << std::right << std::fixed << MrmsMisfit;

  os << ss.str() << std::endl;
}


/* ----- END OF result.cc  ----- */
