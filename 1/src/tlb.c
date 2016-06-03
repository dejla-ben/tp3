
#include <stdint.h>
#include <stdio.h>

#include "tlb.h"

// Ne pas modifier cette fonction
void
tlb_init (struct tlb *tlb, FILE * log)
{
  for (unsigned int i = 0; i < TLB_NUM_ENTRIES; i++)
    {
      tlb->entries[i].page_number = -1;
      tlb->entries[i].frame_number = -1;
    }

  tlb->log = log;
  tlb->next_entry_available = 0;
}


// Ne pas modifier cette fonction
void
tlb_clean (struct tlb *tlb)
{
  if (tlb->log)
    {
      for (unsigned int i = 0; i < TLB_NUM_ENTRIES; i++)
	{
	  fprintf (tlb->log,
		   "%d: %d -> %d\n",
		   i,
		   tlb->entries[i].page_number, tlb->entries[i].frame_number);
	}
    }
}


// Sur entrée d'un numéro de page, retourne le numéro de frame
// associé. S'il n'y a pas d'entrée correspondante à la page,
// -1 est retourné.
int32_t
tlb_lookup (struct tlb *tlb, uint16_t page_number)
{
  // Complétez cette fonction.
  int i=0;
    for( i = 0 ; i < TLB_NUM_ENTRIES; i++ ){
   
        if(tlb->entries[i].page_number==page_number)
        {
			tlb->entries[i].use++;
            return tlb->entries[i].frame_number;
        }
    }
    ///Le tlb n'est pas trouve'( TLB Miss)
    return -1;
}

int findFreeEntry(struct tlb *tlb){
    for(int i=0; i<TLB_NUM_ENTRIES; i++){
        if(tlb->entries[i].page_number == -1){
            printf("TLB Entry Empty is: %d\n",i);          
            return i;
        }
    }
    return -1; //TLB IS FULL;
}

// Ajoute une entré dans le tlb. Pour bien mettre en oeuvre
// cette fonction, il est nécessaire d'avoir un algorithme
// de remplacement pour bien gérer un tlb plein.
void
tlb_add_entry (struct tlb *tlb, uint16_t page_number, int32_t frame_number)
{
  // Complétez cette fonction.
  int i = findFreeEntry(tlb);
    
    if(i!= -1){
		printf("test tlb_add_entry %d\n",i);
        tlb->entries[i].page_number  = page_number;
        tlb->entries[i].frame_number = frame_number; 
        tlb->entries[i].age = ++ageCounter;  //for first come first serve.
        tlb->entries[i].use = 1; //first time using it (when we add the entry)
    }

    else if(i == -1){
        replaceWithFifo(tlb,page_number, frame_number);
               
    }
}



void
replaceWithFifo(struct tlb *tlb, uint16_t page_number, int16_t frame_number){
		int min = ageCounter +1;
		
        int index = -1;
        for(int i=0; i<TLB_NUM_ENTRIES; i++){
            if(tlb->entries[i].age < min){
                index = i;
                min = tlb->entries[i].age;  //replace entry with smallest age(been there the longest (FIFO))
				            }

        }
        tlb->entries[index].page_number = page_number;
        tlb->entries[index].frame_number = frame_number;
        tlb->entries[index].age = ++ageCounter; //add replacement, set age to largest;
}
