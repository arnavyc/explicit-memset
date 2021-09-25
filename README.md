<!--
SPDX-License-Identifier: 0BSD
-->

# explicit-memset

[![GitHub](https://img.shields.io/github/license/arnavyc/explicit-memset?logo=github&style=flat-square)](LICENSE.md)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/arnavyc/explicit-memset/Build%20and%20test?logo=github&style=flat-square)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/github/arnavyc/explicit-memset?logo=lgtm&style=flat-square)](https://lgtm.com/projects/g/arnavyc/explicit-memset/context:cpp)
[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

`ay_explicit_memset` function, a version of `memset` that can escape dead code
elimination in C compilers. Licensed under 0-clause BSD License (public
domain-like license).

Useful for clearing out memory after a region of memory has been used by the
program.

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Install

To use `ay_explicit_memset()` function:

- Include this project using CMake and link your targets with CMake target
  `explicit-memset::explicit-memset`.

- Or, copy [`include/ay/explicit-memset.h`](include/ay/explicit-memset.h) into
  your project and create a C file with following content (or just copy
  [`src/ay/explicit-memset.c`](src/ay/explicit-memset.c) and change `#include`
  path inside it with path of `explicit-memset.h`):

```c
#define AY_EXPLICIT_MEMSET_IMPLEMENTATION
#include "Path to explicit-memset.h file"
```

## Usage

Refer to [`include/ay/explicit-memset.h`](include/ay/explicit-memset.h) for
usage.

## Contributing

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, shall be licensed as below, without any
additional terms or conditions.

## License

&copy; 2021 arnavyc \<arnavyc@outlook.com\>

Licensed under the BSD Zero Clause License. See [LICENSE file](LICENSE.md) in
the project root, or https://opensource.org/licenses/0BSD for full license
information.

```
SPDX-License-Identifier: 0BSD
```
