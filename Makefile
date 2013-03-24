# this is <Makefile>
# ----------------------------------------------------------------------------
# $Id$
# Copyright (c) 2012 by Daniel Armbruster 
# ----
# This file is part of calex optimization.
# 
# calex optimization is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# calex optimization is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with calex optimization.  If not, see <http://www.gnu.org/licenses/>.
# ----
# Daniel Armbruster 19/03/2012
# 
# REVISIONS and CHANGES
# 19/03/2012	V0.1	Daniel Armbruster
#
# ----------------------------------------------------------------------------
#

PROGRAMS=optcalex optnonlin

.PHONY: all
all: install doc

.PHONY: install
install: $(addprefix $(LOCALBINDIR)/,$(PROGRAMS))
$(LOCALBINDIR)/%: %
	mkdir -pv $(LOCALBINDIR)
	/bin/mv -fv $< $(LOCALBINDIR)

.PHONY: doc
doc: doxydoc

flist: README.doxydoc Makefile $(wildcard *.cc) doxydoc.cfg
	echo $^ | tr ' ' '\n' | sort > $@

.PHONY: edit
edit: README.doxydoc Makefile $(wildcard *.cc) doxydoc.cfg;
	vim -p $^

.PHONY: clean
clean:
	-/bin/rm *.o *.bak *.o77 *.exe flist *.ps $(PROGRAMS) *.xxx *.d
	-find . -name \*.bak | xargs --no-run-if-empty /bin/rm -v
	-find . -name \*.d | xargs --no-run-if-empty /bin/rm -v
	-find . -name \*.o | xargs --no-run-if-empty /bin/rm -v

# =============================================================================
#
CHECKVAR=$(if $($(1)),,$(error ERROR: missing variable $(1)))
CHECKVARS=$(foreach var,$(1),$(call CHECKVAR,$(var)))
#
$(call CHECKVARS,LOCALINCLUDEDIR LOCALLIBDIR LOCALBINDIR)
$(call CHECKVARS, BOOST_FILESYSTEM_VERSION)
$(call CHECKVARS,BROWSER DOCWWWBASEDIR)

FLAGS= -DBOOST_FILESYSTEM_VERSION=$(BOOST_FILESYSTEM_VERSION)
FLAGS += $(MYFLAGS) -std=c++0x
CFLAGS += -march=native -O2 -fno-reorder-blocks -fno-reorder-functions -pipe

CXXFLAGS += -Wall $(FLAGS)
LDFLAGS+=$(addprefix -L,$(LOCALLIBDIR))
CPPFLAGS+=$(addprefix -I,$(LOCALINCLUDEDIR)) $(FLAGS)

#=============================================================================
# dependencies
# ------------
#
# The compiler is used to create dependency files, which are included below.

%.d: %.cc
	$(SHELL) -ec '$(CXX) -M $(CPPFLAGS) $< \
      | sed '\''s,\($(notdir $*)\)\.o[ :]*,$(dir $@)\1.o $@ : ,g'\'' \
        > $@; \
      [ -s $@ ] || rm -f $@'

SRCFILES=$(wildcard *.cc) $(wildcard optnonlinxx/*.cc)
-include $(patsubst %.cc,%.d,$(SRCFILES))

#------------------------------------------------------------------------------

optcalex: %: %.o $(patsubst %.cc,%.o,$(wildcard optcalexxx/*.cc)) 
	$(CXX) -o $@ $^ -I$(LOCALINCLUDEDIR) -loptimizexx -lcalexxx \
		-lboost_filesystem -lboost_program_options -lboost_thread -std=c++0x \
		-L$(LOCALLIBDIR) $(CXXFLAGS) $(FLAGS) $(LDFLAGS)

optnonlin: %: %.o $(patsubst %.cc,%.o,$(wildcard optnonlinxx/*.cc))
	$(CXX) -o $@ $^ -ldatrwxx -lsffxx -lgsexx -ltime++ -laff -loptimizexx \
  	-lboost_filesystem -lboost_program_options -lboost_thread -std=c++0x \
		$(LDFLAGS) $(CXXFLAGS) $(FLAGS)

# ============================================================================
# documentation
# -------------
#
# make doxyclean        removes all documentation
# make doxydoc          creates doxygen documentation in the DOXYWWWPATH
# make doxyview         creates doxygen documentation and launches netscape to
#                       browse in the documentation
# make doxyconf         edit the doxygen configuration file
#
# If you launch "make doxydoc" the documentation will be written to
# DOXYWWWPATH (see below). This is meant to export the documentation through
# your homepage. The doxyfull directory is just a symbolic link to this
# directory.
#

$(call CHECKVARS,DOCWWWBASEDIR BROWSER)

DOXYWWWPATH=$(DOCWWWBASEDIR)/optimize

.PHONY: doxyclean doxyview doxydoc doxyconf

doxyclean: ;/bin/rm -rfv $(DOXYWWWPATH)

DOXYSRC=Makefile README.doxydoc $(wildcard *.cc *.h)

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
	$(BROWSER) file:$< &

# ----- END OF Makefile -----
