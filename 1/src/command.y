%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "vmm.h"
#include "common.h"

/* Virtual Memory Manager */
struct virtual_memory_manager vmm;

const char* usage =
"\n"
"Usage: %s [-p outfile] [-t outfile] [-c outfile] [-h] file\n"
"\n"
"Options\n"
"-------\n"
"\n"
"  -p --log-physical-memory outfile\n"
"                  Outputs the final state of the physical memory\n"
"                  in outfile.\n"
"\n"
"  -t --log-tlb-state outfile\n"
"                  Outputs the final tlb state in outfile.\n"
"\n"
"  -c --log-command outfile\n"
"                  Outputs the command log in outfile.\n"
"\n"
"  -h --help       Prints this message\n"
"\n"
"\n";


/* Parser specific variable */
int recovering = false;
extern int yylex(void);
extern char* yytext;
void yyerrorc(const char c);
void yyerror(const char* msg);

%}

%union {
  uint16_t ivalue;
  char     cvalue;
}

%token tREAD "read command"
%token tWRITE "write command"
%token tERROR
%token tEND
%token <ivalue> tADDRESS "address"
%token <cvalue> tCHAR "character litteral"

%start start

%error-verbose

%%


start: commands_list

commands_list:
  /* empty */
| commands_list command
;


command:
  ';'
| tWRITE tADDRESS tCHAR ';'
    { vmm_write(&vmm, $2, $3); }
| tREAD tADDRESS ';'
    { vmm_read(&vmm, $2); }
| error ';'
    { 
      recovering = false;
      yyerrok;
    }
;

%%
void yyerrorc(const char c) {
  if (!recovering)
    {
      printf("Invalid command: unexpected character \\%d.\n", c);
      recovering = true;
    }
}

void yyerror(const char* msg) {
  if (!recovering)
    {
      printf("Invalid command: %s.\n", msg);
      recovering = true;
    }
}


int main(int argc, char* argv[argc+1])
{
  FILE* pm_log = NULL;
  FILE* tlb_log = NULL;
  FILE* vmm_log = NULL;
  FILE* backstore;

  size_t opt;
  for (opt = 1; opt < argc && argv[opt][0] == '-'; opt++)
    {
      switch (argv[opt][1])
        { case '-':
        if (strcmp(argv[opt], "--log-physical-memory") == 0) case 'p': {
            error(opt == argc - 2, "Option `%s` expected file name\n", argv[opt]);
            pm_log = fopen(argv[++opt], "w+"); 
            error(!pm_log, "Could not open file `%s`\n", argv[opt]);

        } else if (strcmp(argv[opt], "--log-tlb") == 0) case 't': {
            error(opt == argc - 2, "Option `%s` expected file name\n", argv[opt]);
            tlb_log = fopen(argv[++opt], "w+"); 
            error(!tlb_log, "Could not open file `%s`", argv[opt]);
        }

        else if (strcmp(argv[opt], "--log-command") == 0) case 'c': {
            error(opt == argc - 2, "Option `%s` expected file name\n", argv[opt]);
            vmm_log = fopen(argv[++opt], "w+"); 
            error(!vmm_log, "Could not open file `%s`\n", argv[opt]);
        }

        else default: {
            error(true, usage, argv[0]);
        }}
    }

  error(opt == argc, usage, argv[0]);
  backstore = fopen(argv[opt], "r+");
  error(!backstore, "Could not open file `%s`\n", argv[opt]);

  /* Initialisation*/
  vmm_init(&vmm, backstore, vmm_log, tlb_log, pm_log);

  /* Exécution */
  yyparse();

  /* Cleanup */
  vmm_clean(&vmm);
  if (pm_log)  fclose(pm_log);
  if (tlb_log) fclose(tlb_log);
  if (vmm_log) fclose(vmm_log);
  fclose(backstore);

  return EXIT_SUCCESS;
}
