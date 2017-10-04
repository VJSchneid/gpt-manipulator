/**
 * Copyright (c) 2017 Viktor Schneider <info@vjs.io>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "crc32.h"

 uint32_t crc32_for_byte(uint32_t r) {
   for(int j = 0; j < 8; ++j)
     r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
   return r ^ (uint32_t)0xFF000000L;
 }

 void crc32(const void *data, unsigned long n_bytes, uint32_t* crc) {
   static uint32_t table[0x100];
   if(!*table)
     for(unsigned long i = 0; i < 0x100; ++i)
       table[i] = crc32_for_byte(i);
   for(unsigned long i = 0; i < n_bytes; ++i)
     *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
 }
