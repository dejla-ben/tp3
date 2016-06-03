#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "common.h"
#include "vmm.h"
#include "tlb.h"
#include "physical_memory.h"


void
vmm_init (struct virtual_memory_manager *vmm,
          FILE * backing_store, FILE * vmm_log, FILE * tlb_log, FILE * pm_log)
{
    
    // Initialise la mémoire physique.
    pm_init (&vmm->pm, backing_store, pm_log);
    tlb_init (&vmm->tlb, tlb_log);
    
    // Initialise les compteurs.
    vmm->page_fault_count = 0;
    vmm->page_found_count = 0;
    vmm->tlb_hit_count = 0;
    vmm->tlb_miss_count = 0;
    
    // Initialise le fichier de journal.
    vmm->log = vmm_log;
	vmm ->backstore = backing_store ;
    
    // Initialise la table de page.
    for (int i=0; i < NUM_PAGES; i++)
    {
        vmm->page_table[i].flags = 0x0;
        vmm->page_table[i].frame_number = -1;
    }
    
}


// NE PAS MODIFIER CETTE FONCTION
void
vmm_log_command (FILE * out, const char *command, uint16_t laddress,	/* logical address */
                 uint16_t page, int32_t frame, uint16_t offset, uint16_t paddress,	/* physical address */
                 char c)	/* char lu ou écrit */
{
    if (out)
    {
        fprintf (out, "%s[%c]@%d: p=%d, f=%d, o=%d pa=%d\n", command, c, laddress,
                 page, frame, offset, paddress);
    }
}


/* Effectue une lecture à l'adresse logique `laddress` */
void
vmm_read (struct virtual_memory_manager *vmm, uint16_t laddress)
{

  // Vous devez fournir les arguments manquants. Basez-vous sur la signature de
  // la fonction.
    
    uint16_t page = laddress>>8;
    uint16_t offset=laddress&0xFF;
    int32_t frame;
    uint16_t paddress=0;
    
    char value='0';
    
    // chercher dans le tlb
    
    frame=tlb_lookup(&vmm->tlb,page);
    
    if(frame == -1){
        vmm->tlb_miss_count++;
        //chercher dans la table du page
        frame = vmm->page_table[page].frame_number;
        if(frame==-1){
            vmm->page_fault_count++;
            frame=pm_demand_page(&vmm->pm,page,vmm->page_table );
            
            if (frame==-1)
            {
                vmm->page_fault_count++;
            }
            
            else {
                vmm->page_table[page].frame_number=frame;
                tlb_add_entry(&vmm->tlb,page,frame);
            }
        }else{
            vmm->page_found_count++;
            vmm->page_table[page].frame_number=frame;
            tlb_add_entry(&vmm->tlb,page,frame);
            
        }
        
    } else{
        
        
        vmm->tlb_hit_count++;
        vmm->page_found_count++;
    }
    
    
    
    paddress=(frame*PAGE_FRAME_SIZE)+offset;
    value = vmm->pm.memory[frame*PAGE_FRAME_SIZE+offset] ;
    vmm_log_command (stdout, "READING", laddress, page, frame, offset, paddress,value);
    
   
}

/* Effectue une écriture à l'adresse logique `laddress` */

void
vmm_write (struct virtual_memory_manager *vmm, uint16_t laddress, char c)
{
    
    /* Complétez */
	uint16_t paddress=0;
    uint16_t page=laddress >>8;
    uint16_t offset=laddress&0xFF;
	int32_t frame;
    int index;
	frame= tlb_lookup(&vmm->tlb,page);
    if(frame == -1){
        vmm->tlb_miss_count++;
        frame = vmm->page_table[page].frame_number;
        if(frame==-1){
            vmm->page_fault_count++;
            frame=pm_demand_page(&vmm->pm,page,vmm->page_table );
            
            if (frame==-1)
                {
                vmm->page_fault_count++;
                }
            
            else {
                vmm->page_table[page].frame_number=frame;
                
                tlb_add_entry(&vmm->tlb,page,frame);
            }
            
            
        }else{
            vmm->page_found_count++;
            vmm->page_table[page].frame_number=frame;
            tlb_add_entry(&vmm->tlb,page,frame);
            
        }
        
    } else{
        vmm->tlb_hit_count++;
        vmm->page_found_count++;
    }
    // la page a ete modifié
    vmm->page_table[page].flags|=dirty;
    
    index = vmm->page_table[page].frame_number*PAGE_FRAME_SIZE+offset;
    vmm->pm.memory[index] = c;
    paddress=(vmm->page_table[page].frame_number)+offset;
    
    
    vmm_log_command (stdout, "WRITING", laddress, page,vmm->page_table[page].frame_number, offset, paddress, c);
    
    

}


// NE PAS MODIFIER CETTE FONCTION
void
vmm_clean (struct virtual_memory_manager *vmm)
{
    fprintf (stdout, "\n\n");
    fprintf (stdout, "tlb hits:   %d\n", vmm->tlb_hit_count);
    fprintf (stdout, "tlb miss:   %d\n", vmm->tlb_miss_count);
    fprintf (stdout, "tlb hit ratio:   %f\n",
             vmm->tlb_hit_count / (float)(vmm->tlb_miss_count + vmm->tlb_miss_count));
    fprintf (stdout, "page found: %d\n", vmm->page_found_count);
    fprintf (stdout, "page fault: %d\n", vmm->page_fault_count);
    fprintf (stdout, "page fault ratio:   %f\n",
             vmm->page_fault_count / (float)(vmm->page_found_count + vmm->page_fault_count));
    
    if (vmm->log)
    {
        for (unsigned int i = 0; i < NUM_PAGES; i++)
        {
            fprintf (vmm->log,
                     "%d -> {%d, %d%d%d%d%d%d%d%d}\n",
                     i,
                     vmm->page_table[i].frame_number,
                     vmm->page_table[i].flags & 1,
                     vmm->page_table[i].flags & (1 << 1),
                     vmm->page_table[i].flags & (1 << 2),
                     vmm->page_table[i].flags & (1 << 3),
                     vmm->page_table[i].flags & (1 << 4),
                     vmm->page_table[i].flags & (1 << 5),
                     vmm->page_table[i].flags & (1 << 6),
                     vmm->page_table[i].flags & (1 << 7));
        }
    }
    tlb_clean (&vmm->tlb);
	// pm_clean (&vmm->pm);   
	pm_clean (&vmm->pm, vmm->backstore,&vmm->page_table[0]);
}
