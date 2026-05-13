#include "disney_infinity.h"

#include <string.h>

enum
{
  FRAME_PREFIX_QUERY = 0x00,
  FRAME_PREFIX_RESP = 0xAA,
  FRAME_PREFIX_EVENT = 0xAB,
  FRAME_PREFIX_CMD = 0xFF,
  PRESENT_HEXAGON_BASE = 0x10,
  PRESENT_PLAYER1_BASE = 0x20,
  PRESENT_PLAYER2_BASE = 0x30
};

static const uint8_t k_activate_base_response[] = {
    /* Fixed response payload mirrored from Dolphin's command 0x80 behavior. */
    0xAA, 0x15, 0x00, 0x00, 0x0F, 0x01, 0x00, 0x03, 0x02, 0x09, 0x09, 0x43,
    0x20, 0x32, 0x62, 0x36, 0x36, 0x4B, 0x34, 0x99, 0x67, 0x31, 0x93, 0x8C};

/* Mask constant used by Dolphin's Infinity scramble/descramble algorithm. */
static const uint64_t k_scramble_mask = 0x8E55AA1B3999E8AAull;

static uint8_t checksum_u8(const uint8_t* data, uint8_t length)
{
  uint16_t sum = 0;
  uint8_t i;
  for (i = 0; i < length; ++i)
    sum = (uint16_t)(sum + data[i]);
  return (uint8_t)(sum & 0xFFu);
}

static uint32_t rotl32(uint32_t v, uint8_t bits)
{
  return (uint32_t)((v << bits) | (v >> (32u - bits)));
}

static uint64_t scramble_u32(uint32_t num_to_scramble, uint32_t padding_bits)
{
  uint64_t mask = k_scramble_mask;
  uint64_t out = 0;
  uint8_t i;
  for (i = 0; i < 64u; ++i)
  {
    out <<= 1u;
    if ((mask & 1u) != 0u)
    {
      out |= (uint64_t)(num_to_scramble & 1u);
      num_to_scramble >>= 1u;
    }
    else
    {
      out |= (uint64_t)(padding_bits & 1u);
      padding_bits >>= 1u;
    }
    mask >>= 1u;
  }
  return out;
}

static uint32_t descramble_u64(uint64_t in)
{
  uint64_t mask = k_scramble_mask;
  uint32_t out = 0;
  uint8_t i;
  for (i = 0; i < 64u; ++i)
  {
    if ((mask & (1ull << 63u)) != 0u)
      out = (out << 1u) | (uint32_t)(in & 0x1u);
    in >>= 1u;
    mask <<= 1u;
  }
  return out;
}

static void rng_seed(disney_infinity_t* inf, uint32_t seed)
{
  uint8_t i;
  inf->random_a = 0xF1EA5EEDu;
  inf->random_b = seed;
  inf->random_c = seed;
  inf->random_d = seed;
  for (i = 0; i < 23u; ++i)
  {
    uint32_t a = inf->random_a;
    uint32_t b = inf->random_b;
    uint32_t c = inf->random_c;
    uint32_t r = rotl32(inf->random_b, 27u);
    uint32_t temp = (uint32_t)(a + ((r ^ 0xFFFFFFFFu) + 1u));
    b ^= rotl32(c, 17u);
    a = inf->random_d;
    c += a;
    r = b + temp;
    a += temp;
    inf->random_c = a;
    inf->random_a = b;
    inf->random_b = c;
    inf->random_d = r;
  }
}

static uint32_t rng_next(disney_infinity_t* inf)
{
  uint32_t a = inf->random_a;
  uint32_t b = inf->random_b;
  uint32_t c = inf->random_c;
  uint32_t ret = rotl32(inf->random_b, 27u);
  uint32_t temp = (uint32_t)(a + ((ret ^ 0xFFFFFFFFu) + 1u));
  b ^= rotl32(c, 17u);
  a = inf->random_d;
  c += a;
  ret = b + temp;
  a += temp;
  inf->random_c = a;
  inf->random_a = b;
  inf->random_b = c;
  inf->random_d = ret;
  return ret;
}

static void queue_push(disney_infinity_t* inf, const uint8_t frame[32])
{
  if (inf->q_count >= DISNEY_INFINITY_QUEUE_DEPTH)
    return;
  memcpy(inf->queue[inf->q_tail], frame, DISNEY_INFINITY_FRAME_SIZE);
  inf->q_tail = (uint8_t)((inf->q_tail + 1u) % DISNEY_INFINITY_QUEUE_DEPTH);
  inf->q_count++;
}

static bool queue_pop(disney_infinity_t* inf, uint8_t frame[32])
{
  if (inf->q_count == 0u)
    return false;
  memcpy(frame, inf->queue[inf->q_head], DISNEY_INFINITY_FRAME_SIZE);
  inf->q_head = (uint8_t)((inf->q_head + 1u) % DISNEY_INFINITY_QUEUE_DEPTH);
  inf->q_count--;
  return true;
}

static void make_blank_response(uint8_t sequence, uint8_t out[32])
{
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  out[0] = FRAME_PREFIX_RESP;
  out[1] = 0x01;
  out[2] = sequence;
  out[3] = checksum_u8(out, 3);
}

static void response_present_figures(const disney_infinity_t* inf, uint8_t sequence, uint8_t out[32])
{
  uint8_t i;
  uint8_t x = 3;
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  for (i = 0; i < DISNEY_INFINITY_SLOT_COUNT; ++i)
  {
    const disney_figure_slot_t* slot = &inf->storage.slots[i];
    uint8_t base = (i <= DISNEY_INFINITY_HEXAGON_SLOT_MAX)
                       ? PRESENT_HEXAGON_BASE
                       : (i <= DISNEY_INFINITY_PLAYER1_SLOT_MAX) ? PRESENT_PLAYER1_BASE : PRESENT_PLAYER2_BASE;
    if (!slot->present)
      continue;
    out[x] = (uint8_t)(base + slot->order_added);
    out[x + 1u] = 0x09;
    x = (uint8_t)(x + 2u);
  }
  out[0] = FRAME_PREFIX_RESP;
  out[1] = (uint8_t)(x - 2u);
  out[2] = sequence;
  out[x] = checksum_u8(out, x);
}

static void response_get_identifier(const disney_infinity_t* inf, uint8_t order, uint8_t sequence,
                                    uint8_t out[32])
{
  const disney_figure_slot_t* slot = figure_storage_get_by_order(&inf->storage, order);
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  out[0] = FRAME_PREFIX_RESP;
  out[1] = 0x09;
  out[2] = sequence;
  out[3] = 0x00;
  if (slot != NULL && slot->present)
    memcpy(&out[4], slot->data, 7u);
  out[11] = checksum_u8(out, 11);
}

static void response_read_block(const disney_infinity_t* inf, uint8_t order, uint8_t block, uint8_t sequence,
                                uint8_t out[32])
{
  const disney_figure_slot_t* slot = figure_storage_get_by_order(&inf->storage, order);
  uint8_t file_block = figure_storage_protocol_to_file_block(block);
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  out[0] = FRAME_PREFIX_RESP;
  out[1] = 0x12;
  out[2] = sequence;
  out[3] = 0x00;
  if (slot != NULL && slot->present && file_block < DISNEY_INFINITY_BLOCK_COUNT)
    memcpy(&out[4], &slot->data[file_block * DISNEY_INFINITY_BLOCK_SIZE], DISNEY_INFINITY_BLOCK_SIZE);
  out[20] = checksum_u8(out, 20);
}

static void response_write_block(disney_infinity_t* inf, uint8_t order, uint8_t block, const uint8_t* block_data,
                                 uint8_t sequence, uint8_t out[32])
{
  disney_figure_slot_t* slot = figure_storage_get_by_order_mut(&inf->storage, order);
  uint8_t file_block = figure_storage_protocol_to_file_block(block);
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  out[0] = FRAME_PREFIX_RESP;
  out[1] = 0x02;
  out[2] = sequence;
  out[3] = 0x00;
  if (slot != NULL && slot->present && file_block < DISNEY_INFINITY_BLOCK_COUNT)
    memcpy(&slot->data[file_block * DISNEY_INFINITY_BLOCK_SIZE], block_data, DISNEY_INFINITY_BLOCK_SIZE);
  out[4] = checksum_u8(out, 4);
}

static void response_challenge_setup(disney_infinity_t* inf, const uint8_t* in, uint8_t sequence, uint8_t out[32])
{
  uint64_t value = ((uint64_t)in[4] << 56u) | ((uint64_t)in[5] << 48u) | ((uint64_t)in[6] << 40u) |
                   ((uint64_t)in[7] << 32u) | ((uint64_t)in[8] << 24u) | ((uint64_t)in[9] << 16u) |
                   ((uint64_t)in[10] << 8u) | ((uint64_t)in[11]);
  uint32_t seed = descramble_u64(value);
  rng_seed(inf, seed);
  make_blank_response(sequence, out);
}

static void response_challenge_next(disney_infinity_t* inf, uint8_t sequence, uint8_t out[32])
{
  uint64_t scrambled = scramble_u32(rng_next(inf), 0u);
  memset(out, 0, DISNEY_INFINITY_FRAME_SIZE);
  out[0] = FRAME_PREFIX_RESP;
  out[1] = 0x09;
  out[2] = sequence;
  out[3] = (uint8_t)((scrambled >> 56u) & 0xFFu);
  out[4] = (uint8_t)((scrambled >> 48u) & 0xFFu);
  out[5] = (uint8_t)((scrambled >> 40u) & 0xFFu);
  out[6] = (uint8_t)((scrambled >> 32u) & 0xFFu);
  out[7] = (uint8_t)((scrambled >> 24u) & 0xFFu);
  out[8] = (uint8_t)((scrambled >> 16u) & 0xFFu);
  out[9] = (uint8_t)((scrambled >> 8u) & 0xFFu);
  out[10] = (uint8_t)(scrambled & 0xFFu);
  out[11] = checksum_u8(out, 11);
}

void disney_infinity_init(disney_infinity_t* infinity)
{
  memset(infinity, 0, sizeof(*infinity));
  figure_storage_init(&infinity->storage);
}

bool disney_infinity_mount_figure(disney_infinity_t* infinity, uint8_t slot_index, const uint8_t* dump,
                                  uint16_t dump_len)
{
  uint8_t order;
  uint8_t event[32] = {0};
  figure_base_position_t position;
  if (!figure_storage_load_dump(&infinity->storage, slot_index, dump, dump_len, &order))
    return false;
  position = figure_storage_derive_base_position(slot_index);
  if (position == FIGURE_BASE_POSITION_UNKNOWN)
    return true;
  event[0] = FRAME_PREFIX_EVENT;
  event[1] = 0x04;
  event[2] = (uint8_t)position;
  event[3] = 0x09;
  event[4] = order;
  event[5] = 0x00;
  event[6] = checksum_u8(event, 6);
  queue_push(infinity, event);
  return true;
}

bool disney_infinity_unmount_figure(disney_infinity_t* infinity, uint8_t slot_index)
{
  uint8_t order;
  uint8_t event[32] = {0};
  figure_base_position_t position = figure_storage_derive_base_position(slot_index);
  if (!figure_storage_remove(&infinity->storage, slot_index, &order))
    return false;
  if (position == FIGURE_BASE_POSITION_UNKNOWN)
    return true;
  event[0] = FRAME_PREFIX_EVENT;
  event[1] = 0x04;
  event[2] = (uint8_t)position;
  event[3] = 0x09;
  event[4] = order;
  event[5] = 0x01;
  event[6] = checksum_u8(event, 6);
  queue_push(infinity, event);
  return true;
}

void disney_infinity_handle_packet(disney_infinity_t* infinity, const uint8_t out_frame[32], bool* in_valid,
                                   uint8_t in_frame[32])
{
  uint8_t cmd;
  uint8_t seq;
  uint8_t response[32];
  *in_valid = false;
  memset(in_frame, 0, DISNEY_INFINITY_FRAME_SIZE);

  if (out_frame[0] == FRAME_PREFIX_QUERY || out_frame[0] == FRAME_PREFIX_RESP ||
      out_frame[0] == FRAME_PREFIX_EVENT)
  {
    if (queue_pop(infinity, in_frame))
      *in_valid = true;
    return;
  }

  if (out_frame[0] != FRAME_PREFIX_CMD)
    return;

  cmd = out_frame[2];
  seq = out_frame[3];
  memset(response, 0, sizeof(response));

  switch (cmd)
  {
  case 0x80:
    memcpy(response, k_activate_base_response, sizeof(k_activate_base_response));
    break;
  case 0x81:
    response_challenge_setup(infinity, out_frame, seq, response);
    break;
  case 0x83:
    response_challenge_next(infinity, seq, response);
    break;
  case 0x90:
  case 0x92:
  case 0x93:
  case 0x95:
  case 0x96:
  case 0xB5:
    make_blank_response(seq, response);
    break;
  case 0xA1:
    response_present_figures(infinity, seq, response);
    break;
  case 0xA2:
    response_read_block(infinity, out_frame[4], out_frame[5], seq, response);
    break;
  case 0xA3:
    response_write_block(infinity, out_frame[4], out_frame[5], &out_frame[7], seq, response);
    break;
  case 0xB4:
    response_get_identifier(infinity, out_frame[4], seq, response);
    break;
  default:
    make_blank_response(seq, response);
    break;
  }

  queue_push(infinity, response);
}
