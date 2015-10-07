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


AC_INIT(sio, 0.1)
AM_INIT_AUTOMAKE([1.11 subdir-objects])
LT_INIT([])
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_14([], [mandatory])

AC_DEFINE_UNQUOTED([PREFIX], ["$prefix"], [Installation Prefix])
AC_DEFINE_UNQUOTED([SOURCE_DIR], ["$srcdir"], [Absolute Source Directory])

AC_SUBST(GLADEUI_CATDIR, "$ac_gladeui_catdir")
AC_SUBST(GLADEUI_PIXMAPDIR, "$ac_gladeui_pixmapdir")

pkgconfig_dir="`$PKG_CONFIG --debug 2>&1 | sed -ne "/Scanning directory /{s///;s/'//gp;q;}"`"
AC_SUBST(PKG_CONFIG_DIR, "$pkgconfig_dir")


AC_OUTPUT([
Makefile
src/Makefile
src/libsio/Makefile
src/example/Makefile
])
