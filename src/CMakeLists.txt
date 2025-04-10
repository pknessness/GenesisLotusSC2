# The MIT License (MIT)
#
# Copyright (c) 2021-2024 Alexander Kurbatov

set(bot_sources
    main.cpp)

find_library(BFD_LIBRARY NAMES libbfd.a PATHS /usr/lib /usr/lib/x86_64-linux-gnu lib)
find_library(IBERTY_LIBRARY NAMES libiberty.a PATHS /usr/lib /usr/lib/x86_64-linux-gnu lib)
find_library(SFRAME_LIBRARY NAMES libsframe.a PATHS /usr/lib /usr/lib/x86_64-linux-gnu lib)
find_library(ZSTD_LIBRARY NAMES libzstd.a PATHS /usr/lib /usr/lib/x86_64-linux-gnu lib)
find_library(ZLIB_LIBRARY NAMES libz.a PATHS /usr/lib /usr/lib/x86_64-linux-gnu lib)

add_executable(BlankBot ${bot_sources})

if (BUILD_FOR_LADDER)
    target_compile_definitions(BlankBot PRIVATE BUILD_FOR_LADDER)
endif ()

if (MSVC)
    target_compile_options(BlankBot PRIVATE /W4 /EHsc)
else ()
    target_compile_options(BlankBot PRIVATE -Wall -Wextra -pedantic -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(BlankBot PRIVATE cpp_sc2)

if (MINGW)
    target_link_libraries(BlankBot PRIVATE ssp)
elseif (APPLE)
    target_link_libraries(BlankBot PRIVATE "-framework Carbon")
# Building on Linux
elseif (UNIX AND NOT APPLE)
    target_link_libraries(BlankBot PRIVATE pthread ${BFD_LIBRARY} ${IBERTY_LIBRARY} dl ${ZSTD_LIBRARY} ${ZLIB_LIBRARY} ${SFRAME_LIBRARY})
endif ()