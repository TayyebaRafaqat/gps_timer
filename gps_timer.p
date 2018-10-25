#define PRU0_ARM_INTERRUPT 19
#define PRU1_ARM_INTERRUPT 20
#define CONST_PRUCFG C4
#define CONST_IEP C26

// Offset Adresses
#define IEP_REG_GLOBAL_CFG 0x00
#define IEP_REG_COMPEN 0x08
#define IEP_REG_COUNT 0x0c
#define IEP_REG_CMP_CFG 0x40
#define IEP_REG_CMP0 0x48
#define INTC_SECR0 0x20280
#define INTC_HIEISR 0x20034
#define IEP_REG_CMP_STATUS 0x44

#define timer 1000000000  // 5 sec. 200000000 is 1 second.
//#define LoopCount 5 
#define EGPI 14  // GPI of PRU 
#define EGPO 14  // GP0 of PRU 

.origin 0
.entrypoint MAIN

MAIN:
// Enable OCP master port
LBCO r0, CONST_PRUCFG, 4, 4
CLR r0, r0, 4
SBCO r0, CONST_PRUCFG, 4, 4

// setup IEP module

// enable timer clock PRU_ICSS_CFG.CGR.IEP_CLK_EN=1
LBCO r0, CONST_PRUCFG, 0x10, 4                    
SET r0, 17
SBCO r0, CONST_PRUCFG, 0x10, 4   

// define increment value i.e 1
MOV r0, (1 << 8) | (1 << 4)
SBCO r0, CONST_IEP, IEP_REG_GLOBAL_CFG, 4

// Compensation Disabled, Default Increment
MOV r0, 0
SBCO r0, CONST_IEP, IEP_REG_COMPEN, 4

// Reset Count
MOV r0, 0
SBCO r0, CONST_IEP, IEP_REG_COUNT, 4

// Enables Reset of counter after event, Enables event
MOV r0, (1 << 1) | (1 << 0)
SBCO r0, CONST_IEP, IEP_REG_CMP_CFG, 4

// timer comparison value
MOV r0, timer
// Load timer Value in comparison register CMP0
SBCO r0, CONST_IEP, IEP_REG_CMP0, 4

// Clear status
SBCO 0, CONST_IEP, IEP_REG_CMP_STATUS, 4

// Clear ouput Pin
CLR r30, r30, EGPO

//MOV r5, LoopCount

// Wait for ARM_PRU0_INTERRUPT
WBS r31.t30

// Wait for PPS 
WBS r31, EGPI

// STart Timer
LBCO r0, CONST_IEP, IEP_REG_GLOBAL_CFG, 4
SET r0, 0
SBCO r0, CONST_IEP, IEP_REG_GLOBAL_CFG, 4

// Acknowlege
MOV r31.b0, PRU0_ARM_INTERRUPT + 16

// Clear ARM_PRU0_INTERRUPT
MOV r4, 1 << 21
MOV r5, INTC_SECR0
SBBO r4, r5, 0, 4

IEP_WAIT_EVENT:
// Polling status bit
LBCO r2, CONST_IEP, IEP_REG_CMP_STATUS, 4
QBBS END, r31, 30   // Polling ARM_PRU0_INTERRUPT
QBBC IEP_WAIT_EVENT, r2, 0

// Event Achieved
// Clear status
SBCO 0, CONST_IEP, IEP_REG_CMP_STATUS, 4

// PRU0_ARM_INTERRUPT
MOV r31.b0, PRU0_ARM_INTERRUPT + 16

SET r30, r30, EGPO

// Delay for 1 sec.. PIN high
MOV r6, 100000000
DELAY:
    SUB r6, r6, 1
    QBNE DELAY, r6, 0

// Clear Pin
CLR r30, r30, EGPO
//SET r30, r30, EGPO

JMP IEP_WAIT_EVENT
    
END:
// Clear ARM_PRU0_INTERRUPT
    MOV r4, 1 << 21
    MOV r5, INTC_SECR0
    SBBO r4, r5, 0, 4

    MOV r31.b0, PRU1_ARM_INTERRUPT + 16

    HALT