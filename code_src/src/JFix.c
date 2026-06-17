#include "ch32fun.h"
#include <stdio.h>

// Psx Japanese bios import Fix  for CH32V003 by Carmax91
// Compatible with all type of modchips.
// Beware to use the PSX 3.5V / 3.3V power, *NOT* 5V! The installation pictures include an example.
//
// Only for WCH CH32V003 mcu @ 48Mhz internal clock.
//
// Coded using Ch32fun library! https://github.com/cnlohr/ch32fun
//
// PAL PM-41 consoles BIOS patching NOT reccomended, use OneNee instead.
//
// Status Led otuput meaning:
// - Off: Fix OFF or disabled.
// - ON: Fix triggered, patching.
// - Blinking: Waiting for trigger 
//   After 20 sec. if is not triggered, the Fix will be deactivated automatically (turns OFF),
//   reset the console to reneable (turn ON) again.
//
// Changelog:
// Rev1.0: First rev

// PINOUT for wch ch32v003j4m6:
/*
                   ___ ___
       Led (PD6) -|   U   |- (PD5) (SWIO)
             GND -|       |- Bios_CE (PC4)
      Reset (PA2)-|       |- Bios_D2 (PC2)
             VCC -|       |- Speed (PC1)
                   -------

*/

#define Timeout 20000 // Timeout (ms) time for Jap Fix auto-deactivation

/* Global Variable */
volatile uint32_t systick_millis; 
volatile uint32_t laststate = 0;
volatile uint32_t led = 0;
volatile uint8_t Jfix = 0;

// *****************************************************************************
// Function: systick_init
// DESCRIPTION:
// Initialises the SysTick to trigger an IRQ with auto-reload, using HCLK/1 as
// its clock source
// *****************************************************************************

void systick_init(void)
{
  // Reset any pre-existing configuration
  SysTick->CTLR = 0x0000;

  // Set the compare register to trigger once per millisecond
  SysTick->CMP = DELAY_MS_TIME - 1;

  // Reset the Count Register, and the global millis counter to 0
  SysTick->CNT = 0x00000000;
  systick_millis = 0x00000000;

  // Set the SysTick Configuration
  // NOTE: By not setting SYSTICK_CTLR_STRE, we maintain compatibility with
  // busywait delay funtions used by ch32v003_fun.
  SysTick->CTLR |= SYSTICK_CTLR_STE |  // Enable Counter
                   SYSTICK_CTLR_STIE | // Enable Interrupts
                   SYSTICK_CTLR_STCLK; // Set Clock Source to HCLK/1

  // Enable the SysTick IRQ
  NVIC_EnableIRQ(SysTick_IRQn);
}

// *****************************************************************************
// Function: Blink Led
// DESCRIPTION:
// A fast, badly written but working blink no delay led function :)
// *****************************************************************************

void BlinkLed()
{
  if ((systick_millis - laststate) >= 250)
  {
    laststate = systick_millis;
    switch (led)
    {
    case 0:
      GPIOD->BSHR = (1 << 6); // PD6 -> LED High
      led = 1;
      break;
    case 1:
      GPIOD->BSHR = (1 << 16 + 6); // PD6 -> LED Low
      led = 0;
      break;
    }
  }
}

// *****************************************************************************
// Function: Jap_Fix
// DESCRIPTION:
// Patch the bios D2 signal to fool on the fly the game region identification
//
// - After the console has reconised the game, the drive goes at 2X and
//   SPEED signal becomes HIGH.
// - After that we start to monitoring the Bios_CE signal,
// - When it becomes to go LOW (the console is starting to identify the region
//   of the game) we force Bios_D2 signal LOW for a duration of 900uS.
// - After that we release the Bios_D2 signal and the console check fails 
//   unlocking import booting.
//
// *****************************************************************************
void Jap_Fix()
{
  NVIC_DisableIRQ(SysTick_IRQn); // We don't need the timner anymore.

  funDigitalWrite(PD6, 1); // PD6 -> LED High //Start Fix.

  Delay_Us(100);

  while ((GPIOC->INDR >> (4 & 0xf) & 1))
    ; // while Bios_CE signal is High, Wait and do nothing...

  if (!(GPIOC->INDR >> (4 & 0xf) & 1)) // Bios_CE signal LOW, start patching!
  {
    funPinMode(PC2, GPIO_Speed_30MHz | GPIO_CNF_OUT_PP); // PC2 -> Bios_D2 Output.
    funDigitalWrite(PC2, 0);                             // Drags line LOW.
  }

  Delay_Us(900);

  funPinMode(PC2, GPIO_CNF_IN_FLOATING); // PC2 <- Bios_D2 Input, release the pin

  Jfix = 1;                // Jap Fix done, store the var.
  funDigitalWrite(PD6, 0); // PD6 -> LED Low //Fix done!
}

int main(void)
{
  SystemInit();

  // Enable GPIOs 
  funGpioInitAll();

  funPinMode(PA2, GPIO_CNF_IN_FLOATING);               // PA2 <- Reset Input
  funPinMode(PC1, GPIO_CNF_IN_FLOATING);               // PC1 <- Speed Input
  funPinMode(PC2, GPIO_CNF_IN_FLOATING);               // PC2 <- Bios_D2 Input
  funPinMode(PC4, GPIO_CNF_IN_FLOATING);               // PC4 <- Bios_CE Input
  funPinMode(PD6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP); // PD6 -> LED Output

  Delay_Ms(500); // Startup debounce delay.

  systick_init(); // Start the timer.

  // ISR on Reset to reset the patch
  // Falling Edge console reset signal ISR on PA2
  AFIO->EXTICR = AFIO_EXTICR_EXTI2_PA;
  EXTI->INTENR = EXTI_INTENR_MR2; // Enable EXT2
  EXTI->FTENR = EXTI_FTENR_TR2;   // Falling edge trigger
  // enable interrupt
  NVIC_EnableIRQ(EXTI7_0_IRQn);

  while (1)
  {
    if (!Jfix) // while Jfix var is 0
    {
      while ((funDigitalRead(PC1)) == 0) // While Speed signal is LOW
      {
        BlinkLed(); // Wait for the trigger blinkng.
      }

      if (((funDigitalRead(PC1)) == 1) && (!Jfix)) // Speed == 1 && Jfix == 0, Triggered!; Start Bios Fix!
      {
        Jap_Fix();
      }
    }

    while (Jfix) // After patched the bios or timed out...
    {
      funDigitalWrite(PD6, 0); // PD6 -> LED Low
    }
  }
}

// ISR (Interrupt Service Routine)
void EXTI7_0_IRQHandler(void) __attribute__((interrupt));
void EXTI7_0_IRQHandler(void)
{
  Delay_Ms(500); // Debounce delay
  // Resets all the var.
  Jfix = 0;
  led = 0;
  systick_millis = 0;
  laststate = 0;

  systick_init(); // Re-enable timer

  funDigitalWrite(PD6, 0);  // Force PD6 -> LED Low
  EXTI->INTFR = EXTI_Line2; // Ack ISR
  // NVIC_DisableIRQ(EXTI7_0_IRQn);
}

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
  // Increment the Compare Register for the next trigger
  // If more than this number of ticks elapse before the trigger is reset,
  // you may miss your next interrupt trigger
  // (Make sure the IQR is lightweight and CMP value is reasonable)
  SysTick->CMP += DELAY_MS_TIME;

  // Clear the trigger state for the next IRQ
  SysTick->SR = 0x00000000;

  // Increment the milliseconds count
  systick_millis++;

  if (systick_millis > Timeout) // 20 sec. Timeout
  {
    Jfix = 1;                      // Force Jfix var. to done
    funDigitalWrite(PD6, 0);       // Force PD6 -> LED Low
    NVIC_DisableIRQ(SysTick_IRQn); // Disable Timer
  }
}