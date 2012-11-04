#ifndef CRC32_H
#define CRC32_H 1
#include <stdint.h>

#define MASK_DELTA 0xa282ead8

#ifdef RUNTIME_CPUDETECTION
void crc32c_init(void);
extern uint32_t (*calculate_crc32c)(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
uint32_t calculate_crc32c_sse4_2(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
uint32_t calculate_crc32c_generic(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
#else
#ifdef USE_CRC32_SSE4_2
void crc32c_init(void);
#define calculate_crc32c calculate_crc32c_sse4_2
uint32_t calculate_crc32c(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
#else
#define crc32c_init() do {} while(0)
#define calculate_crc32c calculate_crc32c_generic
uint32_t calculate_crc32c(uint32_t crc32c, const unsigned char *buffer, unsigned int length);
#endif
#endif

static inline unsigned int masked_crc32c(const char *buf, size_t len)
{
  unsigned int crc = ~calculate_crc32c(~0, (const unsigned char *)buf, len);
  return ((crc >> 15) | (crc << 17)) + MASK_DELTA;
}

#endif /* CRC32_H */
