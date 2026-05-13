#ifndef DISNEY_INFINITY_H
#define DISNEY_INFINITY_H

#include <stdbool.h>
#include <stdint.h>

#include "figure_storage.h"

#define DISNEY_INFINITY_FRAME_SIZE 32u
#define DISNEY_INFINITY_QUEUE_DEPTH 8u

typedef struct
{
  figure_storage_t storage;
  uint32_t random_a;
  uint32_t random_b;
  uint32_t random_c;
  uint32_t random_d;
  uint8_t queue[DISNEY_INFINITY_QUEUE_DEPTH][DISNEY_INFINITY_FRAME_SIZE];
  uint8_t q_head;
  uint8_t q_tail;
  uint8_t q_count;
} disney_infinity_t;

void disney_infinity_init(disney_infinity_t* infinity);
bool disney_infinity_mount_figure(disney_infinity_t* infinity, uint8_t slot_index, const uint8_t* dump,
                                  uint16_t dump_len);
bool disney_infinity_unmount_figure(disney_infinity_t* infinity, uint8_t slot_index);
void disney_infinity_handle_packet(disney_infinity_t* infinity, const uint8_t out_frame[32],
                                   bool* in_valid, uint8_t in_frame[32]);

#endif
