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
 * 19/03/2012  V0.1  Daniel Armbruster
 * 
 * ============================================================================
 */
 
#define OPTCALEX_VERSION "V0.1"
#define OPTCALEX_LICENSE "GPLv2"

#include <vector>
#include <iomanip>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <optimizexx/parameter.h>
#include <optimizexx/standardbuilder.h>
#include <optimizexx/iterator.h>
#include <optimizexx/globalalgorithms/gridsearch.h>
#include <calexxx/calexvisitor.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace opt = optimize;

using std::cout;
using std::cerr;
using std::endl;

typedef double TcoordType;
typedef calex::CalexResult TresultType;

/*!
 * \todo Variable parameter range specification necessary
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
    "                  --calib-in arg --calib-out arg OUTFILE" "\n"
    "     or: optcalex -h|--help" "\n"
    "     or: optcalex -V|--version" "\n"
  };

  try
  {
    // options which will be allowed on commandline
    //
    /* ISSUE (regarding calex application configuration): 
     * 1. I need the option to specify subsystems to add to the calex
     * application.
     * [--subsys-fo 'Type|val|unc'] first order subsystems
     * [--subsys-so 'Type|val|unc|val|unc'] first order subsystems
     * If the option will be unkown which means that it will be a parameter in
     * the parameter space then I the specification of 'val' isn't
     * necessery. -> Then just put 'xxx' instead of val as the argument.
     * 2. Allow specification of further unknown system parameters
     * [--add-param 'nam|val|unc']
     * 3. Option to specify the parameters searching for including
     * their ranges.
     * [--var-param 'nam|start|stop|delta']
     * -> Problem if I've got two subsystems with two 'per' system
     * parameters - how to treat this special case?
     * -> Then the order of the parameters appearing on the commandline is the
     * striking argument.
     *
     * Notice that the calex application doesn't need to know anything about the
     * range the parameter is varied. This knowledge only is important for the
     * optcalex.cc itself. Though there must be taken care the parameter varied
     * is a valid parameter in the calex configuration file. Another side effect
     * is that parameters varied do need to be passed as a commandline argument,
     * but the uncertainty has to be passed.
     *
     * IDEA: I could allow specifying system parameters for a CalexConfig class
     * and system parameters of subsystems with an unkown flag. Done. Now the
     * only necessity is to guarantee that the order of the parameters and the
     * order of the variable system parameters is equal. How to solve this
     * problem?
     * - One approach would be to pass a vector to the calex application which
     * describes the parameter order. Default then is the chronological order.
     * A drawback would be if using this approach that the problems with
     * system parameters which are part of any kind of subsystem is not solved
     * really yet.
     * On the other hand there is still the problem how the application is able
     * to set the correct variable in the CalexConfig class using its member
     * access functions.
     *
     * IMPORTANT NOTE:
     * How would it be if CalexApplication is the subject so that class
     * CalexConfig is the observer which changes its state every time
     * CalexApplication calls CalexConfigs update function.
     * -> Problem I just would delegate the problem of calling the correct
     *  functions or rather setting the correct values to class CalexConfig.
     *  UP to now I don't see any way to provide a functionalism for a
     *  completely dynamical CalexApplication because always I've got the same
     *  issues regarding the knowledge of setting the correct functions.
     *
     * SOLUTION:
     * - Class CalexConfig could provide a member access function called
     * get_unknown() so that CalexApplication only could call this function to
     * set the appropiated values. CalexApplication and Calexconfig so would be
     * coupled really tightly. The order of how to return the system parameters
     * then must be provided to CalexConfig. This responsability will take
     * optcalex.cc.
     *
     * - Another approach:
     * CalexApplication only calls a function
     * Mconfig->update(std::vector<params>) so that the configuration file 
     * sets/updates within this function its unknown system parameters. As above
     * the order of the parameters must be passed of optcalex.cc to class
     * CalexConfig.
     */
    po::options_description desc("Allowed options", 80);
    desc.add_options()
      ("version,V", "Show version of optcalex.")
      ("verbose,v",po::value<int>()->implicit_value(1), "Be verbose.")
      ("help,h", "Print this help.")    
      ("overwrite,o", "overwrite OUTFILE")
      ("config-file", po::value<fs::path>()->default_value(
        "~/.optimize/optcalex.rc"), "Path to optcalex configuration file.")
      ("maxit,m", po::value<int>()->default_value(80),
       "Number of maximum iterations of calex for each parameter "
       "configuration.")
      ("calib-in", po::value<fs::path>()->required(),
       "Filename of calibration input signal file.")
      ("calib-out", po::value<fs::path>()->required(), 
       "Filename of calibration output signal file.")
      ("output-file", po::value<fs::path>()->required(),
       "Filepath of OUTFILE.")
    ;

    po::positional_options_description p;
    p.add("output-file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(iargc, argv).options(
          desc).positional(p).run(), vm);
    // help requested? print help
    if (vm.count("help"))
    {
      cout << usage_text
        << "------------------------------------------------------------\n";
      cout << desc;
      exit(0);
    } else
    if (vm.count("version"))
    {
      cout << "$Id$" << endl;
      cout << "Version: " << OPTCALEX_VERSION << endl;
      exit(0);
    }
    po::notify(vm);

    // fetch commandline arguments
    fs::path outpath(vm["output-file"].as<fs::path>());
    if (fs::exists(outpath) && ! vm.count("overwrite"))
    {
      throw std::string("OUTFILE existing. Specify option 'overwrite'.");
    }
    fs::path calibInfile(vm["calib-in"].as<fs::path>());
    fs::path calibOutfile(vm["calib-out"].as<fs::path>());

    if (vm.count("verbose"))
    {
      cout << "optcalex: Commandline arguments collected." << endl 
        << "  calib-in:  " << calibInfile.string() << endl 
        << "  calib-out: " << calibOutfile.string() << endl
        << "  out-file:  " << outpath.string() << endl;
    }

    // mayor part
    // ----------
    // parameters
    opt::Parameter<TcoordType> per(119., 121., 0.2, "eigenperiod", "s");
    opt::Parameter<TcoordType> dmp(0.65, 0.75, 0.01, "damping");

    std::vector<opt::Parameter<TcoordType> > params;
    params.push_back(per);
    params.push_back(dmp);

    // parameter space builder
    opt::ParameterSpaceBuilder<TcoordType, TresultType>* builder =
      new opt::StandardParameterSpaceBuilder<TcoordType, TresultType>;

    // gridsearch algorithm
    opt::GlobalAlgorithm<TcoordType, TresultType>* algo = 
      new opt::GridSearch<TcoordType, TresultType>(builder, params);

    // application
    calex::CalexConfig config(calibInfile.string(), calibOutfile.string());
    config.set_maxit(vm["maxit"].as<int>());
    calex::CalexApplication<TcoordType> app(&config, 1., 0.01);

    algo->constructParameterSpace();
    algo->execute(app);
    
    // collect results and write to outpath
    std::ofstream ofs(outpath.string().c_str());
    opt::CompositeIterator<TcoordType, TresultType>* it = 
      algo->getParameterSpace().createIterator(opt::NodeIter);

    for (it->first(); !it->isDone(); it->next())
    {
      std::vector<TcoordType> const& c = it->currentItem()->getCoordinates();
      for (std::vector<TcoordType>::const_iterator cit(c.begin());
          cit != c.end(); ++cit)
      {
        ofs << std::setw(12) << std::fixed << std::left << *cit << " ";
      }
      ofs << "    ";
      it->currentItem()->getResultData().writeLine(ofs);
    }

    delete algo;
    delete builder;

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
