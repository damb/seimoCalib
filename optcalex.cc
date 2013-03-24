/*! \file optcalex.cc
 * \brief Optimization program using both liboptimizexx and libcalexxx.
 * 
 * ----------------------------------------------------------------------------
 * 
 * $Id$
 * \author Daniel Armbruster
 * \date 19/03/2012
 * 
 * Purpose: Optimization program using both liboptimizexx and libcalexxx.  
 * Notice that this tool makes use of the gridsearch algorithm.
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
 * 19/03/2012  V0.1   Daniel Armbruster
 * 08/04/2012  V0.2   Both commandline options and option passing in a
 *                    configuration file is supported.
 * 18/04/2012  V0.3   Dynamic configuration of number of threads started for
 *                    computation.
 * 25/04/2012  V0.4   Adjust program to handle interface changes of
 *                    liboptimizexx and libcalexxx.
 * 10/05/2012  V0.4.1 Bug fixed. System parameters were set correctly now.
 * 15/05/2012  V0.5   Write header information to result data files.
 * 05/07/2012  V0.5.1 Check if there are search parameters for calex
 *                    application. If not set maxit to 0.
 * 21/03/2013  V0.5.2 add comments to help text
 * 24/03/2013  V0.6   make use of boost::program_options custom validators
 * 
 * ============================================================================
 */
 
#define _OPTCALEX_VERSION_ "V0.6"
#define _OPTCALEX_LICENSE_ "GPLv2+"

#include <vector>
#include <iomanip>
#include <fstream>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <optimizexx/parameter.h>
#include <optimizexx/standardbuilder.h>
#include <optimizexx/iterator.h>
#include <optimizexx/globalalgorithms/gridsearch.h>
#include <calexxx/calexvisitor.h>
#include <calexxx/defaults.h>
#include "optcalexxx/validator.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace opt = optimize;

using std::cout;
using std::cerr;
using std::endl;

typedef double TcoordType;
typedef calex::CalexResult TresultType;

int main(int iargc, char* argv[])
{

  // define usage information
  char usage_text[]=
  {
    "Version: " _OPTCALEX_VERSION_ "\n"
    "License: " _OPTCALEX_LICENSE_ "\n" 
    "    SVN: $Id$\n" 
    " Author: Daniel Armbruster" "\n"
    "  Usage: optcalex [-v|--verbose] [-o|--overwrite] [-t|--threads] " "\n"
    "                  [--config-file arg] [--maxit arg]" "\n"
    "                  [--alias arg] [--qac arg] [--finac arg]" "\n"
    "                  [--ns1 arg] [ns2 arg] [--m0 arg] [-p|--param arg]" "\n"
    "                  [--first-order arg] [--second-order arg]" "\n"
    "                  --calib-in arg --calib-out arg OUTFILE" "\n"
    "     or: optcalex -V|--version" "\n"
    "     or: optcalex -h|--help" "\n"
    "     or: optcalex --xhelp" "\n"
  };

  // define notes text to provide additional information on commandline
  // arguments

  char notes_text[]=
  {
    "\n---------------------------------------------------------\n"
    "Additional notes on calex system parameter configuration:\n"
    "---------------------------------------------------------\n"
    "To add additional system parameters to the calex parameter file use the\n"
    "following syntax:\n"
    " -p|--param=nam|val|unc\n"
    "where\n"
    "'nam' is a unique id of the system parameter (three letters)\n"
    "'val' is the initial value of the system parameter\n"
    "'unc' is the uncertainty.\n"
    "If 'unc' is set to zero it will not be optimized by Erhard Wielandt's\n"
    "calex program. System parameters which are optimized are called 'active\n"
    "parameters'. The IDs 'amp', 'del', 'sub' or 'til' belong to obligatory\n"
    "system parameters (see below).\n"
    "Besides of usual system parameter (both active and inactive) there are\n"
    "as well system parameters treated as grid system parameters. Grid\n"
    "system parameters set up the optcalex parameter space.\n"
    "Grid system parameters must be specified with the following syntax:\n"
    " -p|--param=nam|start;end;delta|unc\n"
    "where\n"
    "'start' defines the start of the parameter space axis.\n"
    "'end'   defines the end of the parameter space axis.\n"
    "'delta' defines the interval the range between 'start' and 'end'\n"
    "is sampled.\n\n"
    "Additionally to common system parameters calex has four obligatory\n"
    "system parameters:\n"
    "'amp'\n"
    "'del'\n"
    "'sub'\n"
    "'til'\n"
    "Their meanings are explained in Erhard Wielandt's calex program\n"
    "documentation. Notice that if specifying both 'del' and 'sub' the\n"
    "system parameter passed as the latter one will disable the previous\n"
    "one.\n"
    "In case of defining system parameters with identical 'nam' ids, optcalex\n"
    "is enforced to only take the latter one passed on the commandline into\n"
    "account.\n\n"
    "To describe a system there are first and second order subsystems.\n"
    "----\n"
    "First order subsystems:\n"
    "To specify a first order subsystem the following syntax should be used.\n"
    " --first-order=[LP,HP]|nam|val|unc\n"
    "Of course there is the possibility to specify the subsystem with a grid\n"
    "system parameter, too. Just use the syntax below.\n"
    " --first-order=[LP,HP]|nam|start;end;delta|unc\n"
    "----\n"
    "Second order subsystems:\n"
    "The syntax to define a second order subsystem does not differ from the\n"
    "syntax of first order subsystems. The only difference is the fact that\n"
    "for second order subsystems only one system parameter might be a grid\n"
    "system parameter.\n"
    "So in case both parameters describing the subsystems are system\n"
    "parameters use\n"
    " --second-order=[LP,HP,BP]|nam1|val1|unc1|val2|unc2\n"
    "If the first parameter is a grid system parameter use\n"
    " --second-order=[LP,HP,BP]|nam1|start1;end1;delta1|unc1|nam2|val2|unc2\n"
    "and if the latter system parameter is a grid system parameter use\n"
    " --second-order=[LP,HP,BP]|nam1|val1|unc1|nam2|start2;end2;delta2|unc2\n"
    "In the special case that both parameters are grid system parameters the\n"
    "syntax is as follows:\n"
    " --second-order=\n"
    "   [LP,HP,BP]|nam1|start1;end1;delta1|unc1|nam2|start2;end2;delta2|unc2\n"
    "-------------------------------------------------------------\n"
    "| NOTICE: Separators must be passed exactly as shown above. |\n"
    "-------------------------------------------------------------\n"

    "\n=====================================================================\n"
    "optcalex will write the results computed to OUTFILE. The first line of\n"
    "the file is a header line to simplify the identification of the columns\n"
    "holding the data." "\n"
    "The left hand side columns are the columns holding the parameter space\n"
    "coordinates (which actually are the search parameters) followed by" "\n"
    "the result data columns." "\n"
    "An examplary result data file written by optcalex looks as follows:" "\n"
    "\n"
    "per          dmp               iter         RMS         amp         del\n"
    "19.600000    0.680000             7    0.013982   -1.480059   -0.057004\n"
    "20.000000    0.680000             7    0.013982   -1.480059   -0.057004\n"
    "[...]" "\n\n"
    "Description:" "\n"
    "In the example above the parameter searched for had been the period and\n"
    "the damping of a second order bandpass system (described by the\n"
    "parameters period (per) and damping (dmp)). The results were computed\n"
    "by optcalex's forward algorithm which is nothing else than Erhard" "\n"
    "Wielandt's calex program (http://www.software-for-seismometry.de/)." "\n"
    "calex then adjusted the amplitude (amp) and the delay (del) and" "\n"
    "computed a normalized root mean square (RMS) after the number of" "\n"
    "iterations specified by the 'iter' column." "\n"

  };

  try
  {
    
    fs::path configFilePath;
    fs::path defaultConfigFilePath(std::string(getenv("HOME")));
    defaultConfigFilePath /= ".optimize";
    defaultConfigFilePath /= "optcalex.rc";
    size_t numThreads = boost::thread::hardware_concurrency();

    // declare only commandline options
    po::options_description generic("Commandline options");
    generic.add_options()
      ("version,V", "Show version of optcalex.")
      ("help,h", "Print this help.")
      ("xhelp", "Print extended help text.")
      ("verbose,v",po::value<int>()->implicit_value(1), "Be verbose.")
      ("overwrite,o", "Overwrite OUTFILE")
      ("config-file", po::value<fs::path>(&configFilePath)->default_value(
        defaultConfigFilePath), "Path to optcalex configuration file.")
      ("threads,t",po::value<size_t>(&numThreads)->default_value(numThreads),
       "Number of threads to start for parallel computation")
      ;

    // declare both commandline and configuration file options
    po::options_description config(
        "Both Commandline and optcalex configuration file options");
    config.add_options()
      ("alias", po::value<double>()->default_value(CALEX_ALIAS),
       "Period of anti-alias filter")
      ("qac", po::value<double>()->default_value(CALEX_QAC),
       "Iteration stops when improvement in the rms misfit in one step becomes"
       "less than qac.")
      ("finac", po::value<double>()->default_value(CALEX_FINAC),
       "Iteration stops when normalized parameters change by less than finac.")
      ("ns1", po::value<int>()->default_value(CALEX_NS1),
       "Number of samples skipped at beginning of timeseries.")
      ("ns2", po::value<int>()->default_value(CALEX_NS2),
       "Number of samples skipped at end of timeseries.")
      ("m0", po::value<int>()->default_value(CALEX_M0),
       "Number of additional powers of the Laplace variable s.")
      ("maxit", po::value<int>()->default_value(CALEX_MAXIT),
       "Number of maximum iterations of calex for each parameter "
       "configuration.")
      ("param,p", po::value<std::vector<
       std::shared_ptr<calex::SystemParameter>>>(),
       "calex system parameter.")
      ("first-order", po::value<std::vector<calex::FirstOrderSubsystem> >(),
       "Add first order subsystem to calex parameter file.")
      ("second-order", po::value<std::vector<calex::SecondOrderSubsystem> >(),
       "Add second order subsystem to calex parameter file.")
      ("calib-in", po::value<fs::path>()->required(),
       "Filename of calibration input signal file (format: seife).")
      ("calib-out", po::value<fs::path>()->required(), 
       "Filename of calibration output signal file (format seife).")
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
      cout << "Version: " << _OPTCALEX_VERSION_ << endl;
      exit(0);
    }
    po::notify(vm);

    if (vm.count("verbose"))
    {
      cout << "optcalex: Opening optcalex configuration file." << std::endl;
    }

    // reading configuration file
#if BOOST_FILESYSTEM_VERSION == 2
    std::ifstream ifs(configFilePath.string().c_str());
#else
    std::ifstream ifs(configFilePath.c_str());
#endif
    if (!ifs)
    {
      // if default configuration file does not exist -> create default optcalex
      // configuration file
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
    // check if path is in working directory
    if (! fs::exists(fs::current_path() /= calibInfile) ||
      ! fs::exists(fs::current_path() /= calibOutfile)) 
    {
      throw std::string(
          "Only execute optcalex in directory where datafiles are located.");
    }
    if (vm.count("verbose"))
    {
      cout << "optcalex: Collecting commandline arguments ..." << endl 
        << "optcalex: Calibration input time series:  " 
        << calibInfile.string() << endl 
        << "optcalex: Calibration output time series: " 
        << calibOutfile.string() << endl
        << "optcalex: Result data filepath:           " 
        << outpath.string() << endl;
    }

    /* --------------------------------------------------------------------- */
    // configure calex parameter file
    calex::CalexConfig calex_config(
        calibInfile.string(), calibOutfile.string());
    // fetch system parameter commandline arguments
    if (vm.count("param"))
    {
      std::vector<std::shared_ptr<calex::SystemParameter>> sys_params =
        vm["param"].as<std::vector<std::shared_ptr<calex::SystemParameter>>>();
      for (auto cit(sys_params.cbegin()); cit != sys_params.cend(); ++cit)
      {
        if ((*cit)->get_nam() == "amp")
        {
          calex_config.set_amp(*cit);
        } else
        if ((*cit)->get_nam() == "del")
        {
          calex_config.set_del(*cit);
        } else
        if ((*cit)->get_nam() == "sub")
        {
          calex_config.set_sub(*cit);
        } else
        if ((*cit)->get_nam() == "til")
        {
          calex_config.set_til(*cit);
        } else
        {
          calex_config.add_systemParameter(*cit);
        }
      }
    }

    // fetch and handle first order subsystem commandline arguments
    if (vm.count("first-order"))
    {
      std::vector<calex::FirstOrderSubsystem> first_order =
        vm["first-order"].as<std::vector<calex::FirstOrderSubsystem>>();
      for (auto cit(first_order.cbegin()); cit != first_order.cend(); ++cit)
      {
        calex_config.add_subsystem(
            std::make_shared<calex::FirstOrderSubsystem>(*cit));
      }
    }
    // fetch and handle second order subsystem commandline arguments
    if (vm.count("second-order"))
    {
      std::vector<calex::SecondOrderSubsystem> second_order =
        vm["second-order"].as<std::vector<calex::SecondOrderSubsystem> >();
      for (auto cit(second_order.cbegin()); cit != second_order.cend(); ++cit)
      {
        calex_config.add_subsystem(
            std::make_shared<calex::SecondOrderSubsystem>(*cit));
      }
    }
    
    if (vm.count("verbose"))
    {
      cout << "optcalex: Set alias to: " << vm["alias"].as<double>() << endl
        << "optcalex: Set m0 to:    " << vm["m0"].as<int>() << endl 
        << "optcalex: Set ns1 to:   " << vm["ns1"].as<int>() << endl
        << "optcalex: Set ns2 to:   " << vm["ns2"].as<int>() << endl 
        << "optcalex: Set qac to:   " << vm["qac"].as<double>() << endl
        << "optcalex: Set finac to: " << vm["finac"].as<double>() << endl 
        << "optcalex: Set maxit to: " << vm["maxit"].as<int>() << endl;
    }
    calex_config.set_alias(vm["alias"].as<double>());
    calex_config.set_m0(vm["m0"].as<int>());
    calex_config.set_ns1(vm["ns1"].as<int>());
    calex_config.set_ns2(vm["ns2"].as<int>());
    calex_config.set_qac(vm["qac"].as<double>());
    calex_config.set_finac(vm["finac"].as<double>());
    calex_config.set_maxit(vm["maxit"].as<int>());

    // checks
    if (! calex_config.hasGridSystemParameters())
    {
      throw std::string("No grid system parameters specified.");
    }

    if (0 == calex_config.get_numActiveParameters() &&
       0 != calex_config.get_maxit())
    {
      if (vm.count("verbose"))
      {
        cout << "optcalex: No active parameters for inversion defined." << endl
          << "optcalex: Set 'maxit' parameter to 0." << endl;
      }
      calex_config.set_maxit(0);
    }

    /* --------------------------------------------------------------------- */
    // mayor part
    
    if (vm.count("verbose"))
    {
      cout << "optcalex: Setting up parameter space ..." << endl;
    }
    // create parameter space builder
    std::unique_ptr<opt::ParameterSpaceBuilder<TcoordType, TresultType>>
      builder(new opt::StandardParameterSpaceBuilder<TcoordType, TresultType>);

    // gridsearch algorithm
    std::shared_ptr<opt::GlobalAlgorithm<TcoordType, TresultType>> algo( 
      new opt::GridSearch<TcoordType, TresultType>(
          std::move(builder), numThreads));

    calex_config.set_gridSystemParameters<TcoordType>(*algo);

    calex::CalexApplication<TcoordType> app(&calex_config,
       vm.count("verbose"));

    // synchronization grid and calex configuration file
    calex_config.synchronize<TcoordType>(*algo);

    algo->constructParameterSpace();
    if (vm.count("verbose"))
    {
      cout << "optcalex: Sending calex application through parameter space "
        << "grid ..." << endl;
    }
    algo->execute(app);

    // collect results and write to outpath
    if (vm.count("verbose"))
    {
      cout << "optcalex: Collecting results from parameter space grid ..."
        << endl
        << "optcalex: Writing result file ..."
        << endl;
    }

    std::ofstream ofs(outpath.string().c_str());
    // write header information
    // write header information for parameter space parameters
    std::vector<std::string> param_names(
        calex_config.get_gridSystemParameterNames<TcoordType>(*algo));
    for (auto cit(param_names.cbegin()); cit != param_names.cend(); ++cit)
    {
      ofs << std::setw(12) << std::fixed << std::left << *cit << " ";
    }
    ofs << "    ";
    // write header information of result data
    opt::Iterator<TcoordType, TresultType> it(
      algo->getParameterSpace().createIterator(opt::ForwardNodeIter));
    it.first();
    (*it)->getResultData().writeHeaderInfo(ofs);

    // write data
    while (!it.isDone())
    {
      // write search parameter
      std::vector<TcoordType> const& c = (*it)->getCoordinates();
      for (auto cit(c.cbegin()); cit != c.cend(); ++cit)
      {
        ofs << std::setw(12) << std::fixed << std::left << *cit << " ";
      }
      ofs << "    ";
      // write result data
      (*it)->getResultData().writeLine(ofs);

      ++it;
    }

    if (vm.count("verbose"))
    {
      cout << "optcalex: Calculations successfully finished." << endl;
    }
  }
  catch (std::string e) 
  {
    cerr << "ERROR: " << e << "\n";
    cerr << usage_text;
    return 1;
  }
  catch (std::exception& e) 
  {
    cerr << "ERROR: " << e.what() << "\n";
    cerr << usage_text;
    return 1;
  }
  catch (...)
  {
    cerr << "ERROR: Exception of unknown type!\n";
    cerr << usage_text;
    return 1;
  }

  return 0;
} // function main

/* ----- END OF optcalex.cc  ----- */
