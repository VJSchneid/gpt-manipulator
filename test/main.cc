#include <gpt-manipulator.h>
#include <iostream>

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

  std::cout << "Signature: " << header->signature << std::endl;
  std::cout << "Revision: " << std::hex << std::showbase << header->revision << std::endl;
  std::cout << "Header Size: " << std::dec << std::noshowbase << header->header_size << std::endl;
  std::cout << "Header CRC32: " << std::hex << std::showbase << header->crc32_header << std::endl;
  std::cout << "Pimary Position: " << std::dec << std::noshowbase << header->position_primary << std::endl;
  std::cout << "Secondary Position: " << header->position_secondary << std::endl;
  std::cout << "First Partition LBA: " << header->first_partition_lba << std::endl;
  std::cout << "Last Partition LBA: " << header->last_partition_lba << std::endl;
  std::cout.setf(std::ios::hex, std::ios::basefield);
  std::cout.setf(std::ios::uppercase);
  std::cout << "GUID: " << uuid->time_low << "-"
                        << uuid->time_mid << "-"
                        << uuid->time_high_version << "-"
                        << static_cast<int>(uuid->clock_seq_high_and_reserved) << static_cast<int>(uuid->clock_seq_low) << "-";
  for (int x = 0; x < 6; x++) {
    std::cout << static_cast<int>(uuid->node[x]);
  }
  std::cout << std::endl;
  std::cout.setf(std::ios::dec, std::ios::basefield);

  return 0;
}
