#ifndef VMM_H
#define VMM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "conf.h"
#include "physical_memory.h"
#include "tlb.h"



struct virtual_memory_manager
{
  unsigned int page_fault_count;
  unsigned int page_found_count;
  unsigned int tlb_hit_count;
  unsigned int tlb_miss_count;

  struct page page_table[NUM_PAGES];

  struct tlb tlb;
  struct physical_memory pm;

  FILE* log;
  FILE* backstore;
};

void vmm_init(struct virtual_memory_manager*, FILE*, FILE*, FILE*, FILE*);
void vmm_read(struct virtual_memory_manager*, uint16_t);
void vmm_write(struct virtual_memory_manager*, uint16_t, char);
void vmm_clean(struct virtual_memory_manager*);

#endif
