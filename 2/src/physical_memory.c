#include <stdio.h>
#include <stdbool.h>
#include "conf.h"
#include "physical_memory.h"
bool freeFrame;
FILE * backstore;


// Initialise la mémoire physique
void
pm_init (struct physical_memory *pm, FILE * backing_store, FILE * pm_log)
{
    pm->backing_store = backing_store;
    backstore = backing_store;
    pm->log = pm_log;
    
    for (unsigned int i = 0; i < PHYSICAL_MEMORY_SIZE; i++)
    {
        pm->memory[i] = ' ';
    }
}

// Retourne le numéro d'une trame (frame) libre
uint16_t
pm_find_free_frame (struct physical_memory *pm, struct page page_table[NUM_PAGES] )
{
    
    for(int i = 0; i < NUM_FRAMES; i++){ // Pour chaque frame
        
        freeFrame = false;
        for(int j = 0; j < PAGE_FRAME_SIZE; j++){ // Si tous les byte du frame sont vide
            if(pm->memory[(i * PAGE_FRAME_SIZE) + j] == ' ')
                freeFrame = true;
            else{
                freeFrame = false;
                break;
            }
        }
        if(freeFrame)
            return i;
    }
    
    return -1;

}

// Charge la page demandée du backing store
uint16_t
pm_demand_page (struct physical_memory *pm, uint16_t page,
                struct page page_table[NUM_PAGES])
{
    
    uint16_t f = pm_find_free_frame(pm,page_table);
    fseek( pm->backing_store, page*PAGE_FRAME_SIZE, SEEK_SET );
    
    fread( pm->memory+(f*PAGE_FRAME_SIZE), sizeof(char),
          PAGE_FRAME_SIZE, pm->backing_store);
    
    
    
    return f;
    
    
    
    
}

// Sauvegarde la page spécifiée
void
pm_backup_frame (struct physical_memory *pm, FILE *backstore,uint16_t frame,
                 uint16_t page)
{
    
    if(page < NUM_PAGES  && frame < NUM_FRAMES){
        fseek( pm->backing_store, page*PAGE_FRAME_SIZE, SEEK_SET );
        fwrite( pm->memory+(frame*PAGE_FRAME_SIZE), PAGE_FRAME_SIZE,
               sizeof(char) , pm->backing_store);
        fseek(backstore, page*PAGE_FRAME_SIZE, SEEK_SET );
        fwrite( pm->memory+(frame*PAGE_FRAME_SIZE), PAGE_FRAME_SIZE,
               sizeof(char) , backstore);
        
    }
}

void
pm_clean (struct physical_memory *pm, FILE *backstore, struct page page_table[NUM_PAGES])
{
    
    for(int i=0;i<NUM_PAGES;i++){
        if(page_table[i].flags==dirty){
            
            pm_backup_frame(pm,backstore,page_table[i].frame_number,i);
            
            
            
        }
    }
    
    // Enregistre l'état de la mémoire physique.
    if (pm->log)
    {
        for (unsigned int i = 0; i < PHYSICAL_MEMORY_SIZE; i++)
        {
            if (i % 80 == 0)
                fprintf (pm->log, "%c\n", pm->memory[i]);
            else
                fprintf (pm->log, "%c", pm->memory[i]);
        }
    }
}
