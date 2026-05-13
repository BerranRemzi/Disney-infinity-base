#include "figure_storage.h"

#include <string.h>

void figure_storage_init(figure_storage_t* storage)
{
  uint8_t i;
  memset(storage, 0, sizeof(*storage));
  storage->next_order = 0;
  for (i = 0; i < DISNEY_INFINITY_SLOT_COUNT; ++i)
    storage->slots[i].order_added = 0xFF;
}

bool figure_storage_load_dump(figure_storage_t* storage, uint8_t slot_index, const uint8_t* dump,
                              size_t dump_len, uint8_t* out_order)
{
  disney_figure_slot_t* slot;
  if (slot_index >= DISNEY_INFINITY_SLOT_COUNT || dump == NULL || dump_len != DISNEY_INFINITY_FIGURE_SIZE)
    return false;

  slot = &storage->slots[slot_index];
  memcpy(slot->data, dump, DISNEY_INFINITY_FIGURE_SIZE);
  slot->present = true;
  if (slot->order_added == 0xFF)
    slot->order_added = storage->next_order++;

  if (out_order != NULL)
    *out_order = slot->order_added;
  return true;
}

bool figure_storage_remove(figure_storage_t* storage, uint8_t slot_index, uint8_t* out_order)
{
  disney_figure_slot_t* slot;
  if (slot_index >= DISNEY_INFINITY_SLOT_COUNT)
    return false;

  slot = &storage->slots[slot_index];
  if (!slot->present)
    return false;

  slot->present = false;
  if (out_order != NULL)
    *out_order = slot->order_added;
  return true;
}

const disney_figure_slot_t* figure_storage_get_by_order(const figure_storage_t* storage, uint8_t order)
{
  uint8_t i;
  for (i = 0; i < DISNEY_INFINITY_SLOT_COUNT; ++i)
  {
    if (storage->slots[i].order_added == order)
      return &storage->slots[i];
  }
  return NULL;
}

disney_figure_slot_t* figure_storage_get_by_order_mut(figure_storage_t* storage, uint8_t order)
{
  uint8_t i;
  for (i = 0; i < DISNEY_INFINITY_SLOT_COUNT; ++i)
  {
    if (storage->slots[i].order_added == order)
      return &storage->slots[i];
  }
  return NULL;
}

figure_base_position_t figure_storage_derive_base_position(uint8_t slot_index)
{
  if (slot_index <= 2u)
    return FIGURE_BASE_POSITION_HEXAGON;
  if (slot_index <= 5u)
    return FIGURE_BASE_POSITION_PLAYER1;
  if (slot_index <= 8u)
    return FIGURE_BASE_POSITION_PLAYER2;
  return FIGURE_BASE_POSITION_UNKNOWN;
}

uint8_t figure_storage_protocol_to_file_block(uint8_t protocol_block)
{
  if (protocol_block == 0u)
    return 1u;
  return (uint8_t)(protocol_block * 4u);
}
