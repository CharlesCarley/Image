# -----------------------------------------------------------------------------
#   Copyright (c) Charles Carley.
#
#   This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)
project(Image)

if (Image_ExternalTarget)
    set(TargetFolders ${Image_TargetFolders})
    set(TargetName    ${Image_TargetName})
    set(TargetGroup   ${Image_TargetGroup})
else()
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    subdirs(CMake)
    set(TargetName Image)

    set(FreeImage_INCLUDE ${Image_SOURCE_DIR}/Extern/FreeImage/Source)
    set(FreeImage_LIBRARY FreeImage)
    subdirs(Extern/Utils)
    set(Utils_INCLUDE ${Image_SOURCE_DIR}/Extern)
    set(Utils_LIBRARY Utils)


    set(FreeImage_ExternalTarget TRUE)
    set(FreeImage_TargetFolders  TRUE)
    set(FreeImage_TargetName     FreeImage)
    set(FreeImage_TargetGroup    "Extern/FreeImage")
    subdirs(Extern/FreeImage)

    subdirs(Examples)
endif()

add_definitions(-DFREEIMAGE_LIB)
subdirs(Image)
