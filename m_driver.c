/*Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <asm/cnt32_to_63.h>
#include <asm/div64.h>
#include <asm-arm/arch/hardware.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/udc.h>
#include <asm/arch/pxafb.h>
#include <asm/arch/mmc.h>
#include <asm/arch/irda.h>
#include <asm/arch/i2c.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
MODULE_LICENSE("Dual BSD/GPL");
#define MAX_COOKIE_LENGTH       PAGE_SIZE

/*Structure to Store Timer Details*/
int flag = 0;
int pid;	// Saves User Level PID
char name_cmm[20];	//Saves User Level Name
char star[128];
char update[500];

/* Declaration of Functions */
static int timer_open(struct inode *inode, struct file *filp);
static int timer_release(struct inode *inode, struct file *filp);
static ssize_t timer_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos);
static void timer_exit(void);
static int timer_init(void);

static struct timer_list mytimer;
void  my_timer_callback(unsigned long data);	//Call Back Function When Timer Expires
unsigned long init_load;
/* Structure that Declares File Operations*/
struct file_operations timer_fops = {
	write: timer_write,
	open: timer_open,
	release: timer_release,
};

/* Declaration of the init and exit functions */
module_init(timer_init);
module_exit(timer_exit);

/*Asynchronous Readers*/
static unsigned capacity = 128;
char *t;

/* Major number */
static int timer_major = 61;

/* Buffer to store data */
static char *timer_buffer;

/* length of the current message */
static int timer_len;
char *temp_name;

//****************************CALL BACK FUNCTION***************************************************
void my_timer_callback(unsigned long data)
{
	/****************SETTING ALL GPIO PINS TO LOW********************/
	pxa_gpio_set_value(118,0);
	pxa_gpio_set_value(117,0);
	pxa_gpio_set_value(16,0);
	pxa_gpio_set_value(17,0);
	pxa_gpio_set_value(101,0);
	pxa_gpio_set_value(2,0);
	pxa_gpio_set_value(28,0);
	pxa_gpio_set_value(29,0);
	pxa_gpio_set_value(9,0);
	pxa_gpio_set_value(30,0);
	pxa_gpio_set_value(113,0);
	pxa_gpio_set_value(31,0);
	del_timer(&mytimer);			//DELETING TIMER
}

//****************************INIT FUNCTION****************************************************
static int timer_init(void)
{
	int result;

	/* Registering device */
	result = register_chrdev(timer_major, "motor", &timer_fops);
	if (result < 0)
	{
		return result;
	}
	
	
	/* Allocating timer for the buffer */
	timer_buffer = kmalloc(capacity, GFP_KERNEL); 
	if (!timer_buffer)
	{ 
		result = -ENOMEM;
		goto fail; 
	} 
	memset(timer_buffer, 0, capacity);
	timer_len = 0;
	/****************************SETTING DIRECTION OF GPIO PINS TO OUTPUT*******************************/
	gpio_direction_output(118,1);
	gpio_direction_output(117,1);
	gpio_direction_output(16,1);
	gpio_direction_output(17,1);
	gpio_direction_output(101,1);
	gpio_direction_output(2,1);
	gpio_direction_output(28,1);
	gpio_direction_output(29,1);
	gpio_direction_output(9,1);
	gpio_direction_output(30,1);
	gpio_direction_output(113,1);
	gpio_direction_output(31,1);

	/****************SETTING ALL GPIO PINS TO LOW********************/
	pxa_gpio_set_value(118,0);
	pxa_gpio_set_value(117,0);
	pxa_gpio_set_value(16,0);
	pxa_gpio_set_value(17,0);
	pxa_gpio_set_value(101,0);
	pxa_gpio_set_value(2,0);
	pxa_gpio_set_value(28,0);
	pxa_gpio_set_value(29,0);
	pxa_gpio_set_value(9,0);
	pxa_gpio_set_value(30,0);
	pxa_gpio_set_value(113,0);
	pxa_gpio_set_value(31,0);
	return 0;

fail: 
	timer_exit(); 
	return result;
}

//**********************EXIT FUNCTION*********************************************************
static void timer_exit(void)
{
	/* Freeing the major number */
	del_timer(&mytimer);			//DELETING THE TIMER
	
	/****************SETTING ALL GPIO PINS TO LOW********************/
	pxa_gpio_set_value(118,0);
	pxa_gpio_set_value(117,0);
	pxa_gpio_set_value(16,0);
	pxa_gpio_set_value(17,0);
	pxa_gpio_set_value(101,0);
	pxa_gpio_set_value(2,0);
	pxa_gpio_set_value(28,0);
	pxa_gpio_set_value(29,0);
	pxa_gpio_set_value(9,0);
	pxa_gpio_set_value(30,0);
	pxa_gpio_set_value(113,0);
	pxa_gpio_set_value(31,0);
	/****************************SETTING DIRECTION OF GPIO PINS TO INPUT*******************************/
	gpio_direction_output(118,0);
	gpio_direction_output(117,0);
	gpio_direction_output(16,0);
	gpio_direction_output(17,0);
	gpio_direction_output(101,0);
	gpio_direction_output(2,0);
	gpio_direction_output(28,0);
	gpio_direction_output(29,0);
	gpio_direction_output(9,0);
	gpio_direction_output(30,0);
	gpio_direction_output(113,0);
	gpio_direction_output(31,0);
	unregister_chrdev(timer_major, "motor");
	/* Freeing buffer memory */
	if (timer_buffer)
	{
		kfree(timer_buffer);
	}
}

//**********************OPEN FUNCTION***********************************************************
static int timer_open(struct inode *inode, struct file *filp)
{
	/* Success */
	return 0;
}

//***********************RELEASE FUNCTION**********************************************************
static int timer_release(struct inode *inode, struct file *filp)
{
	/* Success */
	return 0;
}

//*************************TIMER WRITE FUNCTION************************************
static ssize_t timer_write(struct file *filp,const char *buf,size_t count,loff_t *f_pos)
{
	int temp;
	char tbuf[128];
	unsigned long time_jiffies;
	int time_sec;
	char *name;
	char *tbptr = tbuf;
	
	/* end of buffer reached */
	if (*f_pos >= capacity)
	{
		return -ENOSPC;
	}

	if (copy_from_user(timer_buffer + *f_pos, buf, count))
	{
		return -EFAULT;
	}
	printk("%s\n",timer_buffer);

	for (temp = *f_pos; temp < count + *f_pos; temp++)
		tbptr += sprintf(tbptr, "%c", timer_buffer[temp]);
	time_sec = (int)simple_strtoul(tbuf,&name,0);			//USED TO GET TIME IN MILLISECONDS
	time_jiffies = jiffies + msecs_to_jiffies(time_sec);		//CONVERTING MILLISECONDS TO JIFFIES
	setup_timer(&mytimer, my_timer_callback,0);			//TIMER SETPUT
	/*******************SETTING VALUE OF GPIO PINS TO RUN THE MOTOR**************/
	pxa_gpio_set_value(118,1);
	pxa_gpio_set_value(117,0);
	pxa_gpio_set_value(16,0);
	pxa_gpio_set_value(17,1);
	pxa_gpio_set_value(101,1);
	pxa_gpio_set_value(2,1);
	pxa_gpio_set_value(28,1);
	pxa_gpio_set_value(29,1);
	pxa_gpio_set_value(9,1);
	pxa_gpio_set_value(30,1);
	pxa_gpio_set_value(113,0);
	pxa_gpio_set_value(31,0);
	mod_timer(&mytimer,time_jiffies);				//STARTING THE TIMER
	*f_pos += count;
	timer_len = *f_pos;
	return count;
}
