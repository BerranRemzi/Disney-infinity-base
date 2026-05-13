#ifndef FIGURE_STORAGE_H
#define FIGURE_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DISNEY_INFINITY_SLOT_COUNT 9u
#define DISNEY_INFINITY_BLOCK_COUNT 0x14u
#define DISNEY_INFINITY_BLOCK_SIZE 0x10u
#define DISNEY_INFINITY_FIGURE_SIZE (DISNEY_INFINITY_BLOCK_COUNT * DISNEY_INFINITY_BLOCK_SIZE)

#define DISNEY_INFINITY_HEXAGON_SLOT_MAX 2u
#define DISNEY_INFINITY_PLAYER1_SLOT_MAX 5u
#define DISNEY_INFINITY_PLAYER2_SLOT_MAX 8u

typedef struct
{
  uint8_t data[DISNEY_INFINITY_FIGURE_SIZE];
  uint8_t order_added;
  bool present;
} disney_figure_slot_t;

typedef struct
{
  disney_figure_slot_t slots[DISNEY_INFINITY_SLOT_COUNT];
  uint8_t next_order;
} figure_storage_t;

typedef enum
{
  FIGURE_BASE_POSITION_UNKNOWN = 0,
  FIGURE_BASE_POSITION_HEXAGON = 1,
  FIGURE_BASE_POSITION_PLAYER1 = 2,
  FIGURE_BASE_POSITION_PLAYER2 = 3
} figure_base_position_t;

void figure_storage_init(figure_storage_t* storage);
bool figure_storage_load_dump(figure_storage_t* storage, uint8_t slot_index, const uint8_t* dump,
                              size_t dump_len, uint8_t* out_order);
bool figure_storage_remove(figure_storage_t* storage, uint8_t slot_index, uint8_t* out_order);
const disney_figure_slot_t* figure_storage_get_by_order(const figure_storage_t* storage, uint8_t order);
disney_figure_slot_t* figure_storage_get_by_order_mut(figure_storage_t* storage, uint8_t order);
figure_base_position_t figure_storage_derive_base_position(uint8_t slot_index);
uint8_t figure_storage_protocol_to_file_block(uint8_t protocol_block);

#endif
