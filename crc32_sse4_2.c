/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright 2012 Kubo Takehiro <kubo@jiubao.org>
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of the authors.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <nmmintrin.h>
#include "crc32.h"


#define CPUID_ECX_BIT_SSE4_2 (1u << 20)

/* GNU C Compiler */
#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#include <cpuid.h>
/* gcc version >= 4.3 */
static int sse4_2_is_supported(void)
{
  unsigned int eax, ebx, ecx, edx;
  __cpuid(1, eax, ebx, ecx, edx);
  return (ecx & CPUID_ECX_BIT_SSE4_2) ? 1 : 0;
}
#else
/* gcc version < 4.3 */
static int sse4_2_is_supported(void)
{
  unsigned int ecx;
#if defined(__i386__) && defined(__PIC__)
  __asm("movl $1, %%eax;"
        "pushl %%ebx;"
        "cpuid;"
        "popl %%ebx;"
        : "=c" (ecx)
        :
        : "eax", "edx");
#else
  __asm("movl $1, %%eax;"
        "cpuid;"
        : "=c" (ecx)
        :
        : "eax", "ebx", "edx");
#endif
  printf("%08x\n", ecx);
  return (ecx & CPUID_ECX_BIT_SSE4_2) ? 1 : 0;
}
#endif
#endif /*  __GNUC__ */

/* Microsoft Visual C++ */
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#include <intrin.h>
/* msvc version >= 2005 */
static int sse4_2_is_supported(void)
{
  int cpuinfo[4];
  __cpuid(cpuinfo, 1);
  return (cpuinfo[2] & CPUID_ECX_BIT_SSE4_2) ? 1 : 0;
}
#else
/* msvc version < 2005 */
static int sse4_2_is_supported(void)
{
  unsigned int c;
  __asm {
    mov eax, 1
    cpuid
    mov c, ecx
  }
  return (c & CPUID_ECX_BIT_SSE4_2) ? 1 : 0;
}
#endif
#endif /* _MSC_VER */

#ifdef RUNTIME_CPUDETECTION
uint32_t (*calculate_crc32c)(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
#endif

void crc32c_init(void)
{
  static int initialized = 0;
  if (initialized) {
    return;
  }
  initialized = 1;

  if (sse4_2_is_supported()) {
#ifdef RUNTIME_CPUDETECTION
    calculate_crc32c = calculate_crc32c_sse4_2;
#else
    ;
#endif
  } else {
#ifdef RUNTIME_CPUDETECTION
    calculate_crc32c = calculate_crc32c_generic;
#else
    fprintf(stderr, "Fatal error: SSE4.2 is not supported on the CPU(s).\n");
    exit(1);
#endif
  }
}

uint32_t calculate_crc32c_sse4_2(uint32_t crc32c, const unsigned char *buffer, unsigned int length)
{
  unsigned int quotient;

  if ((((size_t)buffer) & 1) && length >= 1) {
    crc32c = _mm_crc32_u8(crc32c, *(uint8_t*)buffer);
    buffer += 1;
    length -= 1;
  }
  if ((((size_t)buffer) & 2) && length >= 2) {
      crc32c = _mm_crc32_u16(crc32c, *(uint16_t*)buffer);
      buffer += 2;
      length -= 2;
    }
#if SIZEOF_SIZE_T > 4
  if ((((size_t)buffer) & 4) && length >= 4) {
    crc32c = _mm_crc32_u32(crc32c, *(uint32_t*)buffer);
    buffer += 4;
    length -= 4;
  }
#endif

  quotient = length / sizeof(size_t);
  while (quotient--) {
#if SIZEOF_SIZE_T == 8
    crc32c = _mm_crc32_u64(crc32c, *(size_t*)buffer);
#elif SIZEOF_SIZE_T == 4
    crc32c = _mm_crc32_u32(crc32c, *(size_t*)buffer);
#else
#error sizeof(size_t) is neither 4 nor 8.
#endif
    buffer += sizeof(size_t);
  }

#if SIZEOF_SIZE_T > 4
  if (length & 4) {
    crc32c = _mm_crc32_u32(crc32c, *(uint32_t*)buffer);
    buffer += 4;
  }
#endif
  if (length & 2) {
    crc32c = _mm_crc32_u16(crc32c, *(uint16_t*)buffer);
    buffer += 2;
  }
  if (length & 1) {
    crc32c = _mm_crc32_u8(crc32c, *(uint8_t*)buffer);
  }
  return crc32c;
}
