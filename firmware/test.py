import usb, iox32a4u

dev = usb.core.find(idVendor=0x59e3, idProduct=0xf000)

def bigWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x16, value, addr, 0)

def smallWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x08, value, addr, 0)

def bigRead(addr):
	res = dev.ctrl_transfer(0x40|0x80, 0x17, 0, addr, 2)
	return (res[1] << 8) | res[0]

def smallRead(addr):
	return dev.ctrl_transfer(0x40|0x80, 0x09, 0, addr, 1)

smallWrite(iox32a4u.PORTE_OUTSET, iox32a4u.PIN0_bm)
smallWrite(iox32a4u.TCE0_CTRLA, iox32a4u.TC_CLKSEL_DIV1_gc)
smallWrite(iox32a4u.TCE0_CTRLB, iox32a4u.TC0_CCAEN_bm | iox32a4u.TC_WGMODE_SINGLESLOPE_gc)
bigWrite(iox32a4u.TCE0_CCA, 0x800)
print bigRead(iox32a4u.TCE0_CCA)
