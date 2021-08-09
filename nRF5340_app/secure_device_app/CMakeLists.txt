# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(secure_device_app)

if(CONFIG_MCUMGR)
    set(DFU_SRC "zephyr_custom/subsys/mgmt/mcumgr/smp_bt.c")
endif()

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
    ${app_sources}
    ${DFU_SRC}
)
target_include_directories(app PRIVATE 
    include
)
