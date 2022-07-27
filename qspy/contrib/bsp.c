/*.$file${.::bsp::bsp.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: blinky_rtt.qm
* File:  ${.::bsp::bsp.c}
*
* This code has been generated by QM 5.1.4 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${.::bsp::bsp.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* Board Support Package implementation for NUCLEO-F401RE board */
#include "qpc.h"    /* QP/C framework API */
#include "bsp.h"    /* Board Support Package interface */

Q_DEFINE_THIS_FILE

#ifdef Q_SPY

/* STM32F401xx - SEGGER J-Link device name: STM32F401RE --------------------*/
#include "rtt.h"

    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;

    /* QSpy source IDs */
    static QSpyId const l_SysTick_Handler = { 0U };

#endif /* Q_SPY */

/* ISRs used in the application ==========================================*/
void SysTick_Handler(void);

/*..........................................................................*/
void SysTick_Handler(void) {

#ifdef Q_SPY
  {
    uint32_t tmp = SysTick->CTRL; /* clear SysTick_CTRL_COUNTFLAG */
    (void) tmp;
    QS_tickTime_ += QS_tickPeriod_; /* account for the clock rollover */
  }
#endif /* Q_SPY */

  QF_TICK_X(0U, SysTick_Handler); /* process time events for rate 0 */
  QV_ARM_ERRATUM_838869();
}

/* BSP functions ===========================================================*/
static void SystemClock_Config(void);

/*..........................................................................*/
void BSP_init(void) {
    SystemClock_Config();

    /* LED for NUCLEO-F401RE board(the GREEN led, PA5, active high) ------------*/
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
    BSP_ledOff();
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
    MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED5, GPIO_OSPEEDR_OSPEED5_0);

    /* initialize the QS software tracing... */
#ifdef Q_SPY
    if (QS_INIT((void *)0) == 0) { /* initialize the QS software tracing */
        Q_ERROR();
    }
    QS_FUN_DICTIONARY(&l_SysTick_Handler);
#endif /* Q_SPY */
} //BSP_Init()
/*..........................................................................*/
void BSP_ledOff(void) {
	GPIOA->BSRR = GPIO_BSRR_BR5;
}
/*..........................................................................*/
void BSP_ledOn(void) {
	GPIOA->BSRR = GPIO_BSRR_BS5;
}
/*..........................................................................*/
//
// System Clock Configuration ... 8MHz (ext. signal) --> 84MHz (PLL)
// Remark: STM32F401 has max 84 MHz
//
static void SystemClock_Config(void) {
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    MODIFY_REG(PWR->CR, PWR_CR_VOS, PWR_CR_VOS_0);
    SET_BIT(SYSCFG->CMPCR, SYSCFG_CMPCR_CMP_PD);
    // HSE configuration
    SET_BIT(RCC->CR, RCC_CR_HSEBYP | RCC_CR_HSEON);
    while ((RCC->CR & RCC_CR_HSERDY) == RESET)
        // Wait for HSE start-up (WARNING, no timeout)
        ;
    // PLL configuration - output at 84 MHz
    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN,
            RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLN_2
            | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLSRC_HSE);
    SET_BIT(RCC->CR, RCC_CR_PLLON); // PLL ON
    while ((RCC->CR & RCC_CR_PLLRDY) == RESET)
        // Wait for PLL start-up (WARNING, no timeout)
        ;
    // FLASH timing: > 48MHz
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2WS);
    // PCLK1 at max
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1_Msk, RCC_CFGR_PPRE1_DIV2);
    // Switch SYSCLK to PLL ==> HCLK at max
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_PLL);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL)
        // Wait for SYSCLK at PLL (WARNING, no timeout)
        ;
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);
    // Update SystemCoreClock (expected result: 84MHz)
    SystemCoreClockUpdate();
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
 * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
 */
enum KernelUnawareISRs { /* see NOTE00 */
    /* ... */
    MAX_KERNEL_UNAWARE_CMSIS_PRI /* keep always last */
};
/* "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts */
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);

enum KernelAwareISRs {
    DUMMY_PRIO = QF_AWARE_ISR_CMSIS_PRI,
	SYSTICK_PRIO,
    /* ... */
    MAX_KERNEL_AWARE_CMSIS_PRI /* keep always last */
};
/* "kernel-aware" interrupts should not overlap the PendSV priority */
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

/* Local-scope objects -----------------------------------------------------*/

/* QF callbacks ============================================================*/
void QF_onStartup(void) {
	/* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE00
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    NVIC_SetPriorityGrouping(0U);
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO);
    /* ... */

    /* enable IRQs... */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QV_onIdle(void) {
//
// The QV_onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV_onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
#ifdef Q_SPY
	QF_INT_ENABLE();

	if (rtt_has_data()) {
        QS_RX_PUT((uint8_t)rtt_getchar());
    	QS_rxParse();  /* parse all the received bytes */
    }

    if (rtt_can_write()) {
    	uint16_t b;
        // RTT write is buffer is not full, try to send next Byte
        QF_INT_DISABLE();
        b = QS_getByte();
        QF_INT_ENABLE();
        if (b != QS_EOD) {
            // Byte to send exists, send it now
            rtt_putchar(b & 0xFF);
        }
    }

#elif defined NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
     * you might need to customize the clock management for your application,
     * see the datasheet for your particular Cortex-M MCU.
     */
    /* !!!CAUTION!!!
    * QV_CPU_SLEEP() contains the WFI instruction, which stops the CPU
    * clock, which unfortunately disables the JTAG port, so the ST-Link
    * debugger can no longer connect to the board. For that reason, the call
    * to QV_CPU_SLEEP() has to be used with CAUTION.
    */
    /* NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    * reset the board, then connect with ST-Link Utilities and erase the part.
    * The trick with BOOT(0) is it gets the part to run the System Loader
    * instead of your broken code. When done disconnect BOOT0, and start over.
    */
    //QV_CPU_SLEEP();  /* atomically go to sleep and enable interrupts */
    QF_INT_ENABLE(); /* for now, just enable interrupts */
#else
    QF_INT_ENABLE(); /* just enable interrupts */
#endif
}
/*..........................................................................*/
Q_NORETURN Q_onAssert(char const *module, int_t loc) {
    /*
     * NOTE: add here your application-specific error handling
     */
    (void) module;
    (void) loc;
    QS_ASSERTION(module, loc, (uint32_t )10000U); /* report assertion to QS */
    NVIC_SystemReset();
}

#ifdef Q_SPY

/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[1024]; /* buffer for Quantum Spy */
    static uint8_t qsRxBuf[256];  /* buffer for QS-RX channel */

    (void)arg; /* avoid the "unused parameter" compiler warning */

    QS_initBuf(qsBuf, sizeof(qsBuf));
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    /* Init of RTT for QSPYing */
    rtt_init();

    QS_tickPeriod_ = SystemCoreClock / BSP_TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; /* to start the timestamp at zero */

    /* setup the QS filters... */

    return 1U; /* return success */
}

/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) { /* NOTE: invoked with interrupts DISABLED */
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) { /* not set? */
        return QS_tickTime_ - (QSTimeCtr) SysTick->VAL;
    }
    else { /* the rollover occured, but the SysTick_ISR did not run yet */
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr) SysTick->VAL;
    }
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t b;
    QF_INT_DISABLE();
    while ((b = QS_getByte()) != QS_EOD) {
        QF_INT_ENABLE();
        /* busy-wait until TX FIFO empty */
        while (rtt_can_write() == 0)
            ;
        /* put into the FIFO */
        rtt_putchar(b & 0xFF);
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
/*..........................................................................*/
/*! callback function to reset the target (to be implemented in the BSP) */
void QS_onReset(void) {
    NVIC_SystemReset();
}
/*..........................................................................*/
/*! callback function to execute a user command (to be implemented in BSP) */
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3) {
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/

/*****************************************************************************
* NOTE00:
* The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
* ISR priority that is disabled by the QF framework. The value is suitable
* for the NVIC_SetPriority() CMSIS function.
*
* Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
* with the numerical values of priorities equal or higher than
* QF_AWARE_ISR_CMSIS_PRI) are allowed to call any QF services. These ISRs
* are "QF-aware".
*
* Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
* level (i.e., with the numerical values of priorities less than
* QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
* Such "QF-unaware" ISRs cannot call any QF services. The only mechanism
* by which a "QF-unaware" ISR can communicate with the QF framework is by
* triggering a "QF-aware" ISR, which can post/publish events.
*
* NOTE01:
* The QV_onIdle() callback is called with interrupts disabled, because the
* determination of the idle condition might change by any interrupt posting
* an event. QV_onIdle() must internally enable interrupts, ideally
* atomically with putting the CPU to the power-saving mode.
*/
/*..........................................................................*/
