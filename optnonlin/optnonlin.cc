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
 
#define OPTNONLIN_VERSION "V0.1"
#define OPTNONLIN_LICENSE "GPLv2"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <optimizexx/globalalgorithms/gridsearch.h>
#include <optimizexx/standardbuilder.h>
#include <optimizexx/iterator.h>
#include <datrwxx/readany.h>
#include <types.h>
#include <visitor.h>
#include <validate.h>
#include <util.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace opt = optimize;

using std::cout;
using std::cerr;
using std::endl;

typedef double TcoordType;
typedef NonLinResult TresultType;

/* ---------------------------------------------------------------------------*/
//! overloaded operator< to sort param commandline args
bool operator<(opt::StandardParameter<double> const& p1,
    opt::StandardParameter<double> const& p2)
{
  if (p1.getId() < p2.getId()) { return true; }
  return false;
}

/* ---------------------------------------------------------------------------*/
bool operator==(opt::StandardParameter<double> const& p1
    opt::StandardParameter<double> const& p2)
{
  if (p1.getId() == p2.getId()) { return true; }
  return false;
}
/* ---------------------------------------------------------------------------*/

int main(int iargc, char* argv[])
{
  // define usage information
  char usage_text[]=
  {
    "Version: "OPTNONLIN_VERSION "\n"
    "License: "OPTNONLIN_LICENSE "\n" 
    "    SVN: $Id$\n" 
    " Author: Daniel Armbruster" "\n"
    "  Usage: optnonlin [-v|--verbose] [-o|--overwrite] [-t|--threads]" "\n"
    "                   [--config-file arg] [--linear] [--iformat arg]" "\n"
    "                   -p|--param arg -p|--param arg -p|--param arg" "\n"
    "                   [-p|--param arg -p|--param arg]" "\n"
    "                   --calib-in arg --calib-out arg OUTFILE" "\n"
    "     or: optnonlin -V|--version" "\n"
    "     or: optnonlin -h|--help" "\n"
    "     or: optnonlin --xhelp" "\n"
  }; // usage text

  // define notes text to provide additional information on commandline
  // arguments

  char notes_text[]=
  {
    "\n-----------------------\n"
    "Nonlinear system model:
    "Parameter search for a nonlinear system is based on the following" "\n"
    "model:" "\n"
    "   a_0*y''+a_1*y'+a_2*y+a_3*y^2+a_4*y^3 = u''" "\n"
    "where a_0, a_1, a_2, a_3 and a_4 are the unknown parameters, y is the" "\n"
    "output time series of the seismometer and u'' is the acceleration" "\n"
    "which is proportional to the calibration force affecting the seismic" "\n"
    "mass." "\n"
    "\n--------------------\n"
    "Linear system model:" "\n"
    "If using the '--linear' option optnonlin will perform a search based" "\n"
    "on the linear model equation for a seismometer:" "\n"
    "   a_0*y''+a_1*y'+a_2*y = u''" "\n"
    "where a_0, a_1 and a_2 are the unknown parameters, y is the output" "\n"
    "time series of the seismometer and u'' is the acceleration which is" "\n"
    "proportional to the calibration force affecting the seismic mass." "\n"
    "Note that if the option '--linear' is specified the commandline" "\n"
    "arguments for the unknown parameters 
    "'-p|--param a3 start end delta' and" "\n"
    "'-p|--param a4 start end delta' will be ignored if passed." "\n"
    "\n-------------------------------------------------------\n"
    "Additional notes on optnonlin unknown parameter syntax:\n"
    "To perform a parameter search with optnonlin search ranges for the" "\n"
    "unknown parameters must specified. To pass such a parameter on the" "\n"
    "commandline the following syntax has to be used:" "\n"
    "-p|--param id start end delta" "\n"
    "where 
    "   id      id of the unknown parameter" "\n"
    "           (either 'a0' or 'a1' or 'a2' or 'a3' or 'a4')" "\n"
    "   start   start of the search range" "\n"
    "   end     end of the search range" "\n"
    "   delta   stepwidth in search range" "\n\n"
    "Note if two parameters with the same id were specified the last one" "\n"
    "will be taken." "\n"
  };

  try
  {
    fs::path configFilePath;
    fs::path defaultConfigFilePath(std::string(getenv("HOME")));
    defaultConfigFilePath /= ".optimize";
    defaultConfigFilePath /= "optnonlin.rc";
    size_t numThreads = boost::thread::hardware_concurrency();
    std::string iformat("bin");
    std::vector<opt::StandardParameter<double>> params;

    // declare only commandline options
    po::options_description generic("Commandline options");
    generic.add_options()
      ("version,V", "Show version of optcalex.")
      ("help,h", "Print this help.")
      ("xhelp", "Print extended help text.")
      ("verbose,v", po::value<int>()->implicit_value(1), "Be verbose.")
      ("overwrite,o", "Overwrite OUTFILE")
      ("config-file", po::value<fs::path>(&configFilePath)->default_value(
        defaultConfigFilePath), "Path to optcalex configuration file.")
      ("linear,l", "Perform a search based on a linear model")
      ;

    // declare both commandline and configuration file options
    po::options_description config(
        "Both Commandline and optcalex configuration file options");
    config.add_options()
      ("param,p",
       po::value<std::vector<opt::StandardParameter<double>>>(
         &params)->required(),
       "Unknown parameter to search for.")
      ("threads,t", po::value<size_t>(&numThreads)->default_value(numThreads),
       "Number of threads to start for parallel computation")
      ("iformat", po::value<std::string>(&iformat)->default_value(iformat),
       "Format of input files (default: 'bin').")
      ("calib-in", po::value<fs::path>()->required(),
       "Filepath of calibration input signal file.")
      ("calib-out", po::value<fs::path>()->required(), 
       "Filepath of calibration output signal file.")
      ;

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("output-file", po::value<fs::path>()->required(),
       "Filepath of OUTFILE.")
      ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);

    po::options_description visible_options("Allowed options", 80);
    visible_options.add(generic).add(config);

    po::positional_options_description p;
    p.add("output-file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(iargc, argv).
      options(cmdline_options).positional(p).run(), vm);
    // help requested? print help
    if (vm.count("help"))
    {
      cout << usage_text
        << "------------------------------------------------------------\n";
      cout << visible_options;
      exit(0);
    } else
    if (vm.count("xhelp"))
    {
      cout << usage_text
        << "------------------------------------------------------------\n";
      cout << visible_options;
      cout << notes_text << std::endl;
      exit(0);
    } else
    if (vm.count("version"))
    {
      cout << "$Id$" << endl;
      cout << "Version: " << OPTNONLIN_VERSION << endl;
      exit(0);
    }
    po::notify(vm);

    if (vm.count("verbose"))
    {
      cout << "optnonlin: Opening optnonlin configuration file." << std::endl;
    }
    // reading configuration file
#if BOOST_FILESYSTEM_VERSION == 2
    std::ifstream ifs(configFilePath.string().c_str());
#else
    std::ifstream ifs(configFilePath.c_str());
#endif
    if (!ifs)
    {
      // if default configuration file does not exist -> create default
      // optnonlin configuration file
      if (configFilePath == defaultConfigFilePath &&
          !fs::exists(defaultConfigFilePath))
      {
        // create directory
        fs::create_directory(defaultConfigFilePath.parent_path());
        // create default configuration file
#if BOOST_FILESYSTEM_VERSION == 2
        std::ofstream ofs(defaultConfigFilePath.string().c_str());
#else
        std::ofstream ofs(defaultConfigFilePath.c_str());
#endif
        ofs.close();
      } else
      {
        throw std::string(
            "Can not open config file '"+configFilePath.string()+"'");
      }
    }
    else
    {
      po::store(parse_config_file(ifs, config_file_options), vm);
      po:: notify(vm);
    }

    // fetch commandline arguments
    fs::path outpath(vm["output-file"].as<fs::path>());
    if (fs::exists(outpath) && ! vm.count("overwrite"))
    {
      throw std::string("OUTFILE exists. Specify option 'overwrite'.");
    }
    fs::path calibInfile(vm["calib-in"].as<fs::path>());
    fs::path calibOutfile(vm["calib-out"].as<fs::path>());

    // check and sort unknown parameters
    std::sort(params.begin(), params.end());
    std::unique(params.begin(), params.end());
    if(vm.count("linear"))
    {
      if (params.size() != 3 || params.at(0).getId() != "a0" ||
          params.at(1).getId() != "a1" || params.at(2).getId() != "a2" )
      {
        throw std::string("Illegal parameter specification.");
      }
    } else
    {
      if (params.size() != 5 || params.at(0).getId() != "a0" ||
          params.at(1).getId() != "a1" || params.at(2).getId() != "a2" ||
          params.at(3).getId() != "a3" || params.at(4).getId() != "a4")
      {
        throw std::string("Illegal parameter specification.");
      }
    }

    // save addresses of unknown parameters
    std::vector<opt::StandardParameter<double> const*> param_adds;
    param_adds.reserve(5);
    for (auto cit params.cbegin(); cit != params.cend(); ++cit)
    {
      param_adds.push_back(&cit);
    }

    // read data files
    datrw::Tdseries calibInSeries;
    datrw::Tdseries calibOutSeries;
    sff::WID2 wid2CalibIn;
    sff::WID2 wid2CalibOut;
    {
#if BOOST_FILESYSTEM_VERSION == 2
      std::ifstream ifs(calibInfile.string().c_str(), 
          datrw::ianystream::openmode(iformat));
#else
      std::ifstream ifs(calibInfile.c_str(), 
          datrw::ianystream::openmode(iformat));
#endif
      if (!ifs.good()) { throw std::string("Cannot open input file!"); }
      datrw::ianystream is(ifs, iformat);    
      is >> calibInSeries;
      is >> wid2CalibIn;
    }
    {
#if BOOST_FILESYSTEM_VERSION == 2
      std::ifstream ifs(calibOutfile.string().c_str(), 
          datrw::ianystream::openmode(iformat));
#else
      std::ifstream ifs(calibOutfile.c_str(), 
          datrw::ianystream::openmode(iformat));
#endif
      if (!ifs.good()) { throw std::string("Cannot open input file!"); }
      datrw::ianystream is(ifs, iformat);    
      is >> calibOutSeries;
      is >> wid2CalibOut;
    }
    // check data header consistency
    if (vm.count("verbose")) 
    { 
      cout << "optnonlin: checking data consistency..." << endl;
    }
    sff::WID2compare compare(sff::Fnsamples | sff::Fdt | sff::Fdate);
    if (!compare (wid2CalibIn, wid2CalibOut))
    {
      throw std::string("Inconsistant time series header information.")
    }
    // prepare data for computation
    // TODO TODO TODO TODO TODO
    // use unique_ptr
    datrw::Tdseries* dif2Series = new datrw::Tdseries(calibOutSeries.size());
    datrw::Tdseries* difSeries = new datrw::Tdseries(calibOutSeries.size());
    datrw::Tdseries* squareSeries = 0;
    datrw::Tdseries* cubeSeries = 0;
    util::dif2(calibOutSeries, *dif2Series, wid2CalibIn.dt)
    util::dif(calibOutSeries, *difSeries, wid2CalibIn.dt)
    if(! vm.count("linear"))
    {
      squareSeries = new datrw::Tdseries(calibOutSeries.size());
      cubeSeries = new datrw::Tdseries(calibOutSeries.size());
      util::square(calibOutSeries, *squareSeries);
      util::cube(calibOutSeries, *cubeSeries);
    }

    // create global algorithm and set up parameter space
    if (vm.count("verbose"))
    {
      cout << "optnonlin: Setting up parameter space ..." << endl;
    }
    // create parameter space builder
    // TODO TODO TODO TODO
    // get parameter order of the builder
    opt::ParameterSpaceBuilder<TcoordType, TresultType>* builder =
      new opt::StandardParameterSpaceBuilder<TcoordType, TresultType>;
    // gridsearch algorithm
    opt::GlobalAlgorithm<TcoordType, TresultType>* algo = 
      new opt::GridSearch<TcoordType, TresultType>(builder, numThreads);

    opt::ParameterSpaceVisitor<TcoordType, TresultType>* app = 0;
    if (vm.count("linear"))
    {
      app = new LinApplication app(calibInSeries, *dif2Series, *difSeries,
          calibOutSeries, vm.count("verbose"));
    } else
    {
      app = new NonLinApplication app(calibInSeries, *dif2Series, *difSeries,
          calibOutSeries, *squareSeries, *cubeSeries, vm.count("verbose"));
    }

    algo->constructParameterSpace();
    if (vm.count("verbose"))
    {
      if (vm.count("linear"))
      {
        cout << "optnonlin: Sending linear application through parameter space "
          << "grid ..." << endl;
      } else
      {
        cout << "optnonlin: Sending nonlinear application through parameter "
          << "space grid ..." << endl;
      }
    }
    algo->execute(app);

    // collect results and write to outpath
    std::ofstream ofs(outpath.string().c_str());
    opt::Iterator<TcoordType, TresultType> it = 
      algo->getParameterSpace().createIterator(opt::ForwardNodeIter);

    if (vm.count("verbose"))
    {
      cout << "optnonlin: Collecting results from parameter space grid ..."
        << endl
        << "optnonlin: Writing result file ..."
        << endl;
    }

    for (it.first(); !it.isDone(); ++it)
    {
      std::vector<TcoordType> const& c = (*it)->getCoordinates();
      for (std::vector<TcoordType>::const_iterator cit(c.begin());
          cit != c.end(); ++cit)
      {
        ofs << std::setw(12) << std::fixed << std::left << *cit << " ";
      }
      ofs << "    ";
      // TODO TODO TODO TODO TODO TODO TODO
      // implement correctly
      (*it)->getResultData().writeLine(ofs);
    }

    // TODO TODO TODO TODO TODO TODO
    // clean up
    delete algo;
    delete app;
    delete builder;

  }
  catch (std::string e) 
  {
    cerr << "ERROR: " << e << "\n";
    cerr << usage_text;
    return 1;
  }
  catch(std::exception& e)
  {
    cerr << "ERROR: Exception of unknown type!\n";
    cerr << usage_text;
    return 1;
  }    

  return 0;

} // function main

/* ----- END OF optnonlin.cc  ----- */
