/*
===============================================================================
 Name        : saveg@s.c
 Author      : $(Vasco Corte Real)
 Version     : v0.1
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "board.h"
#include "lcd_st7565s.h"

/* Font header file from SWIM sources*/
#include "lpc_rom8x8.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#if defined(BOARD_NXP_LPCXPRESSO_4337)
	#define _ADC_CHANNLE ADC_CH1
	#define _LPC_ADC_ID LPC_ADC0
	#define _LPC_ADC_IRQ ADC0_IRQn
	#define _GPDMA_CONN_ADC GPDMA_CONN_ADC_0
#else
	#warning "Channel configurations not set for the board!"
#endif

/* ADC Setup structure */
static ADC_CLOCK_SETUP_T ADCSetup;
static volatile uint32_t trimpot_val;
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	ADC0 interrupt handler sub-routine
 * @return	Nothing
 */
void ADC0_IRQHandler(void)
{
	uint16_t dataADC;
	Chip_ADC_ReadValue(_LPC_ADC_ID, _ADC_CHANNLE, &dataADC);
	trimpot_val = dataADC;
}

void SysTick_Handler(void)
{
	Chip_ADC_SetStartMode(_LPC_ADC_ID, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}

static void print_val(void)
{
	char str[4];
	uint32_t val = trimpot_val;
	str[3] = 0;
	str[2] = "0123456789ABCDEF"[val & 0xF];
	str[1] = "0123456789ABCDEF"[(val >> 4) & 0xF];
	str[0] = "0123456789ABCDEF"[(val >> 8) & 0xF];
	LCD_FillRect(52, 26, 52 + 24, 26 + 8, 0);
	LCD_PutStrXY(52, 26, str); /* Print string */
}

/**
 * @brief	main routine for lcd hello world example
 * @return	Function should not exit.
 */
int main(void)
{
	uint32_t prev = 0;
	SystemCoreClockUpdate();
	Board_Init();

	Board_LCD_Init(); /* Pin Mux and Clock initialization */
	LCD_Init(); /* Initialize LCD Device and Turn it ON */
	Board_ADC_Init();

	/*ADC Init */
	Chip_ADC_Init(_LPC_ADC_ID, &ADCSetup);
	Chip_ADC_EnableChannel(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);

	Chip_ADC_SetSampleRate(_LPC_ADC_ID, &ADCSetup, 400000);
	NVIC_EnableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);
	Chip_ADC_SetStartMode(_LPC_ADC_ID, ADC_START_NOW, ADC_TRIGGERMODE_RISING);

	/* Draw a rectangle border */
	LCD_DrawRect(0, 0, LCD_X_RES - 1, LCD_Y_RES - 21, 1);
	LCD_DrawRect(1, 1, LCD_X_RES - 2, LCD_Y_RES - 22, 1);

	/* Set foreground as ON */
	LCD_SetFontColor(1);
	LCD_SetFontBgColor(0); /* Background is OFF */
	LCD_SetFont(&font_rom8x8); /* Use 8x8 Fonts */
	LCD_PutStrXY(13, 10, "TRIM POT DEMO"); /* Print string */

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 10000);

	LCD_FillRect(0, 50, trimpot_val >> 3, 60, 1);
	while (1) {
		uint32_t curr = trimpot_val >> 3;
		if (prev != curr) {
			if (curr < prev) {
				LCD_FillRect(curr, 50, prev, 60, 0);
			} else {
				LCD_FillRect(prev, 50, curr, 60, 1);
			}
			print_val();
			prev = curr;
		}
		__WFI();
	}

	return 0;
}
