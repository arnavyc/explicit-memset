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

#include <ay/explicit-memset.h>

/*
 * The memmem() function finds the start of the first occurrence of the
 * substring 'needle' of length 'nlen' in the memory area 'haystack' of
 * length 'hlen'.
 *
 * The return value is a pointer to the beginning of the sub-string, or
 * NULL if the substring is not found.
 */
void *memmem(const void *haystack, size_t hlen, const void *needle,
             size_t nlen) {
  int needle_first;
  const unsigned char *p = (const unsigned char *)haystack;
  size_t plen = hlen;

  if (!nlen)
    return NULL;

  needle_first = *(unsigned char *)needle;

  while (plen >= nlen && (p = (unsigned char *)memchr(p, needle_first, plen - nlen + 1))) {
    if (!memcmp(p, needle, nlen))
      return (void *)p;

    p++;
    plen = hlen - (p - haystack);
  }

  return NULL;
}

void printStack( const char* what )
{
	ULONG_PTR lo, high;
	GetCurrentThreadStackLimits( &lo, &high );
	printf( "%s: %p - %p\n", what, (void *)lo, (void *)high);
}

void* fiberMain, *fiberSecondary;

#define cbBuffer 24

void __stdcall test_without_explicit_memset(void* lpFiberParameter) {
  unsigned char localBuffer[cbBuffer];
  memcpy(localBuffer, (unsigned char *)lpFiberParameter, cbBuffer);

  /*ay_explicit_memset(localBuffer, 0, cbBuffer);*/

	printStack("fiber");
  ULONG_PTR low, high;
  GetCurrentThreadStackLimits(&low, &high);
  assert(low < high);

  void *found_ptr = memmem(low, high - low, localBuffer, sizeof localBuffer);
  assert(found_ptr != 0);

	SwitchToFiber(fiberMain);
}

void __stdcall test_with_explicit_memset(void* lpFiberParameter) {
  unsigned char localBuffer[cbBuffer];
  memcpy(localBuffer, (unsigned char *)lpFiberParameter, cbBuffer);

  ay_explicit_memset(localBuffer, 0, cbBuffer);

	printStack("fiber");
  ULONG_PTR low, high;
  GetCurrentThreadStackLimits(&low, &high);
  assert(low < high);

  void *found_ptr = memmem(low, high - low, localBuffer, sizeof localBuffer);
  assert(found_ptr == 0);

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

int main() {
	printStack("start");

	unsigned char localBuffer[cbBuffer] = { 0x4e, 0x65, 0x76, 0x65, 0x72, 0x20, 0x67, 0x6f, 0x6e, 0x6e, 0x61, 0x20, 0x67, 0x69, 0x76, 0x65, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x75, 0x70, 0x2c };

	fiberMain = ConvertThreadToFiber(NULL);
	fiberSecondary = CreateFiber( 0, &test_without_explicit_memset, localBuffer);
	void *fiber_with_explicit_memset = CreateFiber( 0, &test_with_explicit_memset, localBuffer);

	printStack( "converted" );

	SwitchToFiber( fiberSecondary );

	printStack( "back to main" );
	return 0;
}
