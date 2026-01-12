extern volatile unsigned char  _sbss[], _ebss [], _etext[] ;
extern volatile unsigned char   _sdata[] , _edata[];
extern volatile unsigned long  _estack[];

extern int main(void);
extern void sys_clock_init(void);

/* start of vector table handlers prototype */
void Default_Handler()				__attribute__((interrupt))  __attribute__((weak));
void Reset_Handler()				__attribute__((interrupt))  __attribute__((weak));
void NMI_Handler()					__attribute__((interrupt))  __attribute__((weak));
void HardFault_Handler()			__attribute__((interrupt))  __attribute__((weak));
void MemManage_Handler()			__attribute__((interrupt))  __attribute__((weak));
void BusFault_Handler()				__attribute__((interrupt))  __attribute__((weak));
void UsageFault_Handler()			__attribute__((interrupt))  __attribute__((weak));
void SVC_Handler()					__attribute__((interrupt))  __attribute__((weak));
void DebugMon_Handler()				__attribute__((interrupt))  __attribute__((weak));
void PendSV_Handler()				__attribute__((interrupt))  __attribute__((weak));
void SysTick_Handler()				__attribute__((interrupt))  __attribute__((weak));
void WWDG_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler()	   		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USB_HP_CAN1_TX_IRQHandler() 	__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USB_LP_CAN1_RX0_IRQHandler()	__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler() 			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler()		__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler()				__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void USBWakeUp_IRQHandler()			__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
void BootRAM()						__attribute__((interrupt))  __attribute__((weak, alias("Default_Handler")));
/* end of vector table handlers prototype*/

__attribute__((section(".isr_vector"))) \
void (* const vectorTable[] ) (void) = \
{
/*assign the SP with _estack == ldr sp, =_estack*/  	
 (void (* const)(void) ) &_estack,
 Reset_Handler,
 NMI_Handler,
 HardFault_Handler,
 MemManage_Handler,
 BusFault_Handler,
 UsageFault_Handler,
 0,
 0,
 0,
 0,
 SVC_Handler,
 DebugMon_Handler,
 0,
 PendSV_Handler,
 SysTick_Handler,
 WWDG_IRQHandler,
 PVD_IRQHandler,
 TAMPER_IRQHandler,
 RTC_IRQHandler,
 FLASH_IRQHandler,
 RCC_IRQHandler,
 EXTI0_IRQHandler,
 EXTI1_IRQHandler,
 EXTI2_IRQHandler,
 EXTI3_IRQHandler,
 EXTI4_IRQHandler,
 DMA1_Channel1_IRQHandler,
 DMA1_Channel2_IRQHandler,
 DMA1_Channel3_IRQHandler,
 DMA1_Channel4_IRQHandler,
 DMA1_Channel5_IRQHandler,
 DMA1_Channel6_IRQHandler,
 DMA1_Channel7_IRQHandler,
 ADC1_2_IRQHandler,
 USB_HP_CAN1_TX_IRQHandler,
 USB_LP_CAN1_RX0_IRQHandler,
 CAN1_RX1_IRQHandler,
 CAN1_SCE_IRQHandler,
 EXTI9_5_IRQHandler,
 TIM1_BRK_IRQHandler,
 TIM1_UP_IRQHandler,
 TIM1_TRG_COM_IRQHandler,
 TIM1_CC_IRQHandler,
 TIM2_IRQHandler,
 TIM3_IRQHandler,
 TIM4_IRQHandler,
 I2C1_EV_IRQHandler,
 I2C1_ER_IRQHandler,
 I2C2_EV_IRQHandler,
 I2C2_ER_IRQHandler,
 SPI1_IRQHandler,
 SPI2_IRQHandler,
 USART1_IRQHandler,
 USART2_IRQHandler,
 USART3_IRQHandler,
 EXTI15_10_IRQHandler,
 RTC_Alarm_IRQHandler,
 USBWakeUp_IRQHandler,
 0,
 0,
 0,
 0,
 0,
 0,
 0,
 BootRAM
};

__attribute__((noreturn)) \
void Reset_Handler(){
    volatile unsigned long *src, *dst;
    volatile unsigned char *byte_src, *byte_dst;
    // Copy initialized data from Flash to RAM (optimize for large chunks)
    src = (unsigned long *) &_etext;
    dst = (unsigned long *) &_sdata;
    while (dst + 1 <= (unsigned long *) &_edata) {  *dst++ = *src++;  }
    // Handle remaining bytes (if the section size isn't a multiple of word size)
    byte_src = (unsigned char *) src;
    byte_dst = (unsigned char *) dst;
    while (byte_dst < (unsigned char *) &_edata) { *byte_dst++ = *byte_src++;  }
    // Clear the .bss section (optimize for large chunks)
    dst = (unsigned long *) &_sbss;
    while (dst + 1 <= (unsigned long *) &_ebss) { *dst++ = 0; }
    // Handle remaining bytes in .bss
    byte_dst = (unsigned char *) dst;
    while (byte_dst < (unsigned char *) &_ebss) { *byte_dst++ = 0; }

    sys_clock_init();
    main();

    /*if main returns then go to infinite loop*/
    for(;;);
}


void Default_Handler() { for(;;); }
void NMI_Handler() { for(;;); }
void HardFault_Handler() { for(;;); }
void MemManage_Handler() { for(;;); }
void BusFault_Handler() { for(;;); }
void UsageFault_Handler(){ for(;;); }
void SVC_Handler() { for(;;); }
void DebugMon_Handler() { for(;;); }
void PendSV_Handler() { for(;;); }
void SysTick_Handler() {
    volatile unsigned * const ODR = (unsigned*)0x4001100c; 
    *ODR ^= (1UL << 13);
}
