# Command line tool.
PROGS = snzip
bin_PROGRAMS = $(PROGS)
snzip_SOURCES = snzip.c snzip.h snzip-format.c snappy-framed-format.c snappy-java-format.c snappy-in-java-format.c comment-43-format.c crc32.h

if USE_CRC32_GENERIC
snzip_SOURCES += crc32_generic.c
endif
if USE_CRC32_SSE4_2
snzip_SOURCES += crc32_sse4_2.c
endif

EXTRA_DIST = autogen.sh
dist_doc_DATA = ChangeLog COPYING INSTALL NEWS README
