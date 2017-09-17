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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GPT_Handle *gpt_create_handle(const char *path, unsigned int lba_size,
                                      uint64_t offset) {
  if (lba_size < 92) {
    return NULL;
  }

  struct GPT_Handle *handle = (struct GPT_Handle *)malloc(sizeof(struct GPT_Handle));

  handle->file = fopen(path, "r");
  if (handle->file == NULL) {
    free((void *)handle);
    return NULL;
  }

  handle->lba_size = lba_size;
  handle->offset = offset * lba_size;

  return handle;
}

struct GPT_Handle *gpt_create_handle_with_signature(const char *path,
                    unsigned int lba_size, const char *signature, uint64_t offset) {
  struct GPT_Handle *handle = gpt_create_handle(path, lba_size, offset);
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

  memcpy(header->signature, data.signature, sizeof(data.signature));
  memcpy(header->guid, data.guid, sizeof(data.guid));
  header->revision = data.revision;
  header->header_size = data.header_size;
  header->crc32_header = data.crc32_header;
  header->position_primary = data.position_primary;
  header->position_secondary = data.position_secondary;
  header->first_partition_lba = data.first_partition_lba;
  header->last_partition_lba = data.last_partition_lba;
  header->position_entries = data.position_entries;
  header->entries = data.entries;
  header->entry_size = data.entry_size;
  header->crc32_entries = data.crc32_entries;

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

  memcpy(entry->type_guid, data.type_guid, sizeof(data.type_guid));
  memcpy(entry->guid, data.guid, sizeof(data.guid));
  memcpy(entry->name, data.name, sizeof(data.name));
  entry->first_lba = data.first_lba;
  entry->last_lba = data.last_lba;
  entry->attributes = data.attributes;

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

    memcpy(entries[x].type_guid, data.type_guid, sizeof(entries->type_guid));
    memcpy(entries[x].guid, data.guid, sizeof(entries->guid));
    memcpy(entries[x].name, data.name, sizeof(entries->name));
    entries[x].first_lba = data.first_lba;
    entries[x].last_lba = data.last_lba;
    entries[x].attributes = data.attributes;

    if (padding != 0 && fseek((FILE *)handle->file, padding, SEEK_CUR) != 0) {
      free(entries);
      return NULL;
    }
  }
}
