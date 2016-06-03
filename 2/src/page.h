#ifndef PAGE_H
#define PAGE_H

enum page_flags
{
  verification = 0x1,
  dirty        = 0x2
};

struct page
{
  uint8_t flags;
 int frame_number;  
};
#endif
