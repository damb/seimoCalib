/*! \file optnonlin.cc
 * \brief Optimization using global search algorithms of \a liboptimizexx.
 *
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/04/2012
 * 
 * Purpose: The aim of this program is to applie or rather minimize the formula
 *
 * \sum_\limits_l\left(a_0\ddot{y}+a_1\dot{y}+a_2y+a_3y^2+a_4y^3 = a\right)^2
 *
 * where $a_0$\ldots $a_4$ are coefficients to be searched, y is the Auslenkung
 * of the seismometer and a are the external forces wirken on the seismic mass.
 * With this approach investigating nonlinear behaviour of seimometers is
 * possible. Usually seismometers were described with the linear formula
 *
 * a_0\ddot{y}+a_1\dot{y}+a_2y = a.
 *
 * ----
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----
 * 
 * Copyright (c) 2012 by Daniel Armbruster
 * 
 * REVISIONS and CHANGES 
 * 19/04/2012  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */
 
#define OPTCALEX_VERSION "V0.1"
#define OPTCALEX_LICENSE "GPLv2"

#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <optimizexx/globalalgorithms/gridsearch.h>
#include <optimizexx/standardbuilder.h>
#include <optimizexx/iterator.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace opt = optimize;

using std::cout;
using std::cerr;
using std::endl;

typedef double TcoordType;
typedef NonLinResult TresultType;

int main(int iargc, char* argv[])
{

} // function main

/* ----- END OF optnonlin.cc  ----- */
