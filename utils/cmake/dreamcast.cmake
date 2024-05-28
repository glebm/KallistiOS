# Auxiliary CMake Utility Functions
#   Copyright (C) 2023 Colton Pawielski
#   Copyright (C) 2024 Falco Girgis
#
# This file implements utilities for the following additional functionality
# which exists in the KOS Make build system:
#   1) linking to existing binaries
#   2) adding a romdisk
#
# NOTE: When using the KOS CMake toolchain file, you do not need to include
#       this file directly!

### This minimum is based on the minimum requirement in dreamcast.toolchain.cmake
cmake_minimum_required(VERSION 3.13)

#### Set Configuration Variables From Environment (if Necessary) ####
if(NOT DEFINED KOS_BASE)
    if(NOT DEFINED ENV{KOS_BASE})
        message(FATAL_ERROR "KOS_BASE environment variable not found!")
    else()
        set(KOS_BASE $ENV{KOS_BASE})
    endif()
endif()

if(NOT DEFINED KOS_CC_BASE)
    if(NOT DEFINED ENV{KOS_CC_BASE})
        message(FATAL_ERROR "KOS_CC_BASE environment variable not found!")
    else()
        set(KOS_CC_BASE $ENV{KOS_CC_BASE})
    endif()
endif()

### Helper Function for Bin2Object ###
function(kos_bin2o inFile symbol)
    # outFile is optional and defaults to the symbol name in the build directory
    if(NOT ${ARGC} EQUAL 3)
        set(outFile ${CMAKE_CURRENT_BINARY_DIR}/${symbol}.o)
    else()
        set(outFile ${ARGN})
    endif()

    # Custom Command to generate romdisk object file from image
    add_custom_command(
        OUTPUT  ${outFile}
        DEPENDS ${inFile}
        COMMAND ${KOS_BASE}/utils/bin2o/bin2o ${inFile} ${symbol} ${outFile}
    )
endfunction()

function(kos_add_binary target inFile symbol)
    file(REAL_PATH "${inFile}" inFile)
    set(outFile ${CMAKE_CURRENT_BINARY_DIR}/${symbol}.o)
    kos_bin2o(${inFile} ${symbol} ${outFile})
    target_sources(${target} PRIVATE ${outFile})
endfunction()

### Helper Function for Generating Romdisk ###
function(kos_add_romdisk target romdiskPath)
    # Name is optional and defaults to "romdisk"
    if(NOT ${ARGC} EQUAL 3)
        set(romdiskName romdisk)
    else()
        set(romdiskName ${ARGN})
    endif()

    file(REAL_PATH "${romdiskPath}" romdiskPath)

    set(obj     ${CMAKE_CURRENT_BINARY_DIR}/${romdiskName}.o)
    set(obj_tmp ${CMAKE_CURRENT_BINARY_DIR}/${romdiskName}_tmp.o)
    set(img     ${CMAKE_CURRENT_BINARY_DIR}/${romdiskName}.img)

    # Variable holding all files in the romdiskPath folder
    # CONFIGURE_DEPENDS causes the folder to always be rechecked
    # at build time not only when CMake is re-run
    file(GLOB romdiskFiles CONFIGURE_DEPENDS
        "${romdiskPath}/*"
    )
    
    # Custom Command to generate romdisk image from folder
    # Only run when folder contents have changed by depending on 
    # the romdiskFiles variable
    add_custom_command(
        OUTPUT  ${img}
        DEPENDS ${romdiskFiles}
        COMMAND ${KOS_BASE}/utils/genromfs/genromfs -f ${img} -d ${romdiskPath} -v
    )

    kos_bin2o(${img} ${romdiskName} ${obj_tmp})

    # Custom Command to generate romdisk object file from image
    add_custom_command(
        OUTPUT  ${obj}
        DEPENDS ${obj_tmp}
        COMMAND ${KOS_CC_BASE}/bin/sh-elf-gcc -o ${obj} -r ${obj_tmp} -L${KOS_BASE}/lib/dreamcast -Wl,--whole-archive -lromdiskbase
        COMMAND rm ${obj_tmp}
    )

    # Append romdisk object to target
    target_sources(${target} PRIVATE ${obj})
endfunction()
