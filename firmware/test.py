import usb
import iox32a4u as x

dev = usb.core.find(idVendor=0x59e3, idProduct=0xf000)

def bigWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x16, value, addr, 0)

def smallWrite(addr, value):
	dev.ctrl_transfer(0x40|0x80, 0x08, value, addr, 0)

def bigRead(addr):
	res = dev.ctrl_transfer(0x40|0x80, 0x17, 0, addr, 2)
	return (res[1] << 8) | res[0]

def smallRead(addr):
	return dev.ctrl_transfer(0x40|0x80, 0x09, 0, addr, 1)[0]

smallWrite(x.PORTE_DIRSET, x.PIN0_bm | x.PIN1_bm)
smallWrite(x.TCE0_CTRLA, x.TC_CLKSEL_DIV1024_gc)
smallWrite(x.TCE0_CTRLB, x.TC0_CCAEN_bm | x.TC0_CCBEN_bm | x.TC_WGMODE_SINGLESLOPE_gc)
bigWrite(x.TCE0_CCA, 0x10)
bigWrite(x.TCE0_CCB, 0x10)
print bigRead(x.TCE0_CCA)

while True:
	print bigRead(x.TCE0_CNT)
