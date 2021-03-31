# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(secure_device_app)

FILE(GLOB app_sources src/*.c)
FILE(GLOB ble_sources bluetooth/*.c)
target_sources(app PRIVATE
    ${app_sources}
    ${ble_sources}
    zephyr_custom/subsys/mgmt/mcumgr/smp_bt.c
)
