#include <gpt-manipulator.h>
#include <iostream>
#include <bitset>
#include <cstring>
#include <error.h>

struct UUID {
  uint32_t time_low;
  uint16_t time_mid;
  uint16_t time_high_version;
  uint8_t clock_seq_high_and_reserved;
  uint8_t clock_seq_low;
  uint8_t node[6];
} __attribute__((packed));

void printGUID(const char *prefix, struct UUID *uuid) {
  std::cout.setf(std::ios::hex, std::ios::basefield);
  std::cout.setf(std::ios::uppercase);
  std::cout << prefix << uuid->time_low << "-"
                        << uuid->time_mid << "-"
                        << uuid->time_high_version << "-"
                        << static_cast<int>(uuid->clock_seq_high_and_reserved)
                        << static_cast<int>(uuid->clock_seq_low) << "-";
  for (int x = 0; x < 6; x++) {
    std::cout << static_cast<int>(uuid->node[x]);
  }
  std::cout << std::endl;
  std::cout.setf(std::ios::dec, std::ios::basefield);
  std::cout.unsetf(std::ios::uppercase);
}

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

  std::cout << "Signature: " << header->signature << std::endl;
  std::cout << "Revision: " << std::hex << std::showbase << header->revision << std::endl;
  std::cout << "Header Size: " << std::dec << std::noshowbase << header->header_size << std::endl;
  std::cout << "Header CRC32: " << std::hex << std::showbase << header->crc32_header << std::endl;
  std::cout << "Pimary Position: " << std::dec << std::noshowbase << header->position_primary << std::endl;
  std::cout << "Secondary Position: " << header->position_secondary << std::endl;
  std::cout << "First Partition LBA: " << header->first_partition_lba << std::endl;
  std::cout << "Last Partition LBA: " << header->last_partition_lba << std::endl;
  printGUID("GUID: ", reinterpret_cast<struct UUID *>(header->guid));
  std::cout << "Position Entries: " << header->position_entries << std::endl;
  std::cout << "Entries: " << header->entries << std::endl;
  std::cout << "Entry Size: " << header->entry_size << std::endl;
  std::cout << "Entries CRC32: " << std::hex << std::showbase << header->crc32_entries
            << std::dec << std::noshowbase << std::endl;

  gpt_refresh_crc32(header);
  std::cout << std::endl << "Calculated Header CRC32: "
            << std::hex << std::showbase << header->crc32_header << std::endl;

  struct GPT_Entry *entries = gpt_get_all_entries(handle, header);
  if (entries == NULL) {
    return 3;
  }

  gpt_refresh_entries(header, entries);
  std::cout << "Calculated Entries CRC32: " << header->crc32_entries << std::endl;

  std::cout.setf(std::ios::dec, std::ios::basefield);
  std::cout.unsetf(std::ios::showbase);

  for (int x = 0; x < header->entries; x++) {
    /* skip unused entries */
    if (*reinterpret_cast<uint64_t *>(entries[x].type_guid) == 0 && *(reinterpret_cast<uint64_t *>(entries[x].type_guid) + 1) == 0) {
      continue;
    }
    std::cout << std::endl << "Entry " << x + 1 << ":" << std::endl;
    printGUID("Partition-GUID: ", reinterpret_cast<struct UUID *>(entries[x].type_guid));
    printGUID("GUID: ", reinterpret_cast<struct UUID *>(entries[x].guid));
    std::cout << "Attributes: 0b" << std::bitset<sizeof(GPT_Entry::attributes) * 8>(entries[x].attributes) << std::endl;
    std::cout << "First LBA: " << entries[x].first_lba << std::endl;
    std::cout << "Last LBA: " << entries[x].last_lba << std::endl;
    std::cout << "Name: ";
    for (int y = 0; y < 36; y++) {
      std::cout << static_cast<char>(entries[x].name[y]);
    }
    std::cout << std::endl;
  }

  /* swap two partitions */
  struct GPT_Entry cache;
  std::memcpy(&cache, entries + 1, sizeof(struct GPT_Entry));

  std::memcpy(entries + 1, entries + 2, sizeof(struct GPT_Entry));
  std::memcpy(entries + 2, &cache, sizeof(struct GPT_Entry));

  gpt_refresh_entries(header, entries);
  gpt_refresh_crc32(header);

  std::cout << std::endl;
  GPT_Error error;
  error = gpt_write_secondary_header(handle, header);
  if (error != GPT_SUCCESS) {
    std::cout << "failed to write secondary header" << " code: " << error << std::endl;
  }
  error = gpt_write_entries(handle, header, entries);
  if (error != GPT_SUCCESS) {
      std::cout << "failed to write entries " << strerror(errno) << std::endl;
  }
  error = gpt_write_header(handle, header);
  if (error != GPT_SUCCESS) {
    std::cout << "failed to write primary header" << " code: " << error << std::endl;
  }

  gpt_free_entries(entries);
  gpt_free_header(header);
  gpt_close_handle(handle);
  return 0;
}
