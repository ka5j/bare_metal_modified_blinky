/* 
 * Project: LED Blink Speed Control using Button
 * Target: STM32F446RE (ARM Cortex-M4)
 * Author: ka5j 
 * Description:
 *  - Blink an LED connected to PA5 at three different speeds.
 *  - Cycle between speeds using a button connected to PC13 (EXTI13 interrupt).
 *  - Blinking handled by SysTick timer.
 */

 #include <stdint.h>

 /*----------------------------- Macros & Constants -----------------------------*/
 
 #define HIGH_SPEED     4000000   // Fast blink (~250ms delay)
 #define MEDIUM_SPEED   8000000   // Medium blink (~500ms delay)
 #define LOW_SPEED     16000000   // Slow blink (~1s delay)
 
 #define RCC_BASE        0x40023800
 #define SYSTICK_BASE    0xE000E010
 #define SYSCFG_BASE     0x40013800
 #define EXTI_BASE       0x40013C00
 #define NVIC_BASE       0xE000E100
 #define GPIOA_BASE      0x40020000
 #define GPIOC_BASE      0x40020800
 
 // RCC registers
 #define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))
 #define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x44))
 
 // SysTick registers
 #define SYSTICK_CTRL    (*(volatile uint32_t*)(SYSTICK_BASE + 0x00))
 #define SYSTICK_RELOAD  (*(volatile uint32_t*)(SYSTICK_BASE + 0x04))
 #define SYSTICK_CURRENT (*(volatile uint32_t*)(SYSTICK_BASE + 0x08))
 
 // SYSCFG EXTI configuration
 #define SYSCFG_EXTICR4  (*(volatile uint32_t*)(SYSCFG_BASE + 0x14))
 
 // EXTI registers
 #define EXTI_IMR        (*(volatile uint32_t*)(EXTI_BASE + 0x00))
 #define EXTI_EMR        (*(volatile uint32_t*)(EXTI_BASE + 0x04))  // Not used, keep masked
 #define EXTI_RTSR       (*(volatile uint32_t*)(EXTI_BASE + 0x08))
 #define EXTI_FTSR       (*(volatile uint32_t*)(EXTI_BASE + 0x0C))
 #define EXTI_PR         (*(volatile uint32_t*)(EXTI_BASE + 0x14))
 
 // NVIC register for enabling EXTI15_10 interrupt
 #define NVIC_ISER1      (*(volatile uint32_t*)(NVIC_BASE + 0x04))
 
 // GPIOA registers (LED output)
 #define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
 #define GPIOA_OTYPER    (*(volatile uint32_t*)(GPIOA_BASE + 0x04))
 #define GPIOA_OSPEEDER  (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
 #define GPIOA_PUPDR     (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))
 #define GPIOA_ODR       (*(volatile uint32_t*)(GPIOA_BASE + 0x14))
 
 // GPIOC registers (button input)
 #define GPIOC_MODER     (*(volatile uint32_t*)(GPIOC_BASE + 0x00))
 #define GPIOC_PUPDR     (*(volatile uint32_t*)(GPIOC_BASE + 0x0C))
 
 /*----------------------------- Global Variables -----------------------------*/
 
 volatile uint8_t speed = 1;  // Current blink speed state (1 to 3)
 
 /*----------------------------- Function Prototypes -----------------------------*/
 
 void RCC_Init(void);
 void GPIOA_Init(void);
 void GPIOC_Init(void);
 void SysTick_Init(uint32_t ticks);
 void SysTick_Set_Counter(uint32_t ticks);
 void EXTI13_Init(void);
 
 /*----------------------------- Main Function -----------------------------*/
 
 int main(void)
 {
	 RCC_Init();         // Enable clocks for GPIOA, GPIOC, and SYSCFG
	 GPIOA_Init();       // Configure PA5 as output (LED)
	 GPIOC_Init();       // Configure PC13 as input (button)
	 SysTick_Init(HIGH_SPEED); // Start with fast blink
	 EXTI13_Init();      // Configure external interrupt for PC13 (EXTI13)
 
	 while (1);          // All logic handled in SysTick and EXTI ISRs
 }
 
 /*----------------------------- Peripheral Initialization -----------------------------*/
 
 // Enable peripheral clocks
 void RCC_Init(void)
 {
	 RCC_AHB1ENR |= (1 << 0);   // GPIOA clock enable
	 RCC_AHB1ENR |= (1 << 2);   // GPIOC clock enable
	 RCC_APB2ENR |= (1 << 14);  // SYSCFG clock enable
 }
 
 // Configure PA5 as push-pull output with no pull-up/down
 void GPIOA_Init(void)
 {
	 GPIOA_MODER &= ~(3 << (5 * 2));  // Clear mode
	 GPIOA_MODER |=  (1 << (5 * 2));  // Set as output
	 GPIOA_OTYPER &= ~(1 << 5);       // Push-pull
	 GPIOA_OSPEEDER &= ~(3 << (5 * 2)); // Low speed
	 GPIOA_PUPDR &= ~(3 << (5 * 2));  // No pull-up/down
 }
 
 // Configure PC13 as input with pull-up
 void GPIOC_Init(void)
 {
	 GPIOC_MODER &= ~(3 << (13 * 2));     // Set as input
	 GPIOC_PUPDR &= ~(3 << (13 * 2));     // Clear first
	 GPIOC_PUPDR |=  (1 << (13 * 2));     // Enable pull-up
 }
 
 // Configure SysTick for periodic interrupts
 void SysTick_Init(uint32_t ticks)
 {
	 SysTick_Set_Counter(ticks);
	 SYSTICK_CTRL |= (1 << 2);   // Use processor clock
	 SYSTICK_CTRL |= (1 << 1);   // Enable SysTick interrupt
	 SYSTICK_CTRL |= (1 << 0);   // Enable SysTick
 }
 
 // Update SysTick reload value
 void SysTick_Set_Counter(uint32_t ticks)
 {
	 SYSTICK_RELOAD = ticks - 1;
	 SYSTICK_CURRENT = 0;        // Reset counter
 }
 
 // Configure EXTI13 to trigger on falling edge of PC13 (button press)
 void EXTI13_Init(void)
 {
	 NVIC_ISER1 |= (1 << (40 - 32));       // Enable IRQ40 (EXTI15_10)
 
	 SYSCFG_EXTICR4 &= ~(0xF << 4);        // Clear EXTI13 config
	 SYSCFG_EXTICR4 |= (0x2 << 4);         // Set EXTI13 to Port C
 
	 EXTI_EMR &= ~(1 << 13);               // Mask event request
	 EXTI_RTSR &= ~(1 << 13);              // Disable rising edge
	 EXTI_FTSR |= (1 << 13);               // Enable falling edge
	 EXTI_IMR |= (1 << 13);                // Unmask EXTI13 line
 }
 
 /*----------------------------- Interrupt Service Routines -----------------------------*/
 
 // Toggle LED on SysTick timer interrupt
 void SysTick_Handler(void)
 {
	 GPIOA_ODR ^= (1 << 5);    // Toggle PA5
 }
 
 // Handle button press and cycle blink speed
 void EXTI15_10_IRQHandler(void)
 {
	 EXTI_PR |= (1 << 13);     // Clear pending interrupt
 
	 speed = (speed % 3) + 1;  // Cycle through 1-3
 
	 switch(speed)
	 {
		 case 1:
			 SysTick_Set_Counter(HIGH_SPEED);
			 break;
		 case 2:
			 SysTick_Set_Counter(MEDIUM_SPEED);
			 break;
		 case 3:
			 SysTick_Set_Counter(LOW_SPEED);
			 break;
	 }
 }
 