#
# Makefile gnucobol/bin
#
# Copyright (C) 2001-2012, 2014, 2017-2019 Free Software Foundation, Inc.
# Written by Roger While, Simon Sobisch
#
# This file is part of GnuCOBOL.
#
# The GnuCOBOL compiler is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# GnuCOBOL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GnuCOBOL.  If not, see <https://www.gnu.org/licenses/>.

bin_SCRIPTS = cob-config
bin_PROGRAMS = cobcrun
cobcrun_SOURCES = cobcrun.c
dist_man_MANS = cobcrun.1 cob-config.1
COBCRUN = "$(top_builddir)/pre-inst-env" cobcrun$(EXEEXT)

AM_LDFLAGS = $(COB_EXPORT_DYN)
AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)
cobcrun_LDADD = $(top_builddir)/lib/libsupport.a $(top_builddir)/libcob/libcob.la \
	$(CODE_COVERAGE_LIBS)

# Add rules for code-coverage testing, as provided AX_CODE_COVERAGE
@CODE_COVERAGE_RULES@
CODE_COVERAGE_BRANCH_COVERAGE=1
CODE_COVERAGE_LCOV_OPTIONS =  --no-external

MAINTAINERCLEANFILES = cobcrun.1

cobcrun.1: cobcrun.c $(top_srcdir)/configure.ac |  cobcrun$(EXEEXT)
	$(HELP2MAN) --output=$@ --info-page=$(PACKAGE) "$(COBCRUN) -q"

