# STM32F446RE LED Blink Speed Control (Bare-Metal)

## Project Description

This project demonstrates a **bare-metal embedded C** implementation on the **STM32F446RE Nucleo board**, where a user can cycle through **three different LED blink speeds** using a push-button interrupt. It avoids HAL libraries and directly configures registers using the STM32 reference manual.

## Key Features

- **GPIO Configuration**:  
  - PA5 is configured as output (onboard LED).
  - PC13 is configured as input with pull-up (user push-button).

- **SysTick Timer**:  
  - Generates periodic interrupts to toggle the LED.

- **External Interrupt (EXTI13)**:  
  - Triggered on the falling edge of the button press.
  - Cycles LED blink speed between Fast (~250ms), Medium (~500ms), and Slow (~1000ms).

- **Speed Control Logic**:  
  - Controlled via a global `speed` variable and dynamic reload of the SysTick timer.

## Peripherals Used
| Peripheral | Usage |
|-----------|-------|
| GPIOA     | LED Output (PA5) |
| GPIOC     | Button Input (PC13) |
| EXTI13    | External interrupt on falling edge |
| SYSCFG    | To map EXTI13 to PC13 |
| SysTick   | Periodic timer interrupt for LED toggling |

## Tools & Environment
- STM32F446RE Nucleo board  
- STM32CubeIDE (for initial development)  
- Compiled using `arm-none-eabi-gcc` (can be ported to VS Code + Makefile setup)  
- No use of HAL or CMSIS libraries — 100% register-level programming

## How to Run
1. Flash the code to your STM32F446RE board using STM32CubeIDE or `st-flash`.
2. Press the button (PC13) to change the LED blink speed.
3. Observe LED on PA5 toggling at different rates:
   - Initial Speed: Fast (~250ms) 
   - Speed 1: Fast (~250ms)
   - Speed 2: Medium ~(500ms)
   - speed 3: Slow (~1000ms)

## Skills Demonstrated
- Bare-metal register configuration
- Interrupt-based event handling
- SysTick timer setup and reload logic
- Modular C programming with clear function abstractions
- Embedded system design patterns

## Future Enhancements
- Software button debouncing
- Add UART output to display current blink mode
- Use hardware timers (TIMx) instead of SysTick
- Add FreeRTOS for task-based design

---

This project serves as a foundation for developing professional embedded firmware, and is a great showcase of low-level STM32 development without relying on abstraction layers like HAL.