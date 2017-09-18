#include <gpt-manipulator.h>
#include <stdio.h>

struct UUID {
  uint32_t time_low;
  uint16_t time_mid;
  uint16_t time_high_version;
  uint8_t clock_seq_high_and_reserved;
  uint8_t clock_seq_low;
  uint8_t node[6];
} __attribute__((packed));

int main() {
  struct GPT_Handle *handle;
  handle = gpt_create_handle("../test/test.img",
                                            GPT_DEFAULT_LBA_SIZE,
                                            GPT_DEFAULT_OFFSET,
                                            false);
  if (handle == NULL) {
    return 1;
  }

  struct GPT_Header *header = gpt_read_header(handle);
  if (header == NULL) {
    return 2;
  }

  struct UUID *uuid = (struct UUID *)header->guid;

  printf("Signature: %s\n", header->signature);
  printf("Revision: 0x%xi\n", header->revision);
  printf("Header Size: %i\n", header->header_size);
  printf("Header CRC32: 0x%x\n", header->crc32_header);
  printf("Pimary Position: %i\n", header->position_primary);
  printf("Secondary Position: %i\n", header->position_secondary);
  printf("First Partition LBA: %i\n", header->first_partition_lba);
  printf("Last Partition LBA: %i\n", header->last_partition_lba);
  printf("GUID: %x-%x-%x\n", uuid->time_low,
                                     uuid->time_mid,
                                     uuid->time_high_version,
                                     uuid->clock_seq_high_and_reserved,
                                     uuid->clock_seq_low,
                                     *(uint16_t *)(header->guid + 14));


  return 0;
}
