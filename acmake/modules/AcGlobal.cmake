# __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
#
# Copyright (C) 1993-2009, ART+COM AG Berlin, Germany <www.artcom.de>
#
# This file is part of the ART+COM CMake Library (acmake).
#
# It is distributed under the Boost Software License, Version 1.0. 
# (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)             
# __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
#
#
# Global definition of AcMake internals.
#
# __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
#

# Subdirectory for files generated by acmake, relative to CMAKE_CURRENT_BINARY_DIR
set(ACMAKE_BINARY_SUBDIR "ACMakeFiles")

set(ACMAKE_GLOBAL_LIBRARY_DIRS
)

set(ACMAKE_GLOBAL_INCLUDE_DIRS
    "${ACMAKE_INCLUDE_DIR}"       # acmake distribution
    "${CMAKE_BINARY_DIR}/include" # builddir-global generated headers
)
