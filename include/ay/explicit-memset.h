/*
 * Copyright 2021 arnavyc <arnavyc@outlook.com>
 * SPDX-License-Identifier: 0BSD
 */

#ifndef AY_EXPLICIT_MEMSET_H
#define AY_EXPLICIT_MEMSET_H

/* To use this single-file library, create a file explicit_memset.c with the
 * following content: (or just copy src/ay/explicit-memset.c)
 *
 * #define AY_EXPLICIT_MEMSET_IMPLEMENTATION
 * #include "<location of header>"
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

void *ay_explicit_memset(void *str, int c, size_t n);

#ifdef AY_EXPLICIT_MEMSET_IMPLEMENTATION

#ifndef __STDC_WANT_LIB_EXT1__
/* memset_s() function in C11 Annex K (bound-checking interface) */
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <stdlib.h>
#include <string.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/param.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif /* _WIN32 */

#ifdef __GLIBC__
#if __GLIBC_PREREQ(2, 25)
#define HAVE_EXPLICIT_BZERO 1
#endif
#endif

#if !defined(HAVE_EXPLICIT_BZERO) &&                                           \
        (defined(__FreeBSD__) && __FreeBSD_version >= 1100037) ||              \
    (defined(__OpenBSD__) && OpenBSD >= 201405)
/* explicit_bzero was added in:
 * - glibc 2.25
 * - FreeBSD 11.0 (__FreeBSD_version == 1100037). From:
 *   https://docs.freebsd.org/en/books/porters-handbook/versions/#versions-11
 * - OpenBSD 5.5 (OpenBSD >= 201405)
 */
#define HAVE_EXPLICIT_BZERO 1
#endif

/* explicit_memset was added in:
 * - NetBSD 7.0 (__NetBSD_Version__ >= 702000000).
 */
#if defined(__NetBSD__) && __NetBSD_Version__ >= 702000000
#define HAVE_EXPLICIT_MEMSET 1
#endif

#if defined(__GNUC__) && (defined(__ELF__) || defined(__APPLE_CC__))
#define HAVE_WEAK_LINKING_SUPPORT 1
__attribute__((weak)) void weak_sym_to_avoid_optimization(void *str, size_t n);
#else
static void *(*const volatile memset_ptr)(void *, int, size_t) = memset;
#endif

void *ay_explicit_memset(void *str, int c, size_t n) {
#if defined(HAVE_EXPLICIT_MEMSET)
  return explicit_memset(str, c, n);
#elif defined(__STDC_LIB_EXT1__)
  /* memset_s() function in C11 Annex K (bounds-checking interface) */
  (void)memset_s(str, (rsize_t)n, c, (rsize_t)n);
  return str;
#else

  if (c == 0) {
#if defined(_WIN32)
    return SecureZeroMemory(str, n);
#elif defined(HAVE_EXPLICIT_BZERO)
    explicit_bzero(str, n);
    return str;
#endif
  }

  void *result_of_memset = NULL;
#if defined(HAVE_WEAK_LINKING_SUPPORT)
  /* Use weak linking if available to prevent compiler from optimizing away
   * memset calls. (Approach used by libsodium[1]).
   *
   * [1]:
   * https://github.com/jedisct1/libsodium/blob/master/src/libsodium/sodium/utils.c#L106
   */
  result_of_memset = memset(str, c, n);
  if (weak_sym_to_avoid_optimization) {
    weak_sym_to_avoid_optimization(str, n);
  }
#else
  /* Use a volatile pointer to memset as used by OpenSSL[1] for securely
   * clearing memory.
   *
   * [1]:
   * https://github.com/openssl/openssl/blob/f77208693ec3bda99618e6f76c0f8d279c0077bb/crypto/mem_clr.c
   */
  result_of_memset = (memset_ptr)(str, c, n);
#endif /* defined(HAVE_WEAK_LINKING_SUPPORT) */

#ifdef __GNUC__
  /* Use a ASM memory barrier to force GCC to not optimize memset away. (Used by
   * Glibc) */
  __asm__ __volatile__("" : : "r"(str) : "memory");
#endif /* __GNUC__ */

  return result_of_memset;
#endif
}

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AY_EXPLICIT_MEMSET_H */
