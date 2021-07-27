#ifdef NDEBUG
#undef NDEBUG
#endif

#include <string.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <munit.h>
#include <ay/explicit-memset.h>

/*
 * The memmem() function finds the start of the first occurrence of the
 * substring 'needle' of length 'nlen' in the memory area 'haystack' of
 * length 'hlen'.
 *
 * The return value is a pointer to the beginning of the sub-string, or
 * NULL if the substring is not found.
 */
void *memmem(const void *haystack, size_t haystack_len, 
    const void * const needle, const size_t needle_len)
{
    if (haystack == NULL) return NULL; // or assert(haystack != NULL);
    if (haystack_len == 0) return NULL;
    if (needle == NULL) return NULL; // or assert(needle != NULL);
    if (needle_len == 0) return NULL;

    for (const char *h = haystack;
            haystack_len >= needle_len;
            ++h, --haystack_len) {
        if (!memcmp(h, needle, needle_len)) {
            return (void *)h;
        }
    }
    return NULL;
}

void printStack( const char* what )
{
	ULONG_PTR lo, high;
	GetCurrentThreadStackLimits( &lo, &high );
	printf( "%s: %p - %p\n", what, (void *)lo, (void *)high);
}

#define cbBuffer 24
static const unsigned char localBuffer[cbBuffer] = { 0x4e, 0x65, 0x76, 0x65, 0x72, 0x20, 0x67, 0x6f, 0x6e, 0x6e, 0x61, 0x20, 0x67, 0x69, 0x76, 0x65, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x75, 0x70, 0x2c };

struct found_secrets {
  int without_bzero;
  int with_bzero;
};

static struct found_secrets secrets_found = {0};

void* fiberMain;

void __stdcall test_without_explicit_memset(void* lpFiberParameter) {
  unsigned char localBuffer[cbBuffer];
  memcpy(localBuffer, (unsigned char *)lpFiberParameter, cbBuffer);

  /*ay_explicit_memset(localBuffer, 0, cbBuffer);*/

	printStack("fiber");
  ULONG_PTR low, high;
  GetCurrentThreadStackLimits(&low, &high);
  munit_assert_ptr(low, <, high);

  void *found_ptr = memmem(low, high - low, localBuffer, sizeof localBuffer);
  secrets_found.without_bzero == !!found_ptr;

	SwitchToFiber(fiberMain);
}

void __stdcall test_with_explicit_memset(void* lpFiberParameter) {
  unsigned char localBuffer[cbBuffer];
  memcpy(localBuffer, (unsigned char *)lpFiberParameter, cbBuffer);

  ay_explicit_memset(localBuffer, 0, cbBuffer);

	printStack("fiber");
  ULONG_PTR low, high;
  GetCurrentThreadStackLimits(&low, &high);
  munit_assert_ptr(low, <, high);

  void *found_ptr = memmem(low, high - low, localBuffer, sizeof localBuffer);
  secrets_found.without_bzero == !found_ptr;

	SwitchToFiber(fiberMain);
}

/*
void __stdcall fiberProc(void* lpFiberParameter) {
  unsigned char localBuffer[cbBuffer];
  memcpy(localBuffer, (unsigned char *)lpFiberParameter, cbBuffer);

#ifdef SECURE_ZERO
  SecureZeroMemory(localBuffer, cbBuffer);
#else
  memset(localBuffer, 0, cbBuffer);
#endif

	printStack("fiber");
	SwitchToFiber(fiberMain);
}*/

static MunitResult without_bzero_test(const MunitParameter params[],
                                   void *user_data_or_fixture) {
  void *fiber_without_explicit_memset = CreateFiber(0, &test_without_explicit_memset, localBuffer);

  SwitchToFiber(fiber_without_explicit_memset);

  munit_assert_int(secrets_found.without_bzero, ==, 1);
  return MUNIT_OK;
}

static MunitResult with_bzero_test(const MunitParameter params[],
                                   void *user_data_or_fixture) {
  void *fiber_with_explicit_memset = CreateFiber(0, &test_with_explicit_memset, localBuffer);

  SwitchToFiber(fiber_with_explicit_memset);

  munit_assert_int(secrets_found.with_bzero, ==, 0);
  return MUNIT_OK;
}

static MunitTest tests[] = {
    {(char *)"/without-bzero", without_bzero_test, NULL, NULL, MUNIT_TEST_OPTION_NONE,
     NULL},
    {(char *)"/with-bzero", with_bzero_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

static const MunitSuite suite = {
    (char *)"/explicit-bzero", /* name */
    tests,                     /* tests */
    NULL,                      /* suites */
    1,                         /* iterations */
    MUNIT_SUITE_OPTION_NONE,   /* options */
};

int main(int argc, char *const argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  fiberMain = ConvertThreadToFiber(NULL);
	return munit_suite_main(&suite, NULL, argc, argv);
}
