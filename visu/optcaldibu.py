#!/usr/bin/env python2.7
## @file optcaldibu.py
# @brief Plot data calculated with optcalex.cc.
# 
# -----------------------------------------------------------------------------
# 
# $Id$
# @author Daniel Armbruster
# \date 26/03/2012
# 
# Purpose: Plot data calculated with optcalex.cc.  
#
# ----
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ----
# 
# Copyright (c) 2012 by Daniel Armbruster
# 
# REVISIONS and CHANGES 
# 26/03/2012  V0.1  Daniel Armbruster
# 05/04/2012  V0.2  --min option added
# 14/05/2012  V0.3  provide plotting legend and multiple files
# 15/05/2012  V0.4  handle header line of datafile
# 13/06/2012  V0.5  simple 2D representation availabel now
# 
# =============================================================================
"""
Plot optcalex.cc output data as 3D.
"""
 
import sys
import os
import getopt
import matplotlib
import pylab
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from matplotlib.mlab import griddata
from matplotlib.colors import Normalize
from matplotlib import rc
import matplotlib.pyplot as plt
import numpy as np

__version__ = "V0.5"
__subversion__ = "$Id$"
__license__ = "GPLv2+"
__author__ = "Daniel Armbruster"
__copyright__ = "Copyright (c) 2012 by Daniel Armbruster"

# -----------------------------------------------------------------------------
class Error(Exception):
  def __init__(self, msg=""):
    self.msg = msg
  def display(self):
    sys.stderr.write("optcaldibu (ERROR): " + self.msg + "\n")

class Usage(Error):
  def display(self):
    usage_text = "Version: "+__version__+"\nLicense: "+__license__+ \
      "\n"+__subversion__+"\nAuthor: "+__author__+ """
 Usage: optcaldibu [-v|--verbose] [-o|--overwrite] [-t|--title arg]
                   [--x-axis arg] [--y-axis arg] [--z-axis arg] 
                   [--x-label arg] [--y-label arg] [--z-label arg] 
                   [--scatter|--surface|--pcolor]
                   [--min] [--add-data arg] [--outbase arg] [--oformat arg]
                   [--legend arg] [--legpos arg] [--legfontsize arg]
                   DATAFILE [DATAFILE [...]]
    or: optcaldibu -h|--help\n"""
    if 0 != len(self.msg):
      sys.stderr.write("optcaldibu: " + self.msg + "\n")
    sys.stderr.write(usage_text)
    if 0 != len(self.msg):
      sys.stderr.write("\n")

# -----------------------------------------------------------------------------
def help():
  """
  Printing helptext to stdout.
  """
  help_text = \
  """
-------------------------------------------------------------------------------
 -v|--verbose         Be verbose.
 -h|--help            Display this help.
 --overwrite          Overwrite output file if existing.
 --outbase arg        Basename of the output file. If not specified there will
                      be plotted an interactive plot.
 --oformat arg        Format of the output file.
                      Supported formats: emf, eps, jpeg, jpg, pdf, png,
                      ps (default), raw, rgba, svg, svgz, tif, tiff.

 Plot appearance:
 -t|--title arg       Title of the plot.
 --x-axis arg         Set x-axis column in DATAFILE (default 0).
 --y-axis arg         Set y-axis column in DATAFILE (default 1).
 --z-axis arg         Set z-axis column in DATAFILE (default 3).
 --x-label arg        Set the label of the x-axis.
 --y-label arg        Set the label of the y-axis.  
 --z-label arg        Set the label of the z-axis.
                      The default labels are the header file information at the
                      corresponding column of the first DATAFILE passed.
 --scatter            Scattered plot (default).
 --surface            Surface plot.
 --pcolor             2D plot representation using matplotlibs pcolor function.
 --orientation arg    Orientation of the plot. Valid arguments are 'landscape'
                      and 'portrait' (default).
 -g|--grid            Enable grid.
 --min                Show minimal absolute values of z-component in plot. (Only
                      for scattered plots.)
 --legend arg         Specify legend arguments.
                      arg is a list of colon separated strings and must be
                      of the same size as the number of DATAFILE(s) passed.
                      By default plotting a legend is enabled.
                      To disable a legend use the '--legpos' commandline
                      argument. Additionally the legend position can be
                      specified using the '--legpos' commandline argument.
 --legpos arg         Set legend position or disable legend.
                      Valid arguments are 'unset' to disable legend, 'best',
                      'upper right' (default), 'upper left', 'lower left',
                      'lower right', 'right', 'center left', 'center right',
                      'lower center', 'upper center' and 'center'.
 --legfontsize arg    Set the fontsize of legend text. (arg of int type)
 DATAFILE(s)          File(s) which contain(s) the data. Up to now plotting of
                      seven datasets is provided. Each DATAFILE must contain
                      exactly one header line.\n"""
  Usage().display()
  sys.stdout.write(help_text)

# -----------------------------------------------------------------------------
def main(argv=None):
  if argv is None:
    argv = sys.argv
  try:
    try:
      opts, args = getopt.getopt(argv[1:], "hvogt:", ["help", "verbose", \
          "overwrite", "title=", "x-axis=", "y-axis=", "z-axis=", "scatter", \
          "surface", "pcolor", "min", "outbase=", "oformat=", "legend=", \
          "legpos=", "legfontsize=", "grid", "orientation=", "x-label=", \
          "y-label=", "z-label="])
    except getopt.GetoptError as err:
      raise Usage(err.msg)
    # fetch arguments
    verbose = False
    overwrite = False
    plotTitle=""
    X=0
    Y=1
    Z=3
    scatterPlot=False
    surfacePlot=False
    pcolorPlot=False
    showMinimum=False
    oformat = "ps"
    outbase = ""
    orientation = "portrait"
    legargs = args
    legpos = 'upper right'
    legfontsize = "12"
    # proxy artists cause legend not supported for 3D scattered plots
    colors = ('#800000', 'r', 'b','#808000', 'm', 'b', 'g', 'k')
    legproxies = [plt.Rectangle((0, 0), 1, 1, fc=colors[0]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[1]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[2]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[3]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[4]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[5]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[6]), \
        plt.Rectangle((0, 0), 1, 1, fc=colors[7])]
    grid = False
    xLabel = "" 
    yLabel = "" 
    zLabel = "" 
    #rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
    #rc('text', usetex=True)

    for opt, arg in opts:
      if opt in ("-v", "--verbose"):
        verbose = True
      elif opt in ("-h", "--help"):
        help()
        sys.exit()
      elif opt in ("-o", "--overwrite"):
        overwrite = True
      elif opt in ("--oformat"):
        oformat = arg
      elif opt in ("--outbase"):
        outbase = arg
      elif opt in ("--fit"):
        fit = "tight"
      elif opt in ("-g", "--grid"):
        grid = True
      elif opt in ("--orientation"):
        orientation = arg
        if orientation not in ("portrait", "landscape"):
          raise Usage("Invalid 'orientation' argument.")
      elif opt in ("-t", "--title"):
        plotTitle = arg
      elif opt in ("--x-axis"):
        X = int(arg)
      elif opt in ("--y-axis"):
        Y = int(arg)
      elif opt in ("--z-axis"):
        Z = int(arg)
      elif opt in ("--x-label"):
        xLabel = arg
      elif opt in ("--y-label"):
        yLabel = arg
      elif opt in ("--z-label"):
        zLabel = arg
      elif opt in ("--scatter"):
        scatterPlot = True
      elif opt in ("--surface"):
        surfacePlot = True
      elif opt in ("--pcolor"):
        pcolorPlot = True
      elif opt in ("--min"):
        showMinimum = True
      elif opt in ("--add-data"):
        addDataFilePath = arg
      elif opt in ("--legend"):
        legargs = arg.split(':')
        if len(legargs) != len(args):
          raise Usage("Invalid number of legend arguments.")
      elif opt in ("--legpos"):
        legpos = arg
      elif opt in ("--legfontsize"):
        try:
          legfontsize = int(arg)
        except:
          raise Usage("Invalid 'legfontsize' argument.")
      else:
        raise Usage("Unhandled option chosen.")

    if len(args) < 1:
      raise Usage("Invalid arguments.")

    if len(outbase) and os.access(outbase+"."+string.lower(oformat), os.F_OK) \
        and not overwrite:
      raise Usage(outbase+"."+string.lower(oformat)+" already exists.")

    # surface plot does not support '--min' option
    if (surfacePlot or pcolorPlot)and showMinimum:
      showMinimum = False
      if verbose:
        sys.stdout.write( \
            "optcaldibu: Surface or pcolor plot does not support option" \
            " '--min'.\n")

    # read data from datafile(s)
    all_data = []
    for datafile in args:
      if not os.access(datafile, os.F_OK):
        raise Error("Invalid path to DATAFILE '{0}'.".format(datafile))
      if 0 == os.stat(datafile).st_size:
        raise Error("Given DATAFILE '{0}' is an empty file.".format(datafile))
      if verbose:
        sys.stdout.write( \
            "optcaldibu: Reading file '{0}' ... \n".format(datafile))
      try:
        file = open(datafile, "r")
        lines = (line.strip().split() for line in file)
        header = lines.next()
        columns = [map(float, col) for col in zip(*lines)]
        if len(header) != len(columns):
          raise Error("Header info and data inconsistancy.")
        all_data.append(columns)
      except IOError as err:
        raise Error( \
            "[Errno "+str(err.errno)+"] "+err.strerror+": "+err.filename)
      else:
        file.close()

    # show minimum option
    if showMinimum and not (surfacePlot or pcolorPlot):
      if verbose:
        sys.stdout.write("optcaldibu: Extracting minimum data ... \n")
      min_data = []
      for data in all_data:
        # get indeces of minimal values
        minval = min(data[Z])
        minval_idxs = [i for i, v in enumerate(data[Z]) if v == minval]
        # extract all values to own columns
        min_x = [data[X][i] for i in minval_idxs]
        min_y = [data[Y][i] for i in minval_idxs]
        min_z = [data[Z][i] for i in minval_idxs]
        min_data.append((min_x, min_y, min_z))
        # pop values from original lists
        for idx in minval_idxs:
          for col in data:
            col.pop(idx)

    if verbose:
      if surfacePlot:
        sys.stdout.write("optcaldibu: Generating surface plot ... \n")
      elif pcolorPlot:
        sys.stdout.write("optcaldibu: Generating pcolor 2D plot ... \n")
      else:
        sys.stdout.write("optcaldibu: Generating scattered plot ... \n")

  # generate plot
    # 2D pcolor plot
    if pcolorPlot:
      if verbose:
          sys.stdout.write( \
              "optcaldibu: Only plotting data of first DATAFILE.\n")
      # basic example taken from http://matplotlib.sourceforge.net/examples/
      # pylab_examples/griddata_demo.html
      fig = plt.figure()
      ax = fig.add_subplot(111)
      data = all_data[0]
      dx = data[X][1]-data[X][0]
      dy = data[Y][1]-data[Y][0]
      xi = np.linspace(min(data[X])-dx, max(data[X])+dx, 500)
      yi = np.linspace(min(data[Y])-dy, max(data[Y])+dy, 300)
      zi = griddata(data[X], data[Y], data[Z], xi, yi, interp='nn')
      pylab.pcolor(xi, yi, zi, label=str(args[0]), \
          norm=Normalize(vmin=zi.min(), vmax=zi.max()))
      #    norm=Normalize(vmin=0.016, vmax=0.017))
      #plt.contourf(xi, yi, zi, label=str(args[0]))
      c = plt.colorbar(spacing='proportinal')
      plt.xlim(min(data[X]), max(data[X]))
      plt.ylim(min(data[Y]), max(data[Y]))
      #plt.xlim(365.45, max(data[X]))
      #plt.ylim(0.712, 0.7145)
      # axis labels
      if "" == xLabel:
        xLabel = header[X]
      if "" == yLabel:
        yLabel = header[Y]
      if "" == zLabel:
        zLabel = header[Z]
      ax.set_xlabel(xLabel)
      ax.set_ylabel(yLabel)
      c.set_label(zLabel)
      # grid
      if grid:
        sub.grid()

    # surface or scatter 3D plot
    else:
      fig = plt.figure()
      sub = fig.add_subplot(111)
      ax = Axes3D(fig)
      normalize_colormap = True
      for data, color in zip(all_data, colors):
        try:
          if surfacePlot:
            # surface plot - first render the data
            xi = np.linspace(min(data[X]), max(data[X]),100)
            yi = np.linspace(min(data[Y]), max(data[Y]),100)
            xim, yim = np.meshgrid(xi, yi)
            zi = griddata(data[X], data[Y], data[Z], xi, yi, interp='nn')

            surf = ax.plot_surface(xim, yim, zi, rstride=1,
                cstride=1, cmap=cm.jet, linewidth=0, antialiased=False)
            # normalize the colormap for all plots which means that one color
            # represents the same value in all datasets plotted
            if normalize_colormap:
              surf_norm = surf.norm
              normalize_colormap = False
            else:
              surf.set_norm(surf_norm)
              greatest = max(data[Z])
              smallest = min(data[Z])
              if greatest > surf_norm.vmax:
                surf_norm.vmax = greatest
              if smallest < surf_norm.vmin:
                surf_norm.vmin = smallest
          else:
            # scattered plot
            ax.scatter(data[X], data[Y], data[Z], c=color, marker='+')
            if showMinimum:
              # plot minimum data (diamonds)
              for min_dat, color in zip(min_data, colors):
                ax.scatter(min_dat[0], min_dat[1], min_dat[2], c=color, \
                   marker='D', s=40)
        except IndexError as err:
          raise Error(err.message)
      # print minimum values to terminal
      if showMinimum and verbose:
        for min_dat, legarg in zip(min_data, legargs):
          sys.stdout.write( \
              "optcaldibu: Minimal values for '{0}':\n".format(legarg))
          min_lines = zip(min_dat[0], min_dat[1], min_dat[2])
          for min_x,min_y,min_z in min_lines:
            sys.stdout.write("({0:6f}, {1:6f}, {2:6f})\n".format( \
              min_x,min_y,min_z))

      if grid:
        ax.grid()
      # set axis labels
      if "" == xLabel:
        xLabel = header[X]
      if "" == yLabel:
        yLabel = header[Y]
      if "" == zLabel:
        zLabel = header[Z]
      ax.set_xlabel(xLabel)
      ax.set_ylabel(yLabel)
      ax.set_zlabel(zLabel)

      # plot colorbar if surface plot specified (normalized colorbar)
      if surfacePlot:
        fig.colorbar(surf, shrink=0.5, aspect=5)
      # legend if scattered plot specified
      if 'unset' != legpos and not surfacePlot:
        try:
          if verbose:
            sys.stdout.write("optcaldibu: Plotting legend ...\n")
          ax.legend(legproxies[0:len(legargs)], legargs, loc=legpos, \
              prop={"size":legfontsize})
        except:
          raise Error("While plotting legend.")

    # plot title
    plt.title(plotTitle)
    # save plot to file
    if len(outbase):
      if verbose:
        sys.stdout.write( \
          "optcaldibu: Saving plot to file '{0}' ... \n".format( \
          outbase+"."+string.lower(oformat))) 
      try:
        plt.savefig(outbase+"."+string.lower(oformat), \
          orientation=orientation, format=oformat, bbox_inches=fit)
      except ValueError as err:
        raise Usage(err.message)
    else:
      if verbose:
        sys.stdout.write("optcaldibu: Generating interactive plot ... \n")
    plt.show()

  except Error as err:
    err.display()
    return 2
  except Usage as err:
    err.display()
    return 2

# -----------------------------------------------------------------------------
if __name__ == "__main__":
  sys.exit(main())

# ----- END OF optcaldibu.py ----- 
