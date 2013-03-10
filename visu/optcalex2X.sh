#!/bin/sh
# This is <optcalex2X.sh>
# -----------------------------------------------------------------------------
# $Id$
# 
# Copyright (c) 2012 by Daniel Armbruster (BFO Schiltach) 
# 
# Purpose: Generate a GMT postscript plot of the output of optcalex.
# Conversion to other formats is supported as well using ps2raster.
# 
# REVISIONS and CHANGES 
#    20/09/2012   V1.0   Daniel Armbruster
#    07/09/2012   V1.1   Contour plots provided. Batch plotting provided.
#    10/10/2012   V1.2   Provide '-W' and '-T', '-t' options.
# 
# =============================================================================
#
MINPARAMS=2

# -----------------------------------------------------------------------------
# DEFAULT VALUES
# -----------------------------------------------------------------------------
# column for data in datafile
cols=(1 2 4)
# labels
labels=("Eigenperiod T@-0@- / s" "Damping h" "rms misfit")
# grid spacing
gridSpacing="0.012/0.0004"
# viewpoint settings
viewPointAzimuth=120
viewPointInclination=30
# master color table 
colorTable=jet
# select output format
fflag=
fARG=
# generate contour plot
contourPlot=
# fixed region of interest
Rflag=

# help function
function usage {
  echo -ne "This is <optcalex2X.sh>\n" >&2
  echo -ne "\$Id$\n" >&2
  echo -ne "Copyright (c) 2012 by Daniel Armbruster\n" >&2
  echo -ne "Purpose: Generate a 3D GMT plot from optcalex output files.\n" >&2
  echo -ne "Usage: `basename $0` [-h] [-v] [-f ARG] [-c ARG] [-l ARG]\n" >&2
  echo -ne "\t          [-R ARG] [-V ARG] [-Z] [-C] [-H] [-W] [-T ARG]\n" >&2
  echo -ne "\t          [-t ARG] BASENAME DATAFILE [DATAFILE [...]]\n" >&2
  echo -ne "------------------------------------------------------------\n" >&2
  echo -ne "\t-h          \tDisplay this help.\n" >&2
  echo -ne "\t-v          \tBe verbose.\n" >&2
  echo -ne "\t-f ARG      \tConvert output implicitly to ARG format.\n" >&2
  echo -ne "\t            \tARG will be passed directl to the GMT\n" >&2
  echo -ne "\t            \tps2raster program. See also 'man ps2raster'.\n" >&2
  echo -ne "\t-c Cx:Cy:Cz \tData columns to plot. Default is 1:2:4.\n" >&2
  echo -ne "\t-l L1:L2:L3 \tSet axis labels to values passed.\n" >&2
  echo -ne "\t-R ARG      \tFix axis range of GMT plot.\n" >&2
  echo -ne "\t-V Az:Incl  \tSet viewpoint. Values for azimuth and\n" >&2
  echo -ne "\t            \tinclination in degrees.\n" >&2
  echo -ne "\t-C          \tGenerate contour plot instead of surface\n" >&2
  echo -ne "\t            \tplot.\n"
  echo -ne "\t-H          \tPlot horizontal colorbar.\n" >&2
  echo -ne "\t-T Tx:Ty:Tz \tSet fixed tickmark intervals.\n" >&2
  echo -ne "\t-W          \tDraw contour lines on top of surface.\n" >&2
  echo -ne "\t-Z          \tContinous color palette.\n" >&2
  echo -ne "\t-t ARG      \tPlot title (see also pstext). If empty no\n" >&2
  echo -ne "\t            \ttitle will be plotted.\n" >&2
  echo -ne "\tBASENAME    \tBasename opf output file. An index will be\n" >&2
  echo -ne "\t            \tappended.\n" >&2
  echo -ne "\tDATAFILE    \toptcalex outputfile to plot.\n" >&2
  echo -ne "Copyright © 2007 Free Software Foundation, Inc\n" >&2
  echo -ne "<http://www.gnu.org/licenses/gpl.html>\n" >&2

  if [ ${hflag} ]
  then
    exit 0
  else
    exit 2
  fi
}

# fetch commandline arguments
while getopts 'hvc:l:f:R:V:ZCHWT:t:' Option;
do
  case ${Option} in
    h) hflag=1; usage exit 0;;
    v) verbose=1;;
    c) cols[0]=`echo ${OPTARG} | cut -f 1 -d ':' | tr -d ' '`
      cols[1]=`echo ${OPTARG} | cut -f 2 -d ':' | tr -d ' '`
      cols[2]=`echo ${OPTARG} | cut -f 3 -d ':' | tr -d ' '`;;
    f) fflag=1; fARG=${OPTARG};;
    l) labels[0]=`echo ${OPTARG} | cut -f 1 -d ':'`
      labels[1]=`echo ${OPTARG} | cut -f 2 -d ':'`
      labels[2]=`echo ${OPTARG} | cut -f 3 -d ':'`;;
    R) Rflag=1; gmtRange=${OPTARG};;
    V) viewPointAzimuth=`echo ${OPTARG} | cut -f 1 -d ':' | tr -d ' '`
      viewPointInclination=`echo ${OPTARG} | cut -f 2 -d ':' | tr -d ' '`;;
    C) contourPlot=1;;
    H) horizontalColorbar=1;;
    Z) gmtARGs+=" -Z";;
    W) Wflag=1;;
    T) Tflag=1; ticks[0]=`echo ${OPTARG} | cut -f 1 -d ':' | tr -d '[:alpha:]'`
      ticks[1]=`echo ${OPTARG} | cut -f 2 -d ':' | tr -d '[:alpha:]'`
      ticks[2]=`echo ${OPTARG} | cut -f 3 -d ':' | tr -d ' '`;;
    t) tflag=1; title=${OPTARG};;
    *) echo -e "ERROR: Unimplemented option chosen.\n"
      usage exit 2;;
  esac
done
shift $((${OPTIND} - 1))
# check number of commandline arguments
if [ $# -lt $MINPARAMS ]
then
  echo "ERROR: Missing arguments."
  exit 2
fi

j=0
plotNums=($(seq -w 1 $(($#-1))))
DATAFILES=`echo $@ | cut -f 2-$# -d ' '`
for datafile in ${DATAFILES}
do
  OUTBASE=$1-${plotNums[$j]}
  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): Processing file: '${datafile}' ..." >&2
    echo "$(basename $0): (${datafile}): Output basename: '${OUTBASE}'" >&2
  fi
  if [ ! -f ${datafile} ]
  then
    echo -e "ERROR: '${datafile}' not found.\n"
  fi

  tmp=$(minmax -C ${datafile})
  if [ -z ${Rflag} ]
  then
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Computing region of interest ..."
    fi
    # get region interest
    idx=$((${cols[0]}*2))
    colxRegion=$(echo ${tmp} | cut -f "$((${idx}-1)) ${idx}" -d ' ' | \
      sed -e 's/\ /\//')
    idx=$((${cols[1]}*2))
    colyRegion=$(echo ${tmp} | cut -f "$((${idx}-1)) ${idx}" -d ' ' | \
      sed -e 's/\ /\//')
    gmtRange="${colxRegion}/${colyRegion}"
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Region of interest: ${gmtRange}"
    fi
  fi

  # compute number of tickmarks
  if [ -z ${Tflag} ]
  then
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Computing number of tickmarks ..."
    fi
    ticks[0]=$(echo ${tmp} | awk -v c=$((${cols[0]}*2)) \
      'function abs(val) {return (val<0?-val:val);}; \
      {printf "%.1E", abs($(c-1)-$(c))/6.}')
    ticks[1]=$(echo ${tmp} | awk -v c=$((${cols[1]}*2)) \
      'function abs(val) {return (val<0?-val:val);}; \
      {printf "%.2f", abs($(c-1)-$(c))/6.}')
    ticks[2]=$(echo ${tmp} | awk -v c=$((${cols[2]}*2)) \
      'function abs(val) {return (val<0?-val:val);}; \
      {dz=abs($(c-1)-$(c))/5.;} {printf "%.2f", dz}')
  else
    # use fixed tickmarks
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Using fixed tickmarks."
    fi
  fi

  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): (${datafile}): Tickmark dx: ${ticks[0]}" >&2
    echo "$(basename $0): (${datafile}): Tickmark dy: ${ticks[1]}" >&2
    echo "$(basename $0): (${datafile}): Tickmark dz: ${ticks[2]}" >&2
  fi

  # compute grid spacing
  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): (${datafile}): Compute grid spacing ..." >&2
  fi
  val[0]=$(awk -v c=${cols[0]} '{print $(c)}' ${datafile} | sort | \
    uniq -d | head -n1)
  val[1]=$(awk -v c=${cols[0]} '{print $(c)}' ${datafile} | sort | \
    uniq -d | head -n2 | tail -n1)
  val[2]=$(awk -v c=${cols[1]} '{print $(c)}' ${datafile} | sort | \
    uniq -d | head -n1)
  val[3]=$(awk -v c=${cols[1]} '{print $(c)}' ${datafile} | sort | \
    uniq -d | head -n2 | tail -n1)
  gridSpacing=$(echo | awk -v v1=${val[0]} -v v2=${val[1]} -v v3=${val[2]} \
    -v v4=${val[3]} 'function abs(val) {return (val<0?-val:val);}; \
    {print abs(v1-v2)"/"abs(v3-v4)}')
  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): (${datafile}): Grid spacing is: ${gridSpacing}" >&2
  fi

  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): (${datafile}): Generate plot ..."
  fi
  # generate surface of data
  awk -v colx=${cols[0]} -v coly=${cols[1]} -v colz=${cols[2]} \
    'NR>1 {print $(colx),$(coly),$(colz)}' ${datafile} | \
    surface -G/tmp/${OUTBASE}.nc -R${gmtRange} -I${gridSpacing}
  # make color palette
  grd2cpt /tmp/${OUTBASE}.nc -C${colorTable} ${gmtARGs[*]} \
    > /tmp/${OUTBASE}.cpt

  # generate surface plot
  if [ -z ${contourPlot} ]
  then
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Draw surface plot ..."
    fi
    # compute illumination settings
    grdgradient /tmp/${OUTBASE}.nc -A$((${viewPointAzimuth}+90)) \
      -G/tmp/${OUTBASE}.illumination
    if [ ! -z ${Wflag} ]
    then
      if [ ! -z ${verbose} ]
      then
        echo "$(basename $0): (${datafile}): Draw contour lines ..."
      fi
      # contour on bottom with 
      grdcontour /tmp/${OUTBASE}.nc -C/tmp/${OUTBASE}.cpt -A- -JX4i -W+ \
        -B${ticks[0]}:"${labels[0]}":/${ticks[1]}:"${labels[1]}":SEwnZ \
        -R${gmtRange} -Y2.1i -E${viewPointAzimuth}/${viewPointInclination} -K \
        > ${OUTBASE}.ps 
      grdview /tmp/${OUTBASE}.nc -J -JZ2.7i -I/tmp/${OUTBASE}.illumination \
        -C/tmp/${OUTBASE}.cpt -R -Qi300 -S -Y0i -K -O \
        -E${viewPointAzimuth}/${viewPointInclination} >> ${OUTBASE}.ps
    else
      grdview /tmp/${OUTBASE}.nc -JX4i -JZ2.7i \
        -B${ticks[0]}:"${labels[0]}":/${ticks[1]}:"${labels[1]}":SEwnZ \
        -I/tmp/${OUTBASE}.illumination -C/tmp/${OUTBASE}.cpt \
        -R${gmtRange} -Qi300 -S -Y2. -K ${grdViewARG} \
        -E${viewPointAzimuth}/${viewPointInclination} > ${OUTBASE}.ps
    fi
  else
  # generate contour plot
    if [ ! -z ${verbose} ]
    then
      echo "$(basename $0): (${datafile}): Draw contour plot ..."
    fi
    grdview /tmp/${OUTBASE}.nc -JX5i -Y2.8i \
      -B${ticks[0]}:"${labels[0]}":/${ticks[1]}:"${labels[1]}":SEwnZ \
      -C/tmp/${OUTBASE}.cpt -R${gmtRange} -Qi300 -S -K > ${OUTBASE}.ps
    if [ ! -z ${Wflag} ]
    then
      if [ ! -z ${verbose} ]
      then
        echo "$(basename $0): (${datafile}): Draw contour lines ..."
      fi
      grdcontour /tmp/${OUTBASE}.nc -A- -C/tmp/${OUTBASE}.cpt -J -R -Y -O -K \
        >> ${OUTBASE}.ps 
    fi
  fi

  # plot title
  if [ -z ${tflag} ]
  then
    title="0 1.3 18 0 0 BL ${datafile}"
  fi
  if [ ! -z ${verbose} ] && [ ! -z "${title}" ]
  then
    echo "$(basename $0): (${datafile}): Draw title ..."
  fi
  if [ ! -z "${title}" ]
  then
    echo "${title}" | pstext -R0/30/0/30 -Jx4i -O -K >> ${OUTBASE}.ps 
  fi

  # plot colorbar
  if [ -z ${horizontalColorbar} ] && [ -z ${contourPlot} ]
  then
    OPT="-D6.8i/2.7i/7.5c/1.25c -I/tmp/${OUTBASE}.illumination"
  elif [ ! -z ${horizontalColorbar} ] && [ -z ${contourPlot} ] 
  then
    OPT="-D2.75i/-0.5i/4i/1.25ch -I/tmp/${OUTBASE}.illumination"
  elif [ -z ${horizontalColorbar} ] && [ ! -z ${contourPlot} ]
  then
    OPT="-D7i/2.6i/7.5c/1.25c"
  elif [ ! -z ${horizontalColorbar} ] && [ ! -z ${contourPlot} ]
  then
    OPT="-D2.5i/-1.0i/4i/1.25ch"
  fi

  if [ ! -z ${verbose} ]
  then
    echo "$(basename $0): (${datafile}): Draw colorbar ..."
  fi
  tsmall=$(echo | awk -v v=${ticks[2]} '{print v/2.}')
  psscale ${OPT} -O -C/tmp/${OUTBASE}.cpt \
    -B${ticks[2]}f${tsmall}:"${labels[2]}": >> ${OUTBASE}.ps 
  if [ ! -z ${fflag} ]
  then
    ps2raster ${OUTBASE}.ps -A -T${fARG} -P
  fi

  rm -rf /tmp/${OUTBASE}.nc /tmp/${OUTBASE}.cpt /tmp/${OUTBASE}.illumination
  let j+=1
done

# ----- END OF optcalex2X.sh ----- 
