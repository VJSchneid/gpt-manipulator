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

#include <gpt-manipulator.h>

struct GPT_Header_Raw {
  uint8_t signature[8];
  uint32_t revision;
  uint32_t header_size;
  uint32_t crc32_header;
  uint32_t null;
  uint64_t position_primary;
  uint64_t position_secondary;
  uint64_t first_partition_lba;
  uint64_t last_partition_lba;
  uint8_t guid[16];
  uint64_t position_entries;
  uint32_t entries;
  uint32_t entry_size;
  uint32_t crc32_entries;
} __attribute__((packed));

struct GPT_Entry_Raw {
  uint8_t type_guid[16];
  uint8_t guid[16];
  uint64_t first_lba;
  uint64_t last_lba;
  uint64_t attributes;
  uint16_t name[36];
} __attribute__((packed));

void gpt_copy_raw_header(struct GPT_Header *dest, struct GPT_Header_Raw *src);

void gpt_copy_header(struct GPT_Header_Raw *dest, struct GPT_Header *src);

void gpt_copy_raw_entry(struct GPT_Entry *dest, struct GPT_Entry_Raw *src);

void gpt_copy_entry(struct GPT_Entry_Raw *dest, struct GPT_Entry *src);

bool gpt_write_padding(struct GPT_Handle *handle, int padding);

bool gpt_write_pad(struct GPT_Handle *handle, void *pad, int pad_size,
                        int padding);
