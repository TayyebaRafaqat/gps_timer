#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 0 /* which of the two PRUs are we using? */
#define INTC_PHYS_BASE                0x4a320000
#define PRU_INTC_SECR1_REG   0x280
#define PRU_INTC_HIEISR_REG  0x034

int pru_setup(const char * const path);
int pru_cleanup(void);
int pru_clear_event(unsigned int host_interrupt, unsigned int sysevent);
