#include <stdint.h>
#include <stdio.h>

#include "interrupt.h"
#include "pwr_clk_mgmt.h"
#include "gpio.h"
#include "rtc2.h"
#include "uart.h"

uint8_t ir_state;

interrupt_isr_rtc2()
{
	switch (ir_state) {
		case 1: ir_state = 2; break;
		case 2: ir_state = 0; break;
	}
}

void main()
{
	bool last_on = 1;
	uint16_t last_time = 0;

	// Setup pin P0.0 for led
	gpio_pin_configure(GPIO_PIN_ID_P0_0,
			GPIO_PIN_CONFIG_OPTION_DIR_INPUT);

	// Setup UART pins
	gpio_pin_configure(GPIO_PIN_ID_FUNC_RXD,
			GPIO_PIN_CONFIG_OPTION_DIR_INPUT |
			GPIO_PIN_CONFIG_OPTION_PIN_MODE_INPUT_BUFFER_ON_NO_RESISTORS);

	gpio_pin_configure(GPIO_PIN_ID_FUNC_TXD,
			GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT |
			GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_SET |
			GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH);

	uart_configure_8_n_1_38400();
	uart_rx_disable();

	pwr_clk_mgmt_clklf_configure(PWR_CLK_MGMT_CLKLF_CONFIG_OPTION_CLK_SRC_RCOSC32K);
	pwr_clk_mgmt_wait_until_clklf_is_ready();
	rtc2_configure(RTC2_CONFIG_OPTION_COMPARE_MODE_1_WRAP_AT_IRQ, 3276);
	interrupt_control_rtc2_enable();
	interrupt_control_global_enable();
	rtc2_run();

	puts("Starting up\r\n");

	while(1)
	{
		bool val = gpio_pin_val_read(GPIO_PIN_ID_P0_0);
		if (val != last_on) {
			uint8_t now_low;
			uint8_t now_high;
			uint16_t now;
			uint16_t diff;

			rtc2_sfr_capture();
			interrupt_control_global_disable();
			now_low = RTC2CPT00;
			now_high = RTC2CPT01;
			interrupt_control_global_enable();
			now = (uint16_t)now_low | (((uint16_t)now_high) << 8);
			diff = (now - last_time) / 3;
		
			if (ir_state == 0 || diff > 1000) {
				printf("\r\n");
			}
			if (val) {
				printf("1 %u\r\n", diff);
				last_on = 1;
			} else {
				printf("0 %u\r\n", diff);
				last_on = 0;
			}

			ir_state = 1;
			last_time = now;
		}
	}
}
