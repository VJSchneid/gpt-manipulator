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

#include <stdint.h>
#include <stdbool.h>

#define GPT_DEFAULT_SIGNATURE "EFI_PART"
#define GPT_DEFAULT_OFFSET 512

struct GPT_Handle {
  void *file;
  uint64_t offset;
  unsigned int lba_size;
};

struct GPT_Header {
  uint8_t signature[8];
  uint32_t revision;
  uint32_t header_size;
  uint32_t crc32_header;
  uint64_t position_primary;
  uint64_t position_secondary;
  uint64_t first_partition_lba;
  uint64_t last_partition_lba;
  uint8_t guid[8];
  uint64_t position_entries;
  uint32_t entries;
  uint32_t entry_size;
  uint32_t crc32_entries;
};

struct GPT_Entry {
    uint8_t type_guid[16];
    uint8_t guid[16];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t attributes;
    uint8_t name[72];
};

enum GPT_Error {
  GPT_SUCCESS,
  GPT_CRC32_INCORRECT,
  GPT_SEEK_ERROR,
  GPT_WRITE_ERROR
};

/**
 * Create a GPT Handle
 * @param  path     Path to device or image with GPT table
 * @param  lba_size Size of one LBA Sector
 * @param  offset   Offset (LBA) for GPT table
 * @return          returns NULL on error
 */
struct GPT_Handle *gpt_create_handle(const char *path, unsigned int lba_size,
                                      uint64_t offset, bool read_only);

/**
 * Create a GPT Handle, but validate table by signature
 * @param  path      GPT Handle
 * @param  lba_size  Size of one LBA Sector
 * @param  signature Non standard GPT Signature7
 * @param  offset   Offset (LBA) for GPT table
 * @return           return NULL on error
 */
struct GPT_Handle *gpt_create_handle_with_signature(const char *path,
                                                    unsigned int lba_size,
                                                    const char *signature,
                                                    uint64_t offset,
                                                    bool read_only);

/**
 * Free resources needed by handle
 * @param handle Handle to free
 */
void gpt_close_handle(struct GPT_Handle *handle);

/**
 * Reads GPT from GPT Handle
 * @param handle GPT Handle
 * @return       returns NULL on error
 */
struct GPT_Header *gpt_read_header(struct GPT_Handle *handle);

/**
 * Free resources needed by header
 * @param header Header to free
 */
void gpt_free_header(struct GPT_Header *header);

/**
 * Read Partition information
 * @param  handle       GPT Handle to read from
 * @param  header       GPT Header
 * @param  partition_no Partition number
 * @return              return NULL on error
 */
struct GPT_Entry *gpt_get_entry(struct GPT_Handle *handle,
                                  struct GPT_Header *header, int partition_no);

/**
 * Read all information of all partitions.
 *    Verification of GPT Header is recommended
 * @param  handle GPT Handle to read from
 * @param  header GPT Header
 * @return        return NULL on error on success GPT_Entry array
 *                       with all partitions
 */
struct GPT_Entry *gpt_get_all_entries(struct GPT_Handle *handle,
                                            struct GPT_Header *header);

/**
 * Free resources needed by partition
 * @param entries entry or entries to free
 */
void gpt_free_entries(struct GPT_Entry *entries);

/**
 * Recalculate GPT crc32 checksum
 * @param header GPT header
 */
void gpt_refresh_crc32(struct GPT_Header *header);

/**
 * Recalculate entries crc32 checksum
 * @param header     GPT header
 * @param entries All GPT partitions
 */
void gpt_refresh_entries(struct GPT_Header *header, struct GPT_Entry *entries,
                        int partition_count);
/**
 * Write GPT Header to device or image. The secondary GPT Header
 *      won't be wirtten to disk.
 * @param  handle GPT Handle
 * @param  header GPT Header
 * @return        returns error code
 */
enum GPT_Error gpt_write_header(struct GPT_Handle *handle,
                                    struct GPT_Header *header);

/**
 * Write GPT entries to device or image
 * @param  handle    GPT Handle
 * @param  partition GPT Partition
 * @return           returns error code
 */
enum GPT_Error gpt_write_entries(struct GPT_Handle *handle,
                                    struct GPT_Header *header,
                                    struct GPT_Entry *entries);

/**
 * Write secondary GPT Header to device or image
 * @param  handle GPT Handle
 * @param  header GPT Header
 * @return        returns error code
 */
enum GPT_Error gpt_write_secondary_header(struct GPT_Handle *handle,
                                              struct GPT_Header *header);

/**
 * Verify GPT Header by CRC32 checksum
 * @param  header    GPT Header to verify
 * @return           returns error code
 */
enum GPT_Error gpt_verify_header(struct GPT_Header *header);

/**
 * Verify GPT Entries
 * @param  handle  GPT Handle
 * @param  header  GPT Header
 * @param  entries All GPT Entries
 * @return         returns error code
 */
enum GPT_Error gpt_verify_entries(struct GPT_Handle *handle,
                              struct GPT_Header *header,
                              struct GPT_Entry *entries);
/**
 * Verify secondary GPT Header
 * @param  handle GPT Handle
 * @param  header GPT Header to verify
 * @return        returns error code
 */
enum GPT_Error gpt_verify_scondary_header(struct GPT_Handle *handle,
                                              struct GPT_Header *header);
