# this is <Makefile>
# ----------------------------------------------------------------------------
# $Id$
# 
# Copyright (c) 2012 by Daniel Armbruster (BFO Schiltach) 
# 
# optnonlin - use liboptimizexx to determine seimometer behaviour basing on a
# nonlinear model
#
# ----
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# ----
# 
# REVISIONS and CHANGES 
#    19/04/2012   V1.0   Daniel Armbruster
# 
# ============================================================================
#

PROGRAMS=optnonlin

.PHONY: all
all: install doxydoc

.PHONY: install
install: $(addprefix $(LOCBINDIR)/,$(PROGRAMS))
$(LOCBINDIR)/%: %
	mkdir -pv $(LOCBINDIR)
	/bin/mv -fv $< $(LOCBINDIR)

# =============================================================================
#
CHECKVAR=$(if $($(1)),,$(error ERROR: missing variable $(1)))
CHECKVARS=$(foreach var,$(1),$(call CHECKVAR,$(var)))
#
$(call CHECKVARS,LOCINCLUDEDIR LOCLIBDIR LOCBINDIR)
$(call CHECKVARS, BOOST_FILESYSTEM_VERSION)

FLAGS= -DBOOST_FILESYSTEM_VERSION=$(BOOST_FILESYSTEM_VERSION)
FLAGS += $(MYFLAGS) -std=c++0x
CFLAGS += -march=native -O2 -fno-reorder-blocks -fno-reorder-functions -pipe

CXXFLAGS += -Wall $(FLAGS)
LDFLAGS+=$(addprefix -L,$(LOCLIBDIR))
CPPFLAGS+=$(addprefix -I,$(LOCINCLUDEDIR)) $(FLAGS)

SRC=$(wildcard *.cc)
OBJ=$(patsubst %.cc,%.o,$(SRC))

#----------------------------------------------------------------------
# standard edit targets
flist: Makefile $(wildcard *.cc *.h) README doxydoc.cfg \
  $(filter-out %.bak, $(wildcard README.*))
	echo $^ | tr ' ' '\n' | sort > $@

.PHONY: edit
edit: flist; vim $<

.PHONY: clean
clean: ; 
	-find . -name \*.bak | xargs --no-run-if-empty /bin/rm -v
	-/bin/rm -vf flist
	-/bin/rm *.o *.bak *.d

#======================================================================
# dependencies
# ------------

%.d: %.cc
	$(SHELL) -ec '$(CXX) -M $(CPPFLAGS) $< \
      | sed '\''s,\($(notdir $*)\)\.o[ :]*,$(dir $@)\1.o $@ : ,g'\'' \
        > $@; \
      [ -s $@ ] || rm -f $@'

-include $(patsubst %.cc,%.d,$(SRC))

#======================================================================

optnonlin: $(OBJ)
	$(CXX) -o $@ $^ -ldatrwxx -lsffxx -lgsexx -ltime++ -laff -loptimizexx \
  	-lboost_filesystem -lboost_program_options -lboost_thread -std=c++0x \
		$(LDFLAGS) $(CXXFLAGS) $(FLAGS)

#======================================================================
help: optnonlin
	$< -help 2>&1 | less

#======================================================================

$(call CHECKVARS,TF_WWWBASEDIR TF_BROWSER)

DOXYWWWPATH=$(TF_WWWBASEDIR)/optimize/optnonlin

.PHONY: doxyclean doxyview doxydoc doxyconf

doxyclean: ;/bin/rm -rfv $(DOXYWWWPATH)

DOXYSRC=$(wildcard *.cc *.h README.*)

# create doxygen intermediate configuration
PWD=$(shell env pwd)
doxydoc.xxx: doxydoc.cfg
	sed 's,<OUTPUTDIRECTORY>,$(DOXYWWWPATH),g;s,<STRIPFROMPATH>,$(PWD),g' \
	  $< > $@
# create commented version of doxygen configuration
doxycomm.xxx: doxydoc.cfg
	/bin/cp -vf $< $@; doxygen -u $@

$(DOXYWWWPATH)/html/index.html: doxydoc.xxx $(DOXYSRC)
	mkdir -vp $(DOXYWWWPATH)
	doxygen $<

doxydoc: $(DOXYWWWPATH)/html/index.html

doxyview: $(DOXYWWWPATH)/html/index.html
	$(TF_BROWSER) file:$< &

# ----- END OF Makefile ----- 
