import usb

#define TCE0_CTRLA_SFR_MEM8(0x0A00) 
#define TCE0_CTRLB_SFR_MEM8(0x0A01) 
#define TCE0_CTRLC_SFR_MEM8(0x0A02) 
#define TCE0_CTRLD_SFR_MEM8(0x0A03) 
#define TCE0_CTRLE_SFR_MEM8(0x0A04) 
#define TCE0_INTCTRLA_SFR_MEM8(0x0A06)
#define TCE0_INTCTRLB_SFR_MEM8(0x0A07)
#define TCE0_CTRLFCLR_SFR_MEM8(0x0A08)
#define TCE0_CTRLFSET_SFR_MEM8(0x0A09)
#define TCE0_CTRLGCLR_SFR_MEM8(0x0A0A)
#define TCE0_CTRLGSET_SFR_MEM8(0x0A0B)
#define TCE0_INTFLAGS_SFR_MEM8(0x0A0C)
#define TCE0_TEMP_SFR_MEM8(0x0A0F)
#define TCE0_CNT_SFR_MEM16(0x0A20)
#define TCE0_PER_SFR_MEM16(0x0A26)
#define TCE0_CCA_SFR_MEM16(0x0A28)
#define TCE0_CCB_SFR_MEM16(0x0A2A)
#define TCE0_CCC_SFR_MEM16(0x0A2C)
#define TCE0_CCD_SFR_MEM16(0x0A2E)
#define TCE0_PERBUF_SFR_MEM16(0x0A36) 
#define TCE0_CCABUF_SFR_MEM16(0x0A38) 
#define TCE0_CCBBUF_SFR_MEM16(0x0A3A) 
#define TCE0_CCCBUF_SFR_MEM16(0x0A3C) 
#define TCE0_CCDBUF_SFR_MEM16(0x0A3E)

dev = usb.core.find(idVendor=0x59e3, idProduct=0xf000)

# bRequest 0x16 is a 16b write
# wValue is a 16b value
# wIndex is the 16b addr

def bigWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x16, value, addr, 0)

def smallWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x08, value, addr, 0)

def bigRead(addr):
	res = dev.ctrl_transfer(0x40|0x80, 0x17, 0, addr, 2)
	return (res[1] << 8) | res[0]

def smallRead(addr):
	return dev.ctrl_transfer(0x40|0x80, 0x09, 0, addr, 1)

bigWrite(0x0681, 0x01) # PORTE_OUTSET 1
smallWrite(0x0a00, 0x01) # TCE0_CTRLA TC_CLKSEL_DIV1_gc
smallWrite(0x0a01, 0x13) # TCE0_CTRLB CCAEN, SINGLESLOPE
bigWrite(0x0a28, 0x8000) # 50% duty cycle
print bigRead(0x0a28)
