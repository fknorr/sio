# Copyright (c) 2015, Fabian Knorr
#
# This file is part of libsio.
#
# libsio is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libsio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libsio.  If not, see <http://www.gnu.org/licenses/>.


check_PROGRAMS = $(top_builddir)/test

__top_builddir__test_SOURCES = \
    main.cc \
    writer.cc

__top_builddir__test_LDADD = \
    $(top_builddir)/libsio.la \
    $(BOOST_LDFLAGS) \
    $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

__top_builddir__test_CPPFLAGS = \
    -I$(top_srcdir)/include\
    $(BOOST_CFLAGS)
