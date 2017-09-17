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

#include "gpt-manipulator.h"
#include "crc32.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void gpt_copy_raw_header(struct GPT_Header *dest, struct GPT_Header_Raw *src) {
  memcpy(dest->signature, src->signature, sizeof(src->signature));
  memcpy(dest->guid, src->guid, sizeof(src->guid));
  dest->revision = src->revision;
  dest->header_size = src->header_size;
  dest->crc32_header = src->crc32_header;
  dest->position_primary = src->position_primary;
  dest->position_secondary = src->position_secondary;
  dest->first_partition_lba = src->first_partition_lba;
  dest->last_partition_lba = src->last_partition_lba;
  dest->position_entries = src->position_entries;
  dest->entries = src->entries;
  dest->entry_size = src->entry_size;
  dest->crc32_entries = src->crc32_entries;
}

void gpt_copy_header(struct GPT_Header_Raw *dest, struct GPT_Header *src) {
  memcpy(dest->signature, src->signature, sizeof(dest->signature));
  memcpy(dest->guid, src->guid, sizeof(dest->guid));
  dest->revision = src->revision;
  dest->header_size = src->header_size;
  dest->crc32_header = src->crc32_header;
  dest->position_primary = src->position_primary;
  dest->position_secondary = src->position_secondary;
  dest->first_partition_lba = src->first_partition_lba;
  dest->last_partition_lba = src->last_partition_lba;
  dest->position_entries = src->position_entries;
  dest->entries = src->entries;
  dest->entry_size = src->entry_size;
  dest->crc32_entries = src->crc32_entries;
}

void gpt_copy_raw_entry(struct GPT_Entry *dest, struct GPT_Entry_Raw *src) {
  memcpy(dest->type_guid, src->type_guid, sizeof(src->type_guid));
  memcpy(dest->guid, src->guid, sizeof(src->guid));
  memcpy(dest->name, src->name, sizeof(src->name));
  dest->first_lba = src->first_lba;
  dest->last_lba = src->last_lba;
  dest->attributes = src->attributes;
}

void gpt_copy_entry(struct GPT_Entry_Raw *dest, struct GPT_Entry *src) {
  memcpy(dest->type_guid, src->type_guid, sizeof(dest->type_guid));
  memcpy(dest->guid, src->guid, sizeof(dest->guid));
  memcpy(dest->name, src->name, sizeof(dest->name));
  dest->first_lba = src->first_lba;
  dest->last_lba = src->last_lba;
  dest->attributes = src->attributes;
}

bool gpt_write_padding(struct GPT_Handle *handle, int padding) {
  uint8_t *buffer;
  int buffer_size;
  if (padding > 256) {
    buffer = (uint8_t *)malloc(256);
    buffer_size = 256;
  } else {
    buffer = (uint8_t *)malloc(padding);
    buffer_size = padding;
  }
  memset(buffer, 0, buffer_size);

  bool rValue = gpt_write_pad(handle, buffer, buffer_size, padding);

  free(buffer);
  return rValue;
}

bool gpt_write_pad(struct GPT_Handle *handle, void *pad, int pad_size,
                        int padding) {
  for (; padding > 0; padding -= pad_size) {
    if (fwrite(pad, pad_size, 1, (FILE *)handle->file) != 1) {
      return false;
    }
  }

  if ((padding += pad_size)) {
    if (fwrite(pad, padding, 1, (FILE *)handle->file) != 1) {
      return false;
    }
  }
  return true;
}

struct GPT_Handle *gpt_create_handle(const char *path, unsigned int lba_size,
                                      uint64_t offset, bool read_only) {
  if (lba_size < 92) {
    return NULL;
  }

  struct GPT_Handle *handle = (struct GPT_Handle *)malloc(sizeof(struct GPT_Handle));

  if (read_only) {
    handle->file = fopen(path, "r");
  } else {
    handle->file = fopen(path, "rw");
  }
  if (handle->file == NULL) {
    free((void *)handle);
    return NULL;
  }

  handle->lba_size = lba_size;
  handle->offset = offset * lba_size;

  return handle;
}

struct GPT_Handle *gpt_create_handle_with_signature(const char *path,
                    unsigned int lba_size, const char *signature, uint64_t offset,
                    bool read_only) {
  struct GPT_Handle *handle = gpt_create_handle(path, lba_size, offset, read_only);
  if (handle == NULL) {
    return NULL;
  }

  if (fseek((FILE *)handle->file, handle->offset, SEEK_SET) != 0) {
    gpt_close_handle(handle);
    return NULL;
  }

  uint64_t file_signature[8];
  if (fread(file_signature, sizeof(file_signature), 1,
                    (FILE *)handle->file) != 1) {
    gpt_close_handle(handle);
    return NULL;
  }

  if (memcmp(file_signature, signature, 8) != 0) {
    gpt_close_handle(handle);
    return NULL;
  }

  return handle;
}

void gpt_close_handle(struct GPT_Handle *handle) {
  fclose((FILE *)handle->file);
  free(handle);
}

struct GPT_Header *gpt_read_header(struct GPT_Handle *handle) {
  if (fseek((FILE *)handle->file, handle->offset, SEEK_SET) != 0) {
    return NULL;
  }

  struct GPT_Header_Raw data;
  if (fread(&data, sizeof(struct GPT_Header_Raw), 1, (FILE *)handle->file) != 1) {
    return NULL;
  }

  struct GPT_Header *header = (struct GPT_Header *)malloc(sizeof(struct GPT_Header));
  gpt_copy_raw_header(header, &data);

  return header;
}

void gpt_free_header(struct GPT_Header *header) {
  free(header);
}

struct GPT_Entry *gpt_get_entry(struct GPT_Handle *handle,
                  struct GPT_Header *header, int partition_no) {
  if (fseek((FILE *)handle->file, handle->offset + handle->lba_size +
              partition_no * header->entry_size, SEEK_SET) != 0) {
    return NULL;
  }

  struct GPT_Entry_Raw data;
  int readLength;
  if (header->entry_size < sizeof(struct GPT_Entry_Raw)) {
    memset(&data, 0, sizeof(struct GPT_Entry_Raw));
    readLength = header->entry_size;
  } else {
    readLength = sizeof(struct GPT_Entry_Raw);
  }

  if (fread(&data, readLength, 1, (FILE *)handle->file) != 1) {
    return NULL;
  }

  struct GPT_Entry *entry = (struct GPT_Entry *)malloc(sizeof(struct GPT_Entry));
  gpt_copy_raw_entry(entry, &data);

  return entry;
}

// TODO verify header
struct GPT_Entry *gpt_get_all_entries(struct GPT_Handle *handle,
                        struct GPT_Header *header) {
  if (fseek((FILE *)handle->file, header->position_entries *
              handle->lba_size, SEEK_SET) != 0) {
    return NULL;
  }

  struct GPT_Entry *entries = (struct GPT_Entry *)malloc(
                        sizeof(struct GPT_Entry) * header->entries);
  if (entries == NULL) {
    return NULL;
  }

  struct GPT_Entry_Raw data;
  int readLength, padding = header->entry_size - sizeof(struct GPT_Entry_Raw);
  if (padding < 0) {
    memset(&data, 0, sizeof(struct GPT_Entry_Raw));
    readLength = header->entry_size;
    padding = 0;
  } else {
    readLength = sizeof(struct GPT_Entry_Raw);
  }

  for (int x = 0; x > header->entries; x++) {
    if (fread(&data, readLength, 1, (FILE *)handle->file) != 1) {
      free(entries);
      return NULL;
    }

    gpt_copy_raw_entry(entries + x, &data);

    if (padding != 0 && fseek((FILE *)handle->file, padding, SEEK_CUR) != 0) {
      free(entries);
      return NULL;
    }
  }
}

void gpt_free_entries(struct GPT_Entry *entries) {
  free(entries);
}

void gpt_refresh_crc32(struct GPT_Header *header) {
  header->crc32_header = 0;

  void *data;
  if (header->header_size <= sizeof(struct GPT_Header_Raw)) {
    data = malloc(sizeof(struct GPT_Header_Raw));
  } else {
    data = malloc(header->header_size);
    if (data == NULL) {
      return;
    }
    memset(data, 0, header->header_size);
  }

  gpt_copy_header((struct GPT_Header_Raw *)data, header);

  header->crc32_header = crc32_generate(data, header->header_size);
}

void gpt_refresh_entries(struct GPT_Header *header, struct GPT_Entry *entries,
                        int partition_count) {
  header->crc32_entries = crc32_generate(entries,
                                  header->entries * header->entry_size);
}

enum GPT_Error gpt_write_header(struct GPT_Handle *handle,
                                    struct GPT_Header *header) {
  if (fseek((FILE *)handle->file, handle->offset, SEEK_SET) != 0) {
    return GPT_SEEK_ERROR;
  }

  struct GPT_Header_Raw data;
  gpt_copy_header(&data, header);

  if (fwrite(&data, sizeof(struct GPT_Header_Raw), 1, (FILE *)handle->file) != 1) {
    return GPT_WRITE_ERROR;
  }

  int padding = header->header_size - sizeof(struct GPT_Header_Raw);
  if (padding > 0 && !gpt_write_padding(handle, padding)) {
    return GPT_WRITE_ERROR;
  }

  if (fflush((FILE *)handle->file) != 0) {
    return GPT_WRITE_ERROR;
  }

  return GPT_SUCCESS;
}

enum GPT_Error gpt_write_entries(struct GPT_Handle *handle,
                                    struct GPT_Header *header,
                                    struct GPT_Entry *entries) {
  if (fseek((FILE *)handle->file, handle->offset + handle->lba_size, SEEK_SET) != 0) {
    return GPT_SEEK_ERROR;
  }

  struct GPT_Entry_Raw data;
  int writeLength, padding = header->entry_size - sizeof(struct GPT_Entry_Raw);
  if (padding < 0) {
    writeLength = header->entry_size;
    padding = 0;
  } else {
    writeLength = sizeof(struct GPT_Entry_Raw);
  }

  void *pad;
  int pad_size;
  if (padding != 0) {
    if (padding > 256) {
      pad = malloc(256);
      pad_size = 256;
    } else {
      pad = malloc(padding);
      pad_size = padding;
    }
  }
  memset(pad, 0, pad_size);

  for (int x = 0; x < header->entries; x++) {
    if (fwrite(entries + x, writeLength, 1, (FILE*)handle->file) != 1) {
      free(pad);
      return GPT_WRITE_ERROR;
    }
    if (padding != 0 && !gpt_write_pad(handle, pad, pad_size, padding)) {
      free(pad);
      return GPT_WRITE_ERROR;
    }
  }
  free(pad);

  if (fflush((FILE *)handle->file) != 0) {
    return GPT_WRITE_ERROR;
  }

  return GPT_SUCCESS;
}
