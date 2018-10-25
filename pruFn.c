#include "__pruFn.h"
int pru_setup(const char * const path) {
   int rtn;
   tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;

   if(!path) {
      fprintf(stderr, "pru_setup(): path is NULL\n");
      return -1;
   }

   /* prussdrv_init() will segfault if called with EUID != 0 */ 
   if(geteuid()) {
      fprintf(stderr, "Must run as root to use prussdrv\n");
      return -1;
   }

   /* initialize PRU */
   if((rtn = prussdrv_init()) != 0) {
      fprintf(stderr, "prussdrv_init() failed\n");
      return rtn;
   }

   /* open the interrupt */
   if((rtn = prussdrv_open(PRU_EVTOUT_0)) != 0) {
      fprintf(stderr, "prussdrv_open() failed\n");
      return rtn;
   }
   if((rtn = prussdrv_open(PRU_EVTOUT_1)) != 0) {
      fprintf(stderr, "prussdrv_open() failed\n");
      return rtn;
   }


   /* initialize interrupt */
   if((rtn = prussdrv_pruintc_init(&intc)) != 0) {
      fprintf(stderr, "prussdrv_pruintc_init() failed\n");
      return rtn;
   }

   /* load and run the PRU program */
   if((rtn = prussdrv_exec_program(PRU_NUM, path)) < 0) {
      fprintf(stderr, "prussdrv_exec_program() failed\n");
      return rtn;
   }

   return rtn;
}


int pru_clear_event(unsigned int host_interrupt, unsigned int sysevent){
    void *virt_addr;
    unsigned int intc_phys_addr = INTC_PHYS_BASE;
    virt_addr = prussdrv_get_virt_addr (intc_phys_addr);

    volatile unsigned int *pruintc_io = (volatile unsigned int *) virt_addr;
    pruintc_io[PRU_INTC_SECR1_REG >> 2] = 1 << sysevent;
    pruintc_io[PRU_INTC_HIEISR_REG >> 2] = host_interrupt + 2;
    return 0;
}

int pru_cleanup(void) {
   int rtn = 0;

   /* clear the event (if asserted) */
   if(pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT)) {
      fprintf(stderr, "pru_clear_event() failed\n");
      rtn = -1;
   }

   /* halt and disable the PRU (if running) */
   if((rtn = prussdrv_pru_disable(PRU_NUM)) != 0) {
      fprintf(stderr, "prussdrv_pru_disable() failed\n");
      rtn = -1;
   }

   /* release the PRU clocks and disable prussdrv module */
   if((rtn = prussdrv_exit()) != 0) {
      fprintf(stderr, "prussdrv_exit() failed\n");
      rtn = -1;
   }

   return rtn;
}
