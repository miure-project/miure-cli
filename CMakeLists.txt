# SPDX-FileCopyrightText: 2026 miure-project
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.15)
project(miurecfg VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(miure-cli SHARED src/cli.c)
target_include_directories(miure-cli SYSTEM PUBLIC include)

if(MSVC)
    target_compile_options(miure-cli PRIVATE /W4)
else()
    target_compile_options(miure-cli PRIVATE -Wall -Wextra -O2)
endif()