## Makefile.am -- Process this file with automake to produce Makefile.in
MAINTAINERCLEANFILES    = Makefile.in

INCLUDES                = -I$(top_srcdir)/src

bin_PROGRAMS            = cl_demo

cl_demo_SOURCES         = DeleteFiles.cpp IndexFiles.cpp SearchFiles.cpp Statistics.cpp Main.cpp stdafx.cpp

cl_demo_LDADD           = $(top_builddir)/src/libclucene.la

EXTRA_DIST = Jamfile.v2 stdafx.h
