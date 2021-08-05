# Copyright 2021 arnavyc <arnavyc@outlook.com>
#
# SPDX-License-Identifier: 0BSD

include(DownloadAndUseMunit)
include(Munit)

if (UNIX)
  add_executable(explicit-memset_tests explicit-memset.test.c)
  target_link_libraries(explicit-memset_tests explicit-memset munit)

  munit_discover_tests(explicit-memset_tests)
endif ()
