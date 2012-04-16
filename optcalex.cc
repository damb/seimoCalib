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
 * 
 * ============================================================================
 */
 
#define OPTCALEX_VERSION "V0.2"
#define OPTCALEX_LICENSE "GPLv2"

#include <vector>
#include <iomanip>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <optimizexx/parameter.h>
#include <optimizexx/standardbuilder.h>
#include <optimizexx/iterator.h>
#include <optimizexx/globalalgorithms/gridsearch.h>
#include <calexxx/calexvisitor.h>
#include <calexxx/defaults.h>
#include <calexxx/parser.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace opt = optimize;

using std::cout;
using std::cerr;
using std::endl;

typedef double TcoordType;
typedef calex::CalexResult TresultType;

/*!
 * \todo Make use of boost::program_options configuration file parser
 */
int main(int iargc, char* argv[])
{

  // define usage information
  char usage_text[]=
  {
    "Version: "OPTCALEX_VERSION "\n"
    "License: "OPTCALEX_LICENSE "\n" 
    "    SVN: $Id$\n" 
    " Author: Daniel Armbruster" "\n"
    "  Usage: optcalex [-v|--verbose] [-o|--overwrite] " "\n"
    "                  [--config-file arg] [--maxit arg]" "\n"
    "                  [--alias arg] [--qac arg] [--finac arg]" "\n"
    "                  [--ns1 arg] [ns2 arg] [--m0 arg]" "\n"
    "                  [--amp-param arg] [--del-param arg |--sub-param arg]\n"
    "                  [--til-param arg] [--sys-param arg]" "\n"
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
    " --sys-param=nam|val|unc\n"
    "where\n"
    "'nam' is a unique id of the system parameter (three letters)\n"
    "'val' is the initial value of the system parameter\n"
    "'unc' is the uncertainty.\n"
    "If 'unc' is set to zero it will not be optimized by Erhard Wielandt's\n"
    "calex program. To avoid ambiguities never use one of the following ids:\n"
    "'amp', 'del', 'sub' or 'til' together with the option --sys-param.\n"
    "Those ids belong to obligatory system parameters.\n"
    "Besides an usual system parameter there are as well system parameters\n"
    "treated as grid system parameters which means that their 'val'\n"
    "parameter will be scanned of the optcalex optimization algorithm.\n"
    "Grid system parameters must be specified with the following syntax:\n"
    " --sys-param=nam|start;end;delta|unc\n"
    "where\n"
    "'start' defines the start of the scanning range.\n"
    "'end'   defines the end of the scanning range.\n"
    "'delta' defines the interval the scanning range will be scanned.\n\n"
    "Additionally to common system parameters calex has four obligatory\n"
    "system parameters:\n"
    "'amp'\n"
    "'del'\n"
    "'sub'\n"
    "'til'\n"
    "Their meanings are explained in Erhard Wielandt's calex program\n"
    "documentation. Notice that if specifying both 'del' and 'sub' the\n"
    "system parameter passed as the latter one will disable the previous\n"
    "one.\n\n"
    "Configuring obligatory arguments is done with the following syntax. To\n"
    "set i.e. the 'amp' system parameter to an usual system parameter use:\n"
    " --amp-param=val|unc\n"
    "and in case it is desired to use the 'amp' system parameter as a grid\n"
    "system parameter then pass the arguments the order as follows:\n"
    " --amp-param=start;stop;end|unc\n\n"
    "To describe a system there are first and second order subsystems.\n"
    "----\n"
    "First order subsystems:\n"
    "To specify a first order subsystem the following syntax should be used.\n"
    " --first-order=[LP,HP]|val|unc\n"
    "Of course there is the possibility to specify the subsystem with a grid\n"
    "system parameter, too. Just use the syntax below.\n"
    " --first-order=[LP,HP]|start;end;delta|unc\n"
    "----\n"
    "Second order subsystems:\n"
    "The syntax to define a second order subsystem does not differ from the\n"
    "syntax of first order subsystems. The only difference is the fact that\n"
    "for second order subsystems only one system parameter might be a grid\n"
    "system parameter.\n"
    "So in case both parameters describing the subsystems are system\n"
    "parameters use\n"
    " --second-order=[LP,HP,BP]|val1|unc1|val2|unc2\n"
    "If the first parameter is a grid system parameter use\n"
    " --second-order=[LP,HP,BP]|start1;end1;delta1|unc1|val2|unc2\n"
    "and if the latter system parameter is a grid system parameter use\n"
    " --second-order=[LP,HP,BP]|val1|unc1|start2;end2;delta2|unc2\n"
    "In the special case that both parameters are grid system parameters the\n"
    "syntax is as follows:\n"
    " --second-order=[LP,HP,BP]|start1;end1;delta1|unc1|start2;end2;delta2|unc2\n"
    "-------------------------------------------------------------\n"
    "| NOTICE: Separators must be passed exactly as shown above. |\n"
    "-------------------------------------------------------------\n"
  };

  try
  {
    
    fs::path configFilePath;
    fs::path defaultConfigFilePath(std::string(getenv("HOME")));
    defaultConfigFilePath /= ".optimize";
    defaultConfigFilePath /= "optcalex.rc";


    // declare only commandline options
    po::options_description generic("Commandline options");
    generic.add_options()
      ("version,V", "Show version of optcalex.")
      ("help,h", "Print this help.")
      ("xhelp", "Print extended help text.")
      ("verbose,v",po::value<int>()->implicit_value(1), "Be verbose.")
      ("overwrite,o", "overwrite OUTFILE")
      ("config-file", po::value<fs::path>(&configFilePath)->default_value(
        defaultConfigFilePath), "Path to optcalex configuration file.")
      ;

    // declare both commandline and configuration file options
    po::options_description config(
        "Both Commandline and optcalex configuration file options");
    config.add_options()
      ("alias", po::value<double>()->default_value(CALEX_ALIAS),
       "Period of anti-alias filter")
      ("qac", po::value<double>()->default_value(CALEX_QAC),
       "iteration stops when improvement in the rms misfit in one step becomes"
       "less than qac")
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
      ("sys-param", po::value<std::vector<std::string> >(),
       "Additional system parameter.")
      ("amp-param", po::value<std::string>(), 
       "Configure amplitude 'amp' system parameter.")
      ("del-param", po::value<std::string>(), 
       "Configure delay 'del' system parameter.")
      ("sub-param", po::value<std::string>(), 
       "Configure 'sub' system parameter.")
      ("til-param", po::value<std::string>(), 
       "Configure 'til' system parameter.")
      ("first-order", po::value<std::vector<std::string> >(),
       "Add first order subsystem to calex parameter file.")
      ("second-order", po::value<std::vector<std::string> >(),
       "Add second order subsystem to calex parameter file.")
      ("calib-in", po::value<fs::path>()->required(),
       "Filename of calibration input signal file.")
      ("calib-out", po::value<fs::path>()->required(), 
       "Filename of calibration output signal file.")
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
      cout << "Version: " << OPTCALEX_VERSION << endl;
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
      ! fs::exists(fs::current_path() /= calibInfile)) 
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
    if (vm.count("sys-param"))
    {
      std::vector<std::string> sys_params =
        vm["sys-param"].as<std::vector<std::string> >();
      for (std::vector<std::string>::const_iterator cit(sys_params.begin());
          cit != sys_params.end(); ++cit)
      {
        calex::SystemParameter* sys_param = 0;
        calex::parser::systemParameterParser(*cit, &sys_param);
        calex_config.add_systemParameter(*sys_param);
      }
    }

    calex::SystemParameter* default_amp_param =
      new calex::SystemParameter("amp", CALEX_AMP, CALEX_AMPUNC);
    calex::SystemParameter* default_del_param =
      new calex::SystemParameter("del", CALEX_DEL, CALEX_DELUNC);
    calex::SystemParameter* default_sub_param =
      new calex::SystemParameter("sub", CALEX_SUB, CALEX_SUBUNC);
    calex::SystemParameter* default_til_param =
      new calex::SystemParameter("til", CALEX_TIL, CALEX_TILUNC);
    if (vm.count("amp-param"))
    {
      calex::SystemParameter* amp_param = 0;
      calex::parser::systemParameterParser(vm["amp-param"].as<std::string>(),
          &amp_param, "amp");
      calex_config.set_amp(*amp_param);
      delete default_amp_param;
    } else
    {
      calex_config.set_amp(*default_amp_param);
    }
    if (vm.count("del-param"))
    {
      calex::SystemParameter* del_param = 0;
      calex::parser::systemParameterParser(vm["del-param"].as<std::string>(),
          &del_param, "del");
      calex_config.set_del(*del_param);
      delete default_del_param;
    } else
    {
      calex_config.set_del(*default_del_param);
    }
    if (vm.count("sub-param"))
    {
      calex::SystemParameter* sub_param = 0;
      calex::parser::systemParameterParser(vm["sub-param"].as<std::string>(),
          &sub_param, "sub");
      calex_config.set_sub(*sub_param);
      delete default_sub_param;
    } else
    {
      calex_config.set_sub(*default_sub_param);
    }
    if (vm.count("til-param"))
    {
      calex::SystemParameter* til_param = 0;
      calex::parser::systemParameterParser(vm["til-param"].as<std::string>(),
          &til_param, "til");
      calex_config.set_til(*til_param);
      delete default_til_param;
    } else
    {
      calex_config.set_til(*default_til_param);
    }
    // fetch first order subsystem commandline arguments
    if (vm.count("first-order"))
    {
      std::vector<std::string> first_order =
        vm["first-order"].as<std::vector<std::string> >();
      for (std::vector<std::string>::const_iterator cit(first_order.begin());
          cit != first_order.end(); ++cit)
      {
        calex::CalexSubsystem* subsys = 0;
        calex::parser::firstOrderParser(*cit, &subsys);
        calex_config.add_subsystem(*subsys);
      }
    }
    // fetch second order subsystem commandline arguments
    if (vm.count("second-order"))
    {
      std::vector<std::string> second_order =
        vm["second-order"].as<std::vector<std::string> >();
      for (std::vector<std::string>::const_iterator cit(second_order.begin());
          cit != second_order.end(); ++cit)
      {
        calex::CalexSubsystem* subsys = 0;
        calex::parser::secondOrderParser(*cit, &subsys);
        calex_config.add_subsystem(*subsys);
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

    if (! calex_config.hasGridSystemParameters())
    {
      throw std::string("No grid system parameters specified.");
    }

    /* --------------------------------------------------------------------- */
    // mayor part
    
    if (vm.count("verbose"))
    {
      cout << "optcalex: Setting up parameter space ..." << endl;
    }
    // create parameter space builder
    opt::ParameterSpaceBuilder<TcoordType, TresultType>* builder =
      new opt::StandardParameterSpaceBuilder<TcoordType, TresultType>;

    // gridsearch algorithm
    opt::GlobalAlgorithm<TcoordType, TresultType>* algo = 
      new opt::GridSearch<TcoordType, TresultType>(builder);

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
    std::ofstream ofs(outpath.string().c_str());
    opt::Iterator<TcoordType, TresultType> it = 
      algo->getParameterSpace().createIterator(opt::ForwardNodeIter);
    if (vm.count("verbose"))
    {
      cout << "optcalex: Collecting results from parameter space grid ..."
        << endl
        << "optcalex: Writing result file ..."
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
      (*it)->getResultData().writeLine(ofs);
    }

    delete algo;
    delete builder;

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
