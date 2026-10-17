/* Compatability shim for zlib/uncompress which re-exports the mz_uncompress
   symbol from miniz. */

extern "C"
{
  // Forward declaration, in miniz.h (via third-party/zip)
  int mz_uncompress(unsigned char *pDest,
                    unsigned long *pDest_len,
                    unsigned char const *pSource,
                    unsigned long source_len);

  // Standard zlib.h API
  int uncompress(unsigned char *pDest,
                 unsigned long *pDest_len,
                 unsigned char const *pSource,
                 unsigned long source_len)
  {
    return mz_uncompress(pDest, pDest_len, pSource, source_len);
  }
}
