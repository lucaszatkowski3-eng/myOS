#pragma once
#include <stdint.h>
typedef struct { volatile uint32_t value; } spinlock_t;
static inline void spin_init(spinlock_t *l){ l->value=0; }
static inline void spin_lock(spinlock_t *l){ uint32_t expected; do { expected=0; } while(!__atomic_compare_exchange_n(&l->value,&expected,1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED)); }
static inline void spin_unlock(spinlock_t *l){ __atomic_store_n(&l->value,0,__ATOMIC_RELEASE); }
