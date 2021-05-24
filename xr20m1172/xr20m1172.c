/*
 * Based on linux/drivers/serial/pxa.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
//#include <asm/arch/ssp.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/serial_reg.h>
#include <linux/circ_buf.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <asm/io.h>
//#include <asm/hardware.h>
#include <asm/irq.h>
//#include <asm/arch/pxa-regs.h>
#ifdef CONFIG_DVFM
#include <asm/arch/cpu-freq-voltage-pxa3xx.h>
#endif
#ifdef CONFIG_IPM
#include <asm/arch/ipmc.h>
#endif

#include "xr20m1172.h"

//static struct ssp_dev ssp1dev;
//#define spi_trace pr_info
//#define spi_trace pr_err

#define MAKE_SPI_NODE_FOR_DEBUG

static u8 cached_lcr[2];
static u8 cached_efr[2];
static u8 cached_mcr[2];

//static spinlock_t xr20m1172_lock = SPIN_LOCK_UNLOCKED;
//static DEFINE_SPINLOCK(xr20m1172_lock);
//static unsigned long xr20m1172_flags;
struct spidev_data	*spi_xr20m1172_data;

#ifdef MAKE_SPI_NODE_FOR_DEBUG
static void create_spi_class(struct spidev_data	*spidev);
static void clear_spi_class(struct spi_device *spi);
static int register_spi_char(void);
#endif

#ifdef DEBUG_VIA_PROC_FS
static struct proc_dir_entry *xr20m_proc_file;

static int xr20m_seq_open(struct inode *inode, struct file *file);
static struct file_operations xr20m_seq_fops = {
	.owner = THIS_MODULE,
	.open = xr20m_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int xr20m_seq_show(struct seq_file *s, void *p)
{
	int i;
	int reg;
	for (i = 0; i < 27; i++) {
		//serial_out(0, i, 0x20+i);
		reg = serial_in(0, i);
		seq_printf(s, "XM0: reg i:%d, value:0x%x\n", i, reg);
	}

	for (i = 0; i < 27; i++) {
		reg = serial_in(1, i);
		seq_printf(s, "XM1: reg i:%d, value:0x%x\n", i, reg);
	}

	return 0;
}

static int xr20m_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, &xr20m_seq_show, NULL);
}

static int create_xr20m_proc_file(void)
{
	//xr20m_proc_file = create_proc_entry(XR20M_PROC_FILE, 0644, NULL);
	xr20m_proc_file = proc_create(XR20M_PROC_FILE, 0644, NULL, &xr20m_seq_fops);
	if (!xr20m_proc_file) {
		printk(KERN_INFO "Create proc file for Arava failed\n");
		return -ENOMEM;
	}

	//xr20m_proc_file->proc_fops = &xr20m_seq_fops;
	return 0;
}

static void remove_xr20m_proc_file(void)
{
	//remove_proc_entry(XR20M_PROC_FILE, &proc_root);
	remove_proc_entry(XR20M_PROC_FILE, NULL);
}
#endif

/* 
 * meaning of the pair:
 * first: the subaddress (physical offset<<3) of the register
 * second: the access constraint:
 * 10: no constraint
 * 20: lcr[7] == 0
 * 30: lcr == 0xbf
 * 40: lcr != 0xbf
 * 50: lcr[7] == 1 , lcr != 0xbf, efr[4] = 1
 * 60: lcr[7] == 1 , lcr != 0xbf,
 * 70: lcr != 0xbf,  and (efr[4] == 0 or efr[4] =1, mcr[2] = 0)
 * 80: lcr != 0xbf,  and (efr[4] = 1, mcr[2] = 1)
 * 90: lcr[7] == 0, efr[4] =1 
 * 100: lcr!= 0xbf, efr[4] =1
 * third:  1: readonly
 * 2: writeonly
 * 3: read/write
 */
static const int reg_info[27][3] = {
	{0x0, 20, 1},		//RHR
	{0x0, 20, 2},		//THR
	{0x0, 60, 3},		//DLL
	{0x8, 60, 3},		//DLM
	{0x10, 50, 3},		//DLD
	{0x8, 20, 3},		//IER:bit[4-7] needs efr[4] ==1,but we dont' access them now
	{0x10, 20, 1},		//ISR:bit[4/5] needs efr[4] ==1,but we dont' access them now
	{0x10, 20, 2},		//FCR :bit[4/5] needs efr[4] ==1,but we dont' access them now
	{0x18, 10, 3},		//LCR
	{0x20, 40, 3},		//MCR :bit[2/5/6] needs efr[4] ==1,but we dont' access them now
	{0x28, 40, 1},		//LSR
	{0x30, 70, 1},		//MSR
	{0x38, 70, 3},		//SPR
	{0x30, 80, 3},		//TCR
	{0x38, 80, 3},		//TLR
	{0x40, 20, 1},		//TXLVL
	{0x48, 20, 1},		//RXLVL
	{0x50, 20, 3},		//IODir
	{0x58, 20, 3},		//IOState
	{0x60, 20, 3},		//IOIntEna
	{0x70, 20, 3},		//IOControl
	{0x78, 20, 3},		//EFCR
	{0x10, 30, 3},		//EFR
	{0x20, 30, 3},		//Xon1
	{0x28, 30, 3},		//Xon2
	{0x30, 30, 3},		//Xoff1
	{0x38, 30, 3},		//Xoff2
};

/*
void xr20m1172_ssp_init(void)
{
	pxa3xx_enable_ssp1_pins();
	ssp_init(&ssp1dev, 1, 0);
	ssp_disable(&ssp1dev);
	ssp_config(&ssp1dev, 0x0, 0x0, 0x0, 0x800008f);
	//ssp_config(&ssp1dev, 0x0, 0x0, 0x0, 0x8f);
	//ssp_config(&ssp1dev, 0x0, 0x0, 0x0, 0x18f);
	//ssp_config(&ssp1dev, 0x0, 0x0, 0x0, 0xf8f);
	//ssp_config(&ssp1dev, 0x0, 0x0, 0x0, 0x7f8f);
	ssp_enable(&ssp1dev);
	ssp_flush(&ssp1dev);
	mdelay(2);
}

void xr20m1172_ssp_exit(void)
{
	ssp_disable(&ssp1dev);
	ssp_exit(&ssp1dev);
}*/

static void serial_out(unsigned char devid, unsigned char regaddr,
		       unsigned char data)
{
	if (!(reg_info[regaddr][2] & 0x2)) {
		printk("Reg not writeable\n");
		return;
	}

	DMSG("%d, value:0x%x", regaddr, data);
	switch (regaddr) {
	case XR20M1170REG_LCR:
		if (data == cached_lcr[devid])
			return;
		cached_lcr[devid] = data;
		break;
	case XR20M1170REG_EFR:
		if (data == cached_efr[devid])
			return;
		cached_efr[devid] = data;
		break;
	case XR20M1170REG_MCR:
		if (data == cached_mcr[devid])
			return;
		cached_mcr[devid] = data;
		break;
	}

	//spin_lock_irqsave(&xr20m1172_lock, xr20m1172_flags);
	EnterConstraint(devid, regaddr);
	SPI_WriteReg(devid, reg_info[regaddr][0], data);
	ExitConstraint(devid, regaddr);
	//spin_unlock_irqrestore(&xr20m1172_lock, xr20m1172_flags);
}

static unsigned char serial_in(unsigned char devid, unsigned char regaddr)
{
	unsigned char ret;

	if (!(reg_info[regaddr][2] & 0x1)) {
		//printk("Reg not writeable\n");
		return 0;
	}

	switch (regaddr) {
	case XR20M1170REG_LCR:
		ret = cached_lcr[devid];
		break;
	case XR20M1170REG_EFR:
		ret = cached_efr[devid];
		break;
	case XR20M1170REG_MCR:
		ret = cached_mcr[devid];
		break;
	default:
		//spin_lock_irqsave(&xr20m1172_lock, xr20m1172_flags);
		EnterConstraint(devid, regaddr);
		ret = SPI_ReadReg(devid, reg_info[regaddr][0]);
		ExitConstraint(devid, regaddr);
		//spin_unlock_irqrestore(&xr20m1172_lock, xr20m1172_flags);
	}

	//DMSG("%d, value:0x%x", regaddr, ret);
	return ret;
}

static void EnterConstraint(unsigned char devid, unsigned char regaddr)
{
	switch (reg_info[regaddr][1]) {
		//10: no contraint
	case 20:		//20: lcr[7] == 0
		if (cached_lcr[devid] & BIT7)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid] & ~BIT7);
		break;
	case 30:		//30: lcr == 0xbf
		if (cached_lcr[devid] != 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
		break;
	case 40:		//40: lcr != 0xbf
		if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0x3f);
		break;
	case 50:		//50: lcr[7] == 1 , lcr != 0xbf, efr[4] = 1
		if (!(cached_efr[devid] & BIT4)) {
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid] | BIT4);
		}
		if ((cached_lcr[devid] == 0xbf)
		    || (!(cached_lcr[devid] & BIT7)))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     (cached_lcr[devid] | BIT7) & ~BIT0);
		break;
	case 60:		//60: lcr[7] == 1 , lcr != 0xbf,
		if ((cached_lcr[devid] == 0xbf)
		    || (!(cached_lcr[devid] & BIT7)))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     (cached_lcr[devid] | BIT7) & ~BIT0);
		break;
	case 70:		//lcr != 0xbf,  and (efr[4] == 0 or efr[4] =1, mcr[2] = 0)
		if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0x3f);
		if ((cached_efr[devid] & BIT4) && (cached_mcr[devid] & BIT2))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_MCR][0],
				     cached_mcr[devid] & ~BIT2);
		break;
	case 80:		//lcr != 0xbf,  and (efr[4] = 1, mcr[2] = 1)
		if (cached_lcr[devid] != 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
		if (!(cached_efr[devid] & BIT4))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid] | BIT4);
		SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
			     cached_lcr[devid] & ~BIT7);
		if (!(cached_mcr[devid] & BIT2))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_MCR][0],
				     cached_mcr[devid] | BIT2);
		break;
	case 90:		//90: lcr[7] == 0, efr[4] =1 
		if (!(cached_efr[devid] & BIT4)) {
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid] | BIT4);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid] & ~BIT7);
		} else if (cached_lcr[devid] & BIT7)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid] & ~BIT7);
		break;
	case 100:		//100: lcr!= 0xbf, efr[4] =1
		if (!(cached_efr[devid] & BIT4)) {
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid] | BIT4);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0x3f);
		} else if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0x3f);
		break;
	}
}

static void ExitConstraint(unsigned char devid, unsigned char regaddr)
{
	//restore
	switch (reg_info[regaddr][1]) {
		//10: no contraint
	case 20:		//20: lcr[7] == 0
		if (cached_lcr[devid] & BIT7)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 30:		//30: lcr == 0xbf
		if (cached_lcr[devid] != 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 40:		//40: lcr != 0xbf
		if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
		break;
	case 50:		//50: lcr[7] == 1 , lcr != 0xbf, efr[4] = 1
		if ((cached_efr[devid] & BIT4) == 0) {
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid]);
		}
		SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
			     cached_lcr[devid]);
		break;
	case 60:		//60: lcr[7] == 1 , lcr != 0xbf,
		if ((cached_lcr[devid] == 0xbf)
		    || (!(cached_lcr[devid] & BIT7)))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 70:		//lcr != 0xbf,  and (efr[4] == 0 or efr[4] =1, mcr[2] = 0)
		if ((cached_efr[devid] & BIT4) && (cached_mcr[devid] & BIT2))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_MCR][0],
				     cached_mcr[devid]);
		if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 80:		//lcr != 0xbf,  and (efr[4] = 1, mcr[2] = 1)
		if (!(cached_mcr[devid] & BIT2))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_MCR][0],
				     cached_mcr[devid]);
		SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0], 0xbf);
		if (!(cached_efr[devid] & BIT4))
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid]);
		if (cached_lcr[devid] != 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 90:		//90: lcr[7] == 0, efr[4] =1 (for ier bit 4-7)
		if (!(cached_efr[devid] & BIT4)) {
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid]);
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     cached_lcr[devid]);
		} else if (cached_lcr[devid] & BIT7)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     cached_lcr[devid]);
		break;
	case 100:		//100: lcr!= 0xbf, efr[4] =1
		if (!(cached_efr[devid] & BIT4)) {
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
			SPI_WriteReg(devid, reg_info[XR20M1170REG_EFR][0],
				     cached_efr[devid]);
			if (cached_lcr[devid] != 0xbf)
				SPI_WriteReg(devid,
					     reg_info[XR20M1170REG_LCR][0],
					     cached_lcr[devid]);
		} else if (cached_lcr[devid] == 0xbf)
			SPI_WriteReg(devid, reg_info[XR20M1170REG_LCR][0],
				     0xbf);
		break;
	}
}

/*-------------------------------------------------------------------------*/

/*
 * We can't use the standard synchronous wrappers for file I/O; we
 * need to protect against async removal of the underlying spi_device.
 */
spinlock_t spi_xr20m_lock;
struct spi_device *spi_xr20m;
struct mutex    spi_xr20m_buf_lock;
static void spidev_complete(void *arg)
{
    spi_trace("%s\n", __FUNCTION__);
	complete(arg);
}

static ssize_t
spidev_sync(struct spidev_data *spidev, struct spi_message *message)
{
    DECLARE_COMPLETION_ONSTACK(done);
    int status;

    spi_trace("%s\n", __FUNCTION__);

    message->complete = spidev_complete;
    message->context = &done;

    spin_lock_irq(&spidev->spi_lock);
    if (spidev->spi == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_async(spidev->spi, message);

    spin_unlock_irq(&spidev->spi_lock);

    if (status == 0) {
        wait_for_completion(&done);
        status = message->status;
        if (status == 0)
            status = message->actual_length;
    }
    return status;
}

static inline ssize_t
spidev_sync_write(struct spidev_data *spidev, u8 *buf, size_t len)
{    
	struct spi_transfer	t = {
			//.tx_buf		= spidev->buffer,
			.tx_buf     = buf,
			.len		= len,
		};
	struct spi_message	m;

    //spi_trace("%s\n", __FUNCTION__);

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

static inline ssize_t
spidev_sync_read(struct spidev_data *spidev, u8 *buf, size_t len)
{    
	struct spi_transfer	t = {
			.rx_buf = buf,
			.len    = len,
		};
	struct spi_message  m;

    //spi_trace("%s\n", __FUNCTION__);

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

static inline ssize_t
spidev_sync_write_read(struct spidev_data *spidev, void *send, size_t ss, void* rcv, size_t rs)
{    
    struct spi_transfer	t[2] = {
        {
            .tx_buf     = send,
            .len        = ss,
        },   
        {
			.rx_buf		= rcv,
			.len		= rs,
		}
    };
	struct spi_message m;

    //spi_trace("%s\n", __FUNCTION__);

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
    spi_message_add_tail(&t[1], &m);
	return spidev_sync(spidev, &m);
}


static unsigned char SPI_ReadReg(unsigned char devid, unsigned char offset)
{
	//u32 command = 0;
	//u32 data = 0;
    
	int ret;
    u8 cmd[64];

	cmd[0] = (0x80 | offset | devid << 1);

    // here add spi read process
    mutex_lock(&spi_xr20m1172_data->buf_lock);
    ret = spidev_sync_write_read(spi_xr20m1172_data, &cmd[0], 1, &cmd[1], 2);

    mutex_unlock(&spi_xr20m1172_data->buf_lock);

	DMSG("addr:%x, command:%x, data:%x%x", offset / 8, cmd[0], cmd[1], cmd[2]);
	return (cmd[2] & 0xff);

	//return 0;
}

static void SPI_WriteReg(unsigned char devid, unsigned char offset,
			 unsigned char value)
{
	//u32 command = 0;

	int ret;
    u8 cmd[64];

	//command = ((offset | devid << 1) << 8) | value;
	cmd[0] = (offset | devid << 1);
    cmd[1] = value;

    // here add spi read process
	mutex_lock(&spi_xr20m1172_data->buf_lock);
    //spi_xr20m1172_data->buffer = cmd;
    
	ret = spidev_sync_write(spi_xr20m1172_data, cmd, 2);

	mutex_unlock(&spi_xr20m1172_data->buf_lock);


	DMSG("addr:%d, command:%x, value:%x", offset / 8, cmd[0], value);


	return;
}

static void serial_xr20m1172_enable_ms(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;

	DMSG("+");
	up->ier |= UART_IER_MSI;
	serial_out(up->devid, XR20M1170REG_IER, up->ier);
	DMSG("-");
}

static void serial_xr20m1172_stop_tx(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	DMSG("+");
	if (up->ier & UART_IER_THRI) {
		up->ier &= ~UART_IER_THRI;
		serial_out(up->devid, XR20M1170REG_IER, up->ier);
	}
	DMSG("-");
}

static void serial_xr20m1172_stop_rx(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;

	DMSG("+");
	up->ier &= ~UART_IER_RLSI;
	up->port.read_status_mask &= ~UART_LSR_DR;
	serial_out(up->devid, XR20M1170REG_IER, up->ier);
	DMSG("-");
}

static void pio_receive_chars(struct xr20m1172_port *up, int *status)
{
	//struct tty_struct *tty = up->port.info->tty;
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int ch, flag;
	int max_count = 256;

	DMSG("+");
	do {
		ch = serial_in(up->devid, XR20M1170REG_RHR);
		flag = TTY_NORMAL;
		up->port.icount.rx++;

		if (unlikely(*status & (UART_LSR_BI | UART_LSR_PE |
					UART_LSR_FE | UART_LSR_OE))) {
			/* For statistics only */
			if (*status & UART_LSR_BI) {
				*status &= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port))
					goto ignore_char;
			} else if (*status & UART_LSR_PE)
				up->port.icount.parity++;
			else if (*status & UART_LSR_FE)
				up->port.icount.frame++;
			if (*status & UART_LSR_OE)
				up->port.icount.overrun++;

			/* Mask off conditions which should be ignored. */
			*status &= up->port.read_status_mask;

			if (*status & UART_LSR_BI) {
				flag = TTY_BREAK;
			} else if (*status & UART_LSR_PE)
				flag = TTY_PARITY;
			else if (*status & UART_LSR_FE)
				flag = TTY_FRAME;
		}

		uart_insert_char(&up->port, *status, UART_LSR_OE, ch, flag);

	      ignore_char:
		*status = serial_in(up->devid, XR20M1170REG_LSR);
	} while ((*status & UART_LSR_DR) && (max_count-- > 0));

	//tty_flip_buffer_push(tty);
	tty_flip_buffer_push(tty->port);
	DMSG("-");
}

static void pio_transmit_chars(struct xr20m1172_port *up)
{
	//struct circ_buf *xmit = &up->port.info->xmit;
	struct circ_buf *xmit = &up->port.state->xmit;
	int count;

	DMSG("+");
	if (up->port.x_char) {
		serial_out(up->devid, XR20M1170REG_THR, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char = 0;
		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&up->port)) {
		serial_xr20m1172_stop_tx(&up->port);
		return;
	}

	count = up->port.fifosize / 2;
	do {
		serial_out(up->devid, XR20M1170REG_THR, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		up->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);

	if (uart_circ_empty(xmit))
		serial_xr20m1172_stop_tx(&up->port);
	DMSG("-");
}

static void serial_xr20m1172_start_tx(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;

	DMSG("+");
	if (!(up->ier & UART_IER_THRI)) {
		up->ier |= UART_IER_THRI;
		serial_out(up->devid, XR20M1170REG_IER, up->ier);
	}
	DMSG("-");

    //wake_up_interruptible(&up->port.state->port.delta_msr_wait);
}

static void check_modem_status(struct xr20m1172_port *up)
{
	int status;

	DMSG("+");
	status = serial_in(up->devid, XR20M1170REG_MSR);

	if (!(status & UART_MSR_ANY_DELTA))
		return;

	if (status & UART_MSR_TERI)
		up->port.icount.rng++;
	if (status & UART_MSR_DDSR)
		up->port.icount.dsr++;
	if (status & UART_MSR_DDCD)
		uart_handle_dcd_change(&up->port, status & UART_MSR_DCD);
	if (status & UART_MSR_DCTS)
		uart_handle_cts_change(&up->port, status & UART_MSR_CTS);

	//wake_up_interruptible(&up->port.info->delta_msr_wait);
	wake_up_interruptible(&up->port.state->port.delta_msr_wait);
	DMSG("-");
}

/*
static irqreturn_t BypassIrq(void)
{
    return IRQ_HANDLED;
}*/

#if 1
/* This handles the interrupt from one port.  */
static irqreturn_t serial_xr20m1172_irq(int irq, void *dev_id)
{
	struct xr20m1172_port *up = dev_id;
	unsigned int isr, lsr;

    //pr_info("%s\n", __func__);
    //DMSG("irq");
    /*
    if (IRQ_HANDLED == BypassIrq()) 
    {
        return IRQ_HANDLED;
    }*/

	DMSG("+");
	isr = serial_in(up->devid, XR20M1170REG_ISR);
	if (isr & UART_IIR_NO_INT) {
		/* FIXME, should return IRQ_NONE normally, but it would report
		 * unknown irq in PIO mode */
		return IRQ_HANDLED;
	}
	lsr = serial_in(up->devid, XR20M1170REG_LSR);
	if (lsr & UART_LSR_DR) {
		pio_receive_chars(up, &lsr);
#ifdef CONFIG_IPM
		ipm_event_notify(IPM_EVENT_DEVICE, IPM_EVENT_DEVICE_OUTD0CS,
				 NULL, 0);
#endif
	}
	check_modem_status(up);
	if (lsr & UART_LSR_THRE)
		pio_transmit_chars(up);
	DMSG("-");
	return IRQ_HANDLED;
}
#endif

static unsigned int serial_xr20m1172_tx_empty(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	//unsigned long flags;
	unsigned int ret;

	DMSG("+");
	//spin_lock_irqsave(&up->port.lock, flags);
	ret =
	    serial_in(up->devid,
		      XR20M1170REG_LSR) & UART_LSR_TEMT ? TIOCSER_TEMT : 0;
	//spin_unlock_irqrestore(&up->port.lock, flags);
	DMSG("-");

	return ret;
}

static unsigned int serial_xr20m1172_get_mctrl(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	unsigned char status;
	unsigned int ret;

	DMSG("+");
	status = serial_in(up->devid, XR20M1170REG_MSR);

	ret = 0;
	if (status & UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & UART_MSR_DSR)
		ret |= TIOCM_DSR;
	if (status & UART_MSR_CTS)
		ret |= TIOCM_CTS;
	DMSG("-");
	return ret;
}

static void serial_xr20m1172_set_mctrl(struct uart_port *port,
				       unsigned int mctrl)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	unsigned char mcr = 0;

	DMSG("+");

	if (mctrl & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (mctrl & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (mctrl & TIOCM_OUT1)
		mcr |= UART_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= UART_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;

	mcr |= up->mcr;

	serial_out(up->devid, XR20M1170REG_MCR, mcr);

	DMSG("-mcr:%x", mcr);
}

static void serial_xr20m1172_break_ctl(struct uart_port *port, int break_state)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	//unsigned long flags;

	DMSG("+");
	//spin_lock_irqsave(&up->port.lock, flags);
	if (break_state == -1)
		up->lcr |= UART_LCR_SBC;
	else
		up->lcr &= ~UART_LCR_SBC;
	serial_out(up->devid, XR20M1170REG_LCR, up->lcr);
	//spin_unlock_irqrestore(&up->port.lock, flags);
	DMSG("-");
}

static irqreturn_t xr20m1172_ist_top(int irq, void *dev_id)
{
	return IRQ_WAKE_THREAD;
}


static int serial_xr20m1172_startup(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	//unsigned long flags;
	int ret;

	DMSG("+");
    
	up->mcr = 0;
	//if (up->port.irq < 0) {
	if (up->port.irq > 0) {

        /*
        ret = request_irq(up->port.irq, serial_xr20m1172_irq, 
                                    IRQF_TRIGGER_FALLING, up->name, (void*) up);
                                    */
        ret = devm_request_threaded_irq(up->port.dev, up->port.irq, xr20m1172_ist_top, serial_xr20m1172_irq,
//					IRQF_ONESHOT | flags, dev_name(dev), s);
					IRQF_TRIGGER_FALLING, up->name, (void*) up);
        
        if (ret) {
            pr_err("request irq err\n");
            return ret;
        }

        pr_err("request_irq succeed\n");
	}

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reenabled in set_termios())
	 */
	serial_out(up->devid, XR20M1170REG_FCR, UART_FCR_ENABLE_FIFO);
	serial_out(up->devid, XR20M1170REG_FCR,
		   UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR |
		   UART_FCR_CLEAR_XMIT);
	serial_out(up->devid, XR20M1170REG_FCR, 0);

	/* Clear the interrupt registers.  */
	(void)serial_in(up->devid, XR20M1170REG_LSR);
	(void)serial_in(up->devid, XR20M1170REG_RHR);
	(void)serial_in(up->devid, XR20M1170REG_ISR);
	(void)serial_in(up->devid, XR20M1170REG_MSR);

	/* Now, initialize the UART */
	serial_out(up->devid, XR20M1170REG_LCR, UART_LCR_WLEN8);

	//spin_lock_irqsave(&up->port.lock, flags);
	up->port.mctrl |= TIOCM_OUT2;
	serial_xr20m1172_set_mctrl(&up->port, up->port.mctrl);
	//spin_unlock_irqrestore(&up->port.lock, flags);

	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	/*RTOIE is 0x10, and should be Sleep Mode Enable */
	//up->ier = UART_IER_RLSI | UART_IER_RDI | UART_IER_RTOIE;
	up->ier = UART_IER_RLSI | UART_IER_RDI;
	serial_out(up->devid, XR20M1170REG_IER, up->ier);

	/* And clear the interrupt registers again for luck.  */
	(void)serial_in(up->devid, XR20M1170REG_LSR);
	(void)serial_in(up->devid, XR20M1170REG_RHR);
	(void)serial_in(up->devid, XR20M1170REG_ISR);
	(void)serial_in(up->devid, XR20M1170REG_MSR);
#ifdef CONFIG_PM
	up->power_mode = POWER_RUN;
#endif
#ifdef CONFIG_DVFM
	up->inuse = 1;
#endif

	DMSG("-");
	return 0;
}

static void serial_xr20m1172_shutdown(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	//unsigned long flags;

	DMSG("+");
#ifdef CONFIG_DVFM
	up->inuse = 0;
#endif
	free_irq(up->port.irq, up);

	/* Disable interrupts from this port */
	up->ier = 0;
	serial_out(up->devid, XR20M1170REG_IER, 0);

	//spin_lock_irqsave(&up->port.lock, flags);
	up->port.mctrl &= ~TIOCM_OUT2;
	up->port.mctrl &= ~TIOCM_RTS;
	up->port.mctrl &= ~TIOCM_CTS;
	serial_xr20m1172_set_mctrl(&up->port, up->port.mctrl);
	//spin_unlock_irqrestore(&up->port.lock, flags);

	/* Disable break condition and FIFOs */
	serial_out(up->devid, XR20M1170REG_LCR,
		   serial_in(up->devid, XR20M1170REG_LCR) & ~UART_LCR_SBC);
	serial_out(up->devid, XR20M1170REG_FCR,
		   UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR |
		   UART_FCR_CLEAR_XMIT);
	serial_out(up->devid, XR20M1170REG_FCR, 0);
	DMSG("-");
}

static void
serial_xr20m1172_set_termios(struct uart_port *port, struct ktermios *termios,
			     struct ktermios *old)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	unsigned char cval, fcr = 0;
	unsigned char efr;
	//unsigned long flags;
	unsigned int baud;
	unsigned char dld_reg;
	unsigned char prescale;
	unsigned char samplemode;
	unsigned short required_diviser;
	unsigned long required_diviser2;

    DBAUT("+");

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = UART_LCR_WLEN5;
		break;
	case CS6:
		cval = UART_LCR_WLEN6;
		break;
	case CS7:
		cval = UART_LCR_WLEN7;
		break;
	case CS8:
	default:
		cval = UART_LCR_WLEN8;
		break;
	}

	if (termios->c_cflag & CSTOPB)
		cval |= UART_LCR_STOP;
	if (termios->c_cflag & PARENB)
		cval |= UART_LCR_PARITY;
	if (!(termios->c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
	if (termios->c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;

	/* Ask the core to calculate the divisor for us.  */
	baud = uart_get_baud_rate(port, termios, old, 0, 4000000);

    DBAUT("baud %d, cval %x\n", baud, cval);

#ifdef CONFIG_DVFM
	up->baud = baud;	/* save for DVFM scale callback */
#endif

	if (baud < 230400)
		fcr = UART_FCR_ENABLE_FIFO;	//8 bytes
	else
		fcr = UART_FCR_ENABLE_FIFO | 0x80;	// 56 bytes

	prescale = 1;		//divide by 1,   (MCR bit 7);
	//samplemode = 4;		//4x,            (DLD bit 5:4);
	samplemode = 16;

	//required_diviser2 = (14745600 * 16) / (prescale * samplemode * baud);
	required_diviser2 = (24000000 * 16) / (prescale * samplemode * baud);
	required_diviser = required_diviser2 / 16;
	dld_reg = required_diviser2 - required_diviser * 16;

	switch (samplemode) {
	case 16:
		break;
	case 8:
		dld_reg |= 0x10;
		break;
	case 4:
		dld_reg |= 0x20;
		break;
	default:
		printk(KERN_ERR "should not be here!\n");
	}

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	//spin_lock_irqsave(&up->port.lock, flags);

	/* Update the per-port timeout.  */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/* Characters to ignore */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/* ignore all characters if CREAD is not set */
	if (!(termios->c_cflag & CREAD))
		up->port.ignore_status_mask |= UART_LSR_DR;

	/* CTS flow control flag and modem status interrupts */
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;
	else
		up->ier &= ~UART_IER_MSI;
	serial_out(up->devid, XR20M1170REG_IER, up->ier);

	serial_out(up->devid, XR20M1170REG_DLM, required_diviser >> 8);
	serial_out(up->devid, XR20M1170REG_DLL, required_diviser & 0xff);
	serial_out(up->devid, XR20M1170REG_DLD, dld_reg);

    DBAUT("DLM[%x], DLL[%x], DLD[%x]\n", required_diviser >> 8, required_diviser & 0xff, dld_reg);
    
	up->lcr = cval;		/* Save LCR */
	serial_out(up->devid, XR20M1170REG_LCR, cval);	/* reset DLAB */
	serial_xr20m1172_set_mctrl(&up->port, up->port.mctrl);
	serial_out(up->devid, XR20M1170REG_FCR, fcr);

	if (termios->c_cflag & CRTSCTS) {
		efr = serial_in(up->devid, XR20M1170REG_EFR);
		efr = efr | UART_EFR_RTS | UART_EFR_CTS;
		serial_out(up->devid, XR20M1170REG_EFR, efr);
	}

	//spin_unlock_irqrestore(&up->port.lock, flags);
	DBAUT("-");
}

static void serial_xr20m1172_pm(struct uart_port *port, unsigned int state,
				unsigned int oldstate)
{
	//struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	//pxa_set_cken(up->cken, !state);
	if (!state)
		udelay(1);
}

static void serial_xr20m1172_release_port(struct uart_port *port)
{
}

static int serial_xr20m1172_request_port(struct uart_port *port)
{
	return 0;
}

static void serial_xr20m1172_config_port(struct uart_port *port, int flags)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	up->port.type = PORT_PXA;
}

static int serial_xr20m1172_verify_port(struct uart_port *port,
					struct serial_struct *ser)
{
	/* we don't want the core code to modify any port params */
	return -EINVAL;
}

static const char *serial_xr20m1172_type(struct uart_port *port)
{
	struct xr20m1172_port *up = (struct xr20m1172_port *)port;
	return up->name;
}

struct uart_ops serial_xr20m1172_pops = {
	.tx_empty = serial_xr20m1172_tx_empty,
	.set_mctrl = serial_xr20m1172_set_mctrl,
	.get_mctrl = serial_xr20m1172_get_mctrl,
	.stop_tx = serial_xr20m1172_stop_tx,
	.start_tx = serial_xr20m1172_start_tx,
	.stop_rx = serial_xr20m1172_stop_rx,
	.enable_ms = serial_xr20m1172_enable_ms,
	.break_ctl = serial_xr20m1172_break_ctl,
	.startup = serial_xr20m1172_startup,
	.shutdown = serial_xr20m1172_shutdown,
	.set_termios = serial_xr20m1172_set_termios,
	.pm = serial_xr20m1172_pm,
	.type = serial_xr20m1172_type,
	.release_port = serial_xr20m1172_release_port,
	.request_port = serial_xr20m1172_request_port,
	.config_port = serial_xr20m1172_config_port,
	.verify_port = serial_xr20m1172_verify_port,
};

static struct xr20m1172_port serial_xr20m1172_ports[] = {
	{			/* XMUART1 */
	 .name = "XMUART0",
	 .devid = 0,
	 .port = {
		  .type = PORT_PXA,
		  .iotype = UPIO_MEM,
		  .membase = 0,
		  .mapbase = 0,
		  //.irq = GPIO_EXT_TO_IRQ(16),
		  //.irq = gpio_to_irq(97),
		  .irq = 97,
		  .uartclk = 24000000,
		  .fifosize = 64,
		  .ops = &serial_xr20m1172_pops,
		  .line = 0,
		  },
#ifdef CONFIG_DVFM
	 .dvfm_notifier = {
			   .name = "xm-uart",
			   .priority = 0,
			   .notifier_call = uart_dvfm_notifier,
			   },
#endif
	 }, {
	     .name = "XMUART1",
	     .devid = 1,
	     .port = {
		      .type = PORT_PXA,
		      .iotype = UPIO_MEM,
		      .membase = 0,
		      .mapbase = 0,
		      //.irq = GPIO_EXT_TO_IRQ(16),
		      //.irq = gpio_to_irq(97),
		      .irq = 97,
		      .uartclk = 24000000,
		      .fifosize = 64,
		      .ops = &serial_xr20m1172_pops,
		      .line = 1,
		      },
#ifdef CONFIG_DVFM
	     .dvfm_notifier = {
			       .name = "xm-uart",
			       .priority = 0,
			       .notifier_call = uart_dvfm_notifier,
			       },
#endif
	     },
};

static struct uart_driver serial_xr20m1172_drv = {
	.owner = THIS_MODULE,
	.driver_name = "xr20m1172",
	.dev_name = "ttyXM",
	.major = TTY_MAJOR,
	.minor = 70,
	.nr = ARRAY_SIZE(serial_xr20m1172_ports),
	.cons = NULL,
};

#ifdef CONFIG_PM
//static int serial_xr20m1172_suspend(struct platform_device *dev,
//				    pm_message_t state)
static int serial_xr20m1172_suspend(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);
	struct xr20m1172_port *sport = platform_get_drvdata(pdev);

	DMSG("+");
	if (sport) {
		sport->power_mode = POWER_PRE_SUSPEND;
		uart_suspend_port(&serial_xr20m1172_drv, &sport->port);
		sport->power_mode = POWER_SUSPEND;
	}
	DMSG("-");
	return 0;
}

//static int serial_xr20m1172_resume(struct platform_device *dev)
static int serial_xr20m1172_resume(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);
	struct xr20m1172_port *sport = platform_get_drvdata(pdev);

	DMSG("+");
	if (sport) {
		sport->power_mode = POWER_PRE_RESUME;
		uart_resume_port(&serial_xr20m1172_drv, &sport->port);
		sport->power_mode = POWER_RUN;
	}

	DMSG("-");
	return 0;
}
#else
#define serial_xr20m1172_suspend    NULL
#define serial_xr20m1172_resume    NULL
#endif

#ifdef CONFIG_DVFM
static int uart_dvfm_notifier(unsigned int cmd, void *client_data, void *info)
{
	struct pxa3xx_fv_notifier_info *notifier_info =
	    (struct pxa3xx_fv_notifier_info *)info;
	struct xr20m1172_port *up = (struct xr20m1172_port *)client_data;

	switch (cmd) {
	case FV_NOTIFIER_QUERY_SET:
		/* entry/exit D0CS mode */
		if (notifier_info->cur.d0cs != notifier_info->next.d0cs) {
			if ((serial_xr20m1172_tx_empty((struct uart_port *)up)
			     != TIOCSER_TEMT) || (up->inuse
						  && (up->baud >= 57600))) {
				if (up->baud >= 921600)
					return -EAGAIN;
			}
		}
		break;

	case FV_NOTIFIER_PRE_SET:
		break;

	case FV_NOTIFIER_POST_SET:
		break;
	}

	return 0;
}

#endif

static atomic_t xr20m1172_next_id = ATOMIC_INIT(0);

static int serial_xr20m1172_probe(struct platform_device *dev)
{
    int xr20m_int = 0;
    int ret = 0;
    
    struct device_node *np = dev->dev.of_node;

    if (dev->id == -1)
        dev->id = atomic_inc_return(&xr20m1172_next_id) - 1;
    
    pr_info("detected port #%d (ttyXM%d)\n", dev->id, dev->id);

    xr20m_int = of_get_named_gpio(np, "xr20m,irq", 0);
    if (!gpio_is_valid(xr20m_int)) {
        pr_err("xr20m_int err\n");
        //return -1;
    } else {

        ret = gpio_request(xr20m_int, "xr20m,irq");
        if (ret < 0) {
            pr_err("request gpio[%d] err\n", xr20m_int);
            return ret;
        }

        gpio_direction_input(xr20m_int);

        if (gpio_to_irq(xr20m_int) < 0) {
            pr_err("gpio[%d] to irq err\n", xr20m_int);
            gpio_to_irq(xr20m_int);
        }

        serial_xr20m1172_ports[0].port.irq = gpio_to_irq(xr20m_int);
        serial_xr20m1172_ports[1].port.irq = gpio_to_irq(xr20m_int);
    }
    
    serial_xr20m1172_ports[dev->id].port.dev = &dev->dev;
    uart_add_one_port(&serial_xr20m1172_drv,
            &serial_xr20m1172_ports[dev->id].port);
    platform_set_drvdata(dev, &serial_xr20m1172_ports[dev->id]);
#ifdef CONFIG_PM
    serial_xr20m1172_ports[dev->id].power_mode = POWER_RUN;
#endif

#ifdef CONFIG_DVFM
    serial_xr20m1172_ports[dev->id].dvfm_notifier.client_data =
        &serial_xr20m1172_ports[dev->id];
    pxa3xx_fv_register_notifier(&serial_xr20m1172_ports[dev->id].
                dvfm_notifier);
    init_waitqueue_head(&serial_xr20m1172_ports[dev->id].delay_wait);
#endif

#ifdef DEBUG_VIA_PROC_FS
    if (create_xr20m_proc_file()) {
        printk(KERN_INFO "%s: Failed to create xr20m proc file.\n",
                __func__);
        return -1;
    }
#endif

    return 0;
}

static int serial_xr20m1172_remove(struct platform_device *dev)
{
	struct xr20m1172_port *sport = platform_get_drvdata(dev);
	platform_set_drvdata(dev, NULL);

	if (sport)
		uart_remove_one_port(&serial_xr20m1172_drv, &sport->port);

#ifdef CONFIG_DVFM
	pxa3xx_fv_unregister_notifier(&sport->dvfm_notifier);
#endif

#ifdef    DEBUG_VIA_PROC_FS
	remove_xr20m_proc_file();
#endif

	return 0;
}

/*-------------------------------------------------------------------------*/
#define SPIDEV_MAJOR			153	/* assigned */
#define N_SPI_MINORS			32	/* ... up to 256 */

//static DECLARE_BITMAP(minors, N_SPI_MINORS);

//static LIST_HEAD(device_list);
//static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 4096;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*
 * This can be used for testing the controller, given the busnum and the
 * cs required to use. If those parameters are used, spidev is
 * dynamically added as device on the busnum, and messages can be sent
 * via this interface.
 */
static int busnum = -1;
module_param(busnum, int, S_IRUGO);
MODULE_PARM_DESC(busnum, "bus num of the controller");

static int chipselect = -1;
module_param(chipselect, int, S_IRUGO);
MODULE_PARM_DESC(chipselect, "chip select of the desired device");

static int maxspeed = 10000000;
module_param(maxspeed, int, S_IRUGO);
MODULE_PARM_DESC(maxspeed, "max_speed of the desired device");

static int spimode = SPI_MODE_3;
module_param(spimode, int, S_IRUGO);
MODULE_PARM_DESC(spimode, "mode of the desired device");


static int spidev_probe(struct spi_device *spi)
{
	//struct spidev_data	*spidev;
	int			status;
    //int     ret;
	//unsigned long		minor;

    spi_trace("%s\n", __FUNCTION__);

	/* Allocate driver data */
	spi_xr20m1172_data = kzalloc(sizeof(*spi_xr20m1172_data), GFP_KERNEL);
	if (!spi_xr20m1172_data)
		return -ENOMEM;

	/* Initialize the driver data */
	spi_xr20m1172_data->spi = spi;
	spin_lock_init(&spi_xr20m1172_data->spi_lock);
	mutex_init(&spi_xr20m1172_data->buf_lock);

    // set spi max speed
    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi->max_speed_hz = 10000000;
    //ret = spi_setup(spi);
    if (spi_setup(spi) < 0) {
        pr_err("setup spi error\n");
    } else {
        pr_err("setup spi ok\n");
    }

	//INIT_LIST_HEAD(&spidev->device_entry);

    dev_dbg(&spi->dev, "busnum=%d cs=%d bufsiz=%d maxspeed=%d",
			busnum, chipselect, bufsiz, maxspeed);

    spi_set_drvdata(spi, spi_xr20m1172_data);

    cached_lcr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_LCR][0]);
	cached_efr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_EFR][0]);
	cached_mcr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_MCR][0]);
	cached_lcr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_LCR][0]);
	cached_efr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_EFR][0]);
	cached_mcr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_MCR][0]);

#ifdef MAKE_SPI_NODE_FOR_DEBUG
    register_spi_char();
    create_spi_class(spi_xr20m1172_data);
#endif

    //spi_xr20m1172_data = spidev;

	/* If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	/*
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		spidev->devt = MKDEV(SPIDEV_MAJOR, minor);
		dev = device_create(spidev_class, &spi->dev, spidev->devt,
				    spidev, "spidev%d.%d",
				    spi->master->bus_num, spi->chip_select);
        
        dev_dbg(&spi->dev, "create spi device, spidev%d.%d\n", spi->master->bus_num, spi->chip_select);
		status = PTR_RET(dev);
	} else {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&spidev->device_entry, &device_list);
	}
	mutex_unlock(&device_list_lock);

	if (status == 0) {
		spi_set_drvdata(spi, spidev);
        //serial_out(0,0,0);
    }
	else {
		kfree(spidev);
        dev_err(&spi->dev, "create dev error\n");
    }

    // add by ycb
    if (0 == status) {
        dev_dbg(&spi->dev, "begin xr20m1172 init\n");
        serial_xr20m1172_init(spidev);
    }*/
    dev_dbg(&spi->dev, "%s exit, status=%d\n", __FUNCTION__, status);
	return status;
}

static int spidev_remove(struct spi_device *spi)
{
	struct spidev_data	*spidev = spi_get_drvdata(spi);

    spi_trace("%s\n", __FUNCTION__);

	/* make sure ops on existing fds can abort cleanly */
	spin_lock_irq(&spidev->spi_lock);
	spidev->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&spidev->spi_lock);


#ifdef MAKE_SPI_NODE_FOR_DEBUG
        clear_spi_class(spi_xr20m1172_data->spi);
#endif


	/* prevent new opens */
	//mutex_lock(&device_list_lock);
	//list_del(&spidev->device_entry);
	//device_destroy(spidev_class, spidev->devt);
	//clear_bit(MINOR(spidev->devt), minors);
	//if (spidev->users == 0)
		//kfree(spidev);
	//mutex_unlock(&device_list_lock);

	return 0;
}


static const struct of_device_id spidev_dt_ids[] = {
	//{ .compatible = "rohm,dh2228fv" },
	{ .compatible = "qcom,spi-test" },
	{},
};

MODULE_DEVICE_TABLE(of, spidev_dt_ids);

static struct spi_driver spidev_spi_driver = {
	.driver = {
		.name =		"spidev",
		.owner =	THIS_MODULE,
		.of_match_table = of_match_ptr(spidev_dt_ids),
	},
	.probe =	spidev_probe,
	.remove =	spidev_remove,

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};

static struct of_device_id szzt_xr20m1172_match_table[] = {
	{
	    
	    .compatible = "szzt,xr20m1172",
		.data = (void *)1,
	},
	{}
};

static struct dev_pm_ops szzt_xr20m1172_dev_pm_ops = {
	.suspend = serial_xr20m1172_suspend,
	.resume = serial_xr20m1172_resume,
	.runtime_suspend = serial_xr20m1172_suspend,
	.runtime_resume = serial_xr20m1172_resume,
};

static struct platform_driver serial_xr20m1172_driver = {
	.probe = serial_xr20m1172_probe,
	.remove = serial_xr20m1172_remove,
	//.suspend = serial_xr20m1172_suspend,
	//.resume = serial_xr20m1172_resume,
	.driver = {
		    .name = "xr20m1172-uart",
            .owner = THIS_MODULE,
            .pm = &szzt_xr20m1172_dev_pm_ops,
            .of_match_table = szzt_xr20m1172_match_table,
		   },
};

int __init serial_xr20m1172_init(void)
{
	int ret;

	//xr20m1172_ssp_init();
	pr_info("%s szzt,xr20m1172\n", __FUNCTION__);
    
    ret = spi_register_driver(&spidev_spi_driver);
	if (ret < 0) {
        pr_err("%s spi register error\n", __func__);
		return ret;
    }

    /*
	cached_lcr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_LCR][0]);
	cached_efr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_EFR][0]);
	cached_mcr[0] = SPI_ReadReg(0, reg_info[XR20M1170REG_MCR][0]);
	cached_lcr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_LCR][0]);
	cached_efr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_EFR][0]);
	cached_mcr[1] = SPI_ReadReg(1, reg_info[XR20M1170REG_MCR][0]);
	*/

	ret = uart_register_driver(&serial_xr20m1172_drv);
	if (ret != 0)
		return ret;

	ret = platform_driver_register(&serial_xr20m1172_driver);
	if (ret != 0)
		uart_unregister_driver(&serial_xr20m1172_drv);

	return ret;
}

void __exit serial_xr20m1172_exit(void)
{
	platform_driver_unregister(&serial_xr20m1172_driver);
	uart_unregister_driver(&serial_xr20m1172_drv);
	//xr20m1172_ssp_exit();
}

module_init(serial_xr20m1172_init);
module_exit(serial_xr20m1172_exit);



#ifdef MAKE_SPI_NODE_FOR_DEBUG

static struct class *spidev_class;
#define SPIDEV_MAJOR			153	/* assigned */
#define N_SPI_MINORS			32	/* ... up to 256 */

static DECLARE_BITMAP(minors, N_SPI_MINORS);

/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for some settings can cause *lots* of trouble for other
 * devices on a shared bus:
 *
 *  - CS_HIGH ... this device will be active when it shouldn't be
 *  - 3WIRE ... when active, it won't behave as it should
 *  - NO_CS ... there will be no explicit message boundaries; this
 *	is completely incompatible with the shared bus model
 *  - READY ... transfers may proceed when they shouldn't.
 *
 * REVISIT should changing those flags be privileged?
 */
#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY)

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

/* Read-only message with current device setup */
static ssize_t
spidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct spidev_data	*spidev;
	ssize_t			status = 0;

    spi_trace("%s\n", __FUNCTION__);

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	spidev = filp->private_data;

	mutex_lock(&spidev->buf_lock);
	status = spidev_sync_read(spidev, spidev->buffer, count);
    
	if (status > 0) {
		unsigned long	missing;

		missing = copy_to_user(buf, spidev->buffer, status);
		if (missing == status)
			status = -EFAULT;
		else
			status = status - missing;
	}
	mutex_unlock(&spidev->buf_lock);

	return status;
}

/* Write-only message with current device setup */
static ssize_t
spidev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct spidev_data	*spidev;
	ssize_t			status = 0;
	unsigned long		missing;

    spi_trace("%s\n", __FUNCTION__);

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	spidev = filp->private_data;

	mutex_lock(&spidev->buf_lock);
	missing = copy_from_user(spidev->buffer, buf, count);
    
	if (missing == 0) {
		status = spidev_sync_write(spidev, spidev->buffer, count);
	} else {
		status = -EFAULT;
    }
	mutex_unlock(&spidev->buf_lock);

	return status;
}

static int spidev_message(struct spidev_data *spidev,
		struct spi_ioc_transfer *u_xfers, unsigned n_xfers)
{
	struct spi_message	msg;
	struct spi_transfer	*k_xfers;
	struct spi_transfer	*k_tmp;
	struct spi_ioc_transfer *u_tmp;
	unsigned		n, total;
	u8			*buf, *bufrx;
	int			status = -EFAULT;

	spi_message_init(&msg);
	k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
	if (k_xfers == NULL)
		return -ENOMEM;

	/* Construct spi_message, copying any tx data to bounce buffer.
	 * We walk the array of user-provided transfers, using each one
	 * to initialize a kernel version of the same transfer.
	 */
	buf = spidev->buffer;
	bufrx = spidev->bufferrx;
	total = 0;
	for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
			n;
			n--, k_tmp++, u_tmp++) {
		k_tmp->len = u_tmp->len;

		total += k_tmp->len;
		if (total > bufsiz) {
			status = -EMSGSIZE;
			goto done;
		}

		if (u_tmp->rx_buf) {
			k_tmp->rx_buf = bufrx;
			if (!access_ok(VERIFY_WRITE, (u8 __user *)
						(uintptr_t) u_tmp->rx_buf,
						u_tmp->len))
				goto done;
		}
		if (u_tmp->tx_buf) {
			k_tmp->tx_buf = buf;
			if (copy_from_user(buf, (const u8 __user *)
						(uintptr_t) u_tmp->tx_buf,
					u_tmp->len))
				goto done;
		}
		buf += k_tmp->len;
		bufrx += k_tmp->len;

		k_tmp->cs_change = !!u_tmp->cs_change;
		k_tmp->bits_per_word = u_tmp->bits_per_word;
		k_tmp->delay_usecs = u_tmp->delay_usecs;
		k_tmp->speed_hz = u_tmp->speed_hz;
#ifdef VERBOSE
		dev_dbg(&spidev->spi->dev,
			"  xfer len %zd %s%s%s%dbits %u usec %uHz\n",
			u_tmp->len,
			u_tmp->rx_buf ? "rx " : "",
			u_tmp->tx_buf ? "tx " : "",
			u_tmp->cs_change ? "cs " : "",
			u_tmp->bits_per_word ? : spidev->spi->bits_per_word,
			u_tmp->delay_usecs,
			u_tmp->speed_hz ? : spidev->spi->max_speed_hz);
#endif
		spi_message_add_tail(k_tmp, &msg);
	}

	status = spidev_sync(spidev, &msg);
	if (status < 0)
		goto done;

	/* copy any rx data out of bounce buffer */
	buf = spidev->bufferrx;
	for (n = n_xfers, u_tmp = u_xfers; n; n--, u_tmp++) {
		if (u_tmp->rx_buf) {
			if (__copy_to_user((u8 __user *)
					(uintptr_t) u_tmp->rx_buf, buf,
					u_tmp->len)) {
				status = -EFAULT;
				goto done;
			}
		}
		buf += u_tmp->len;
	}
	status = total;

done:
	kfree(k_xfers);
	return status;
}

static long
spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int			err = 0;
	int			retval = 0;
	struct spidev_data	*spidev;
	struct spi_device	*spi;
	u32			tmp;
	unsigned		n_ioc;
	struct spi_ioc_transfer	*ioc;

    spi_trace("%s\n", __FUNCTION__);

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	/* guard against device removal before, or while,
	 * we issue this ioctl.
	 */
	spidev = filp->private_data;
	spin_lock_irq(&spidev->spi_lock);
	spi = spi_dev_get(spidev->spi);
	spin_unlock_irq(&spidev->spi_lock);

	if (spi == NULL)
		return -ESHUTDOWN;

	/* use the buffer lock here for triple duty:
	 *  - prevent I/O (from us) so calling spi_setup() is safe;
	 *  - prevent concurrent SPI_IOC_WR_* from morphing
	 *    data fields while SPI_IOC_RD_* reads them;
	 *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&spidev->buf_lock);

	switch (cmd) {
	/* read requests */
	case SPI_IOC_RD_MODE:
		retval = __put_user(spi->mode & SPI_MODE_MASK,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_LSB_FIRST:
		retval = __put_user((spi->mode & SPI_LSB_FIRST) ?  1 : 0,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_BITS_PER_WORD:
		retval = __put_user(spi->bits_per_word, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MAX_SPEED_HZ:
		retval = __put_user(spi->max_speed_hz, (__u32 __user *)arg);
		break;

	/* write requests */
	case SPI_IOC_WR_MODE:
		retval = __get_user(tmp, (u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp & ~SPI_MODE_MASK) {
				retval = -EINVAL;
				break;
			}

			tmp |= spi->mode & ~SPI_MODE_MASK;
			spi->mode = (u8)tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "spi mode %02x\n", tmp);
		}
		break;
	case SPI_IOC_WR_LSB_FIRST:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp)
				spi->mode |= SPI_LSB_FIRST;
			else
				spi->mode &= ~SPI_LSB_FIRST;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "%csb first\n",
						tmp ? 'l' : 'm');
		}
		break;
	case SPI_IOC_WR_BITS_PER_WORD:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->bits_per_word;

			spi->bits_per_word = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->bits_per_word = save;
			else
				dev_dbg(&spi->dev, "%d bits per word\n", tmp);
		}
		break;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		retval = __get_user(tmp, (__u32 __user *)arg);
		if (retval == 0) {
			u32	save = spi->max_speed_hz;

			spi->max_speed_hz = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->max_speed_hz = save;
			else
				dev_dbg(&spi->dev, "%d Hz (max)\n", tmp);
		}
		break;

	default:
		/* segmented and/or full-duplex I/O request */
		if (_IOC_NR(cmd) != _IOC_NR(SPI_IOC_MESSAGE(0))
				|| _IOC_DIR(cmd) != _IOC_WRITE) {
			retval = -ENOTTY;
			break;
		}

		tmp = _IOC_SIZE(cmd);
		if ((tmp % sizeof(struct spi_ioc_transfer)) != 0) {
			retval = -EINVAL;
			break;
		}
		n_ioc = tmp / sizeof(struct spi_ioc_transfer);
		if (n_ioc == 0)
			break;

		/* copy into scratch area */
		ioc = kmalloc(tmp, GFP_KERNEL);
		if (!ioc) {
			retval = -ENOMEM;
			break;
		}
		if (__copy_from_user(ioc, (void __user *)arg, tmp)) {
			kfree(ioc);
			retval = -EFAULT;
			break;
		}

		/* translate to spi_message, execute */
		retval = spidev_message(spidev, ioc, n_ioc);
		kfree(ioc);
		break;
	}

	mutex_unlock(&spidev->buf_lock);
	spi_dev_put(spi);
	return retval;
}

#ifdef CONFIG_COMPAT
static long
spidev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return spidev_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define spidev_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

static int spidev_open(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = -ENXIO;

    spi_trace("%s\n", __FUNCTION__);

	mutex_lock(&device_list_lock);

	list_for_each_entry(spidev, &device_list, device_entry) {
		if (spidev->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		if (!spidev->buffer) {
			spidev->buffer = kmalloc(bufsiz, GFP_KERNEL);
			if (!spidev->buffer) {
				dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
				status = -ENOMEM;
			}
		}
		if (!spidev->bufferrx) {
			spidev->bufferrx = kmalloc(bufsiz, GFP_KERNEL);
			if (!spidev->bufferrx) {
				dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
				kfree(spidev->buffer);
				spidev->buffer = NULL;
				status = -ENOMEM;
			}
		}
		if (status == 0) {
			spidev->users++;
			filp->private_data = spidev;
			nonseekable_open(inode, filp);
		}
	} else
		pr_debug("spidev: nothing for minor %d\n", iminor(inode));

	mutex_unlock(&device_list_lock);
	return status;
}

static int spidev_release(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = 0;

    spi_trace("%s\n", __FUNCTION__);

	mutex_lock(&device_list_lock);
	spidev = filp->private_data;
	filp->private_data = NULL;

	/* last close? */
	spidev->users--;
	if (!spidev->users) {
		int		dofree;

		kfree(spidev->buffer);
		spidev->buffer = NULL;
		kfree(spidev->bufferrx);
		spidev->bufferrx = NULL;

		/* ... after we unbound from the underlying device? */
		spin_lock_irq(&spidev->spi_lock);
		dofree = (spidev->spi == NULL);
		spin_unlock_irq(&spidev->spi_lock);

		if (dofree)
			kfree(spidev);
	}
	mutex_unlock(&device_list_lock);

	return status;
}

static const struct file_operations spidev_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write =	spidev_write,
	.read =		spidev_read,
	.unlocked_ioctl = spidev_ioctl,
	.compat_ioctl = spidev_compat_ioctl,
	.open =		spidev_open,
	.release =	spidev_release,
	.llseek =	no_llseek,
};

static int register_spi_char(void)
{
    int status;

    spi_trace("%s\n", __func__);
        
    BUILD_BUG_ON(N_SPI_MINORS > 256);
    status = register_chrdev(SPIDEV_MAJOR, "spi", &spidev_fops);
    if (status < 0) {
        spi_trace("register char device error\n");
        return status;
    }

    spidev_class = class_create(THIS_MODULE, "spidev");
    if (IS_ERR(spidev_class)) {
        pr_err("create class error\n");
        return -1;
    }

    return 0;
}

static void create_spi_class(struct spidev_data	*spidev)
{
    unsigned long		minor;
    int status;
    /* If we can allocate a minor number, hook up this device.
    * Reusing minors is fine so long as udev or mdev is working.
    */
    mutex_lock(&device_list_lock);
    minor = find_first_zero_bit(minors, N_SPI_MINORS);
    if (minor < N_SPI_MINORS) {
        struct device *dev;

        spidev->devt = MKDEV(SPIDEV_MAJOR, minor);
        dev = device_create(spidev_class, &spidev->spi->dev, spidev->devt,
                    spidev, "spidev%d.%d",
                    spidev->spi->master->bus_num, spidev->spi->chip_select);
        
        //pr_err(&spidev->spi->dev, "create spi device, spidev%d.%d\n", spidev->spi->master->bus_num, spidev->spi->chip_select);
        spi_trace("create spi device, spidev%d.%d\n", spidev->spi->master->bus_num, spidev->spi->chip_select);
        status = PTR_RET(dev);
	} else {
		//pr_err(&spidev->spi->dev, "no minor number available!\n");
		spi_trace("no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&spidev->device_entry, &device_list);
	} else {
        spi_trace("device create error %d\n", status);
    }
	mutex_unlock(&device_list_lock);
}

static void clear_spi_class(struct spi_device *spi) 
{
    struct spidev_data	*spidev = spi_get_drvdata(spi);
    /* prevent new opens */
    mutex_lock(&device_list_lock);
    list_del(&spidev->device_entry);
    device_destroy(spidev_class, spidev->devt);
    clear_bit(MINOR(spidev->devt), minors);
    if (spidev->users == 0)
        kfree(spidev);
    mutex_unlock(&device_list_lock);
}

#endif

MODULE_DESCRIPTION("Driver for szzt,xr20m1172");
MODULE_LICENSE("GPL v2");

