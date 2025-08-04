/**
 * CRC - C Reference Counting Library
 * 
 * A simple, thread-safe reference counting library for C.
 * 
 * Usage:
 *   #define CRC_IMPL        // Include implementation
 *   #include "crc.h"
 * 
 * Basic usage:
 *   int* p = RC_NEW(int);   // Create with refcount=1
 *   int* p2 = RC_CLONE(p);  // Clone reference, refcount=2
 *   RC_DROP(p);             // Drop reference, refcount=1
 *   RC_DROP(p2);            // Drop last reference, auto-free
 */

#ifndef CRC_H
#define CRC_H

#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define RC_MAX_REFS 0x1000000

#ifdef RC_DEBUG_MODE
#define RC_DEBUG(fmt, ...) fprintf(stderr, "[RC_DEBUG] " fmt "\n", ##__VA_ARGS__)
#define RC_ERROR(fmt, ...) fprintf(stderr, "[RC_ERROR] " fmt "\n", ##__VA_ARGS__)
#else  
#define RC_DEBUG(fmt, ...)
#define RC_ERROR(fmt, ...)
#endif
typedef void (*destructor)(void *);
typedef struct {
  _Atomic int count;
  destructor free;
} ref_count;

static inline void* rc_malloc(size_t size, destructor free_fn);
static inline void* rc_inc(void* p);
static inline void* rc_dec(void* p);
static inline int rc_get_count(void* p);
static inline void rc_print_info(void* p);
static inline bool rc_is_valid(void* p);

#ifdef CRC_IMPL
static inline void* rc_malloc(size_t size, destructor free_fn) {
  ref_count* p = (ref_count*)malloc(sizeof(ref_count) + size);
  if (!p) return NULL;
  *p = (ref_count){.count = 1, .free = free_fn};
  return p + 1;
}

static inline void* rc_inc(void* p) {
  if (!p) return NULL;
  ref_count *rc = (ref_count*)p-1;
  atomic_fetch_add_explicit(&rc->count, 1, memory_order_relaxed);
  return p;
}

static inline void* rc_dec(void* p) {
  if (!p) return NULL;
  ref_count *rc = (ref_count*)p-1;
  if (atomic_fetch_sub_explicit(&rc->count, 1, memory_order_relaxed) == 1) {
    rc->free(p);
    free(rc);
    return NULL;
  } 
  return p;
}

static inline int rc_get_count(void* p) {
  if (!p) return -1;
  ref_count *rc = (ref_count*)p-1;
  return atomic_load(&rc->count);
}
static inline void rc_print_info(void* p) {
      if (!p) {
          printf("RC Object: NULL pointer\n");
          return;
      }

      ref_count* rc = (ref_count*)p - 1;
      printf("RC Object: ptr=%p, count=%d, destructor=%p\n",
             p, atomic_load(&rc->count), (void*)rc->free);
  }

static inline bool rc_is_valid(void *p) {
  if (!p) return false;
  if ((uintptr_t)p % sizeof(void*) != 0) return false;
  ref_count* rc = (ref_count*)p-1;
  int count = atomic_load(&rc->count);
  if (count <= 0 || count > RC_MAX_REFS) return false;

  if (!rc->free) return false;

  return true;
}

#define RC_NEW(type) \
  ((type*)rc_malloc(sizeof(type), free))

#define RC_CLONE_SAFE(ptr) \
  ({ \
      typeof(ptr) _tmp = (typeof(ptr))rc_inc(ptr); \
      if (! _tmp && ptr) RC_ERROR("RC_CLONE failed for %p", ptr); \
      _tmp; \
   })

#define RC_DROP_SAFE(ptr) \
  do { \
    if (ptr && !rc_is_valid(ptr)) { \
      RC_ERROR("RC_DROP: invalid pointer %p", ptr); \
      break; \
    } \
    ptr = (typeof(ptr))rc_dec(ptr); \
  } while (0)

#define RC_CLONE(ptr) \
  ((typeof(ptr))rc_inc(ptr))

#define RC_DROP(ptr) \
  do { ptr = (typeof(ptr))rc_dec(ptr); } while(0)

#define RC_NEW_ARRAY(type, count) \
  ((type*)rc_malloc(sizeof(type) * (count), free))

#define RC_NEW_WITH_DESTRUCTOR(type, destructor_fn) \
  ((type*)rc_malloc(sizeof(type), destructor_fn))

#ifdef RC_DEBUG_MODE
#define RC_CLONE_CHECKED RC_CLONE_SAFE
#define RC_DROP_CHECKED RC_DROP_SAFE
#else
#define RC_CLONE_CHECKED RC_CLONE
#define RC_DROP_CHECKED RC_DROP
#endif

// TODO: Cycle Detection
#endif // CRC_IMPL
#endif // CRC_H
