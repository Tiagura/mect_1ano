# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr)

zephyr_include_directories(lib)

target_sources(app PRIVATE src/tasks.c)

target_include_directories(app PRIVATE src/lib)
target_sources(app PRIVATE src/lib/cab.c)
target_sources(app PRIVATE src/lib/imageProcAlg.c)
