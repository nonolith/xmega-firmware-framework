#include "Framework.h"

int8_t CH0Data[1024];
int8_t CH1Data[1024];

int main(void){
	USB_ConfigureClock();
	USB_Init();
	USB.INTCTRLA = USB_BUSEVIE_bm | USB_INTLVL_MED_gc;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

	PORTE.DIRSET = 0b1;
	TCE0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCE0.CCA = 1000;
	TCE0.PER = 0;
	TCE0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;

	// config PWM for complementary active-high outputs, with dead-time
	PORTC.DIRSET = 0b11;
	PORTC.PIN0CTRL = PORT_INVEN_bm;
	PORTC.PIN1CTRL = PORT_INVEN_bm;
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;  
	TCC0.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCC0.CCA = 0;
	TCC0.PER = 0;

	// deadtime = zero with IRS25606
	AWEXC.CTRL= AWEX_DTICCAEN_bm;
	AWEXC.OUTOVEN = 0b11;
	AWEXC.DTBOTH = 0;

	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;

	// 8b, 1.65vref, diff w/ gain
	ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc | 1 << ADC_CONMODE_bp | 0 << ADC_IMPMODE_bp | ADC_CURRLIMIT_NO_gc;
	ADCA.REFCTRL = ADC_REFSEL_VCCDIV2_gc;
	// 8 MHz master clock, 1 MHz sample rate
	ADCA.PRESCALER = ADC_PRESCALER_DIV4_gc;
	// synchronous sweep triggered off TCC0.CCA
	ADCA.EVCTRL = ADC_SWEEP_01_gc | ADC_EVACT_SYNCHSWEEP_gc | ADC_EVSEL_7_gc; 
	EVSYS.CH7MUX = EVSYS_CHMUX_TCC0_CCA_gc;
	// with gain
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN5_gc | ADC_CH_MUXPOS_PIN0_gc;
	ADCA.CH1.MUXCTRL = ADC_CH_MUXNEG_PIN4_gc | ADC_CH_MUXPOS_PIN1_gc;

	NVM.CMD  = NVM_CMD_READ_CALIB_ROW_gc;

	ADCA.CALL = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0)); 
	ADCA.CALH = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));

	// enable ADC
	ADCA.CTRLA = ADC_ENABLE_bm; 

	// use DMA CH0 for ADC CH0 and DMA CH1 for ADC CH1
	DMA.CTRL = DMA_ENABLE_bm | DMA_DBUFMODE_DISABLED_gc | DMA_PRIMODE_RR0123_gc; 
	DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_NONE_gc | DMA_CH_SRCDIR_FIXED_gc | DMA_CH_DESTRELOAD_TRANSACTION_gc | DMA_CH_DESTDIR_INC_gc; 
	DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_NONE_gc | DMA_CH_SRCDIR_FIXED_gc | DMA_CH_DESTRELOAD_TRANSACTION_gc | DMA_CH_DESTDIR_INC_gc; 
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH0_gc; 
	DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH1_gc; 
	DMA.CH0.TRFCNT = 1024;
	DMA.CH1.TRFCNT = 1024;
	DMA.CH0.REPCNT = 0; 
	DMA.CH1.REPCNT = 0; 

	DMA.CH0.SRCADDR0 = ((uint32_t)(&ADCA.CH0RES) >> (8*0)) & 0xFF; 
	DMA.CH0.SRCADDR1 = ((uint32_t)(&ADCA.CH0RES) >> (8*1)) & 0xFF; 
	DMA.CH0.SRCADDR2 = ((uint32_t)(&ADCA.CH0RES) >> (8*2)) & 0xFF; 
	DMA.CH1.SRCADDR0 = ((uint32_t)(&ADCA.CH1RES) >> (8*0)) & 0xFF; 
	DMA.CH1.SRCADDR1 = ((uint32_t)(&ADCA.CH1RES) >> (8*1)) & 0xFF; 
	DMA.CH1.SRCADDR2 = ((uint32_t)(&ADCA.CH1RES) >> (8*2)) & 0xFF; 

	DMA.CH0.DESTADDR0 = ((uint32_t)(&CH0Data) >> (8*0)) & 0xFF; 
	DMA.CH0.DESTADDR1 = ((uint32_t)(&CH0Data) >> (8*1)) & 0xFF; 
	DMA.CH0.DESTADDR2 = ((uint32_t)(&CH0Data) >> (8*2)) & 0xFF; 
	DMA.CH1.DESTADDR0 = ((uint32_t)(&CH1Data) >> (8*0)) & 0xFF; 
	DMA.CH1.DESTADDR0 = ((uint32_t)(&CH1Data) >> (8*1)) & 0xFF; 
	DMA.CH1.DESTADDR0 = ((uint32_t)(&CH1Data) >> (8*2)) & 0xFF; 

	DMA.CH0.CTRLA = DMA_CH_ENABLE_bm | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	DMA.CH1.CTRLA = DMA_CH_ENABLE_bm | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	
	for (;;){
	}

}

#define stringify(s) #s

const char PROGMEM hwversion[] = stringify(HW_VERSION);
const char PROGMEM fwversion[] = stringify(FW_VERSION);

uint8_t usb_cmd = 0;
uint8_t cmd_data = 0;

/** Event handler for the library USB Control Request reception event. */
bool EVENT_USB_Device_ControlRequest(USB_Request_Header_t* req){
	// zero out ep0_buf_in
	for (uint8_t i = 0; i < 64; i++) ep0_buf_in[i] = 0;
	usb_cmd = 0;
	if ((req->bmRequestType & CONTROL_REQTYPE_TYPE) == REQTYPE_VENDOR){
		switch(req->bRequest){
			case 0x00: // Info
				if (req->wIndex == 0){
					USB_ep0_send_progmem((uint8_t*)hwversion, sizeof(hwversion));
				}else if (req->wIndex == 1){
					USB_ep0_send_progmem((uint8_t*)fwversion, sizeof(fwversion));
				}
				return true;
			case 0x08:
				* ((uint8_t *) req->wIndex) = req->wValue;
				USB_ep0_send(0);
				return true;
			case 0x09:
				ep0_buf_in[0] = * ((uint8_t *) req->wIndex);
				USB_ep0_send(1);
				return true;
			case 0x16:
				* ((uint16_t *) req->wIndex) = req->wValue;
				USB_ep0_send(0);
				return true;
			case 0x17:{
				uint16_t *addr;
				addr = (uint16_t *) req->wIndex;
				ep0_buf_in[0] = *addr & 0xFF;
				ep0_buf_in[1] = *addr >> 8;
				USB_ep0_send(2);}
				return true;
			// read EEPROM	
			case 0xE0: 
				eeprom_read_block(ep0_buf_in, (void*)(req->wIndex*64), 64);
				USB_ep0_send(64);
				return true;

			case 0xC0:
				for (int8_t i = 0; i < 1024; i++) send_byte(CH0Data[i]);
				break_and_flush();
				USB_ep0_send(0);
				return true;
			case 0xC1:
				for (int8_t i = 0; i < 1024; i++) send_byte(CH1Data[i]);
				break_and_flush();
				USB_ep0_send(0);
				return true;
	
			// write EEPROM	
			case 0xE1: 
				usb_cmd = req->bRequest;
				cmd_data = req->wIndex;
				USB_ep0_send(0);
				return true; // Wait for OUT data (expecting an OUT transfer)

			// disconnect from USB, jump to bootloader	
			case 0xBB: 
				USB_enter_bootloader();
				return true;
		}
	}
	return false;
}

void EVENT_USB_Device_ControlOUT(uint8_t* buf, uint8_t count){
	switch (usb_cmd){
		case 0xE1: // Write EEPROM
			eeprom_update_block(buf, (void*)(cmd_data*64), count);
			break;
	}
}
