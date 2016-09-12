/**
*******************************************************************************
* @file lkm_kdb_irq.c
* @author Keidan
* @par Project lkm_kdb_irq
* @copyright Copyright 2015 Keidan, all right reserved.
* @par License:
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY.
*
* Licence summary : 
*    You can modify and redistribute the sources code and binaries.
*    You can send me the bug-fix
*
* Term of the licence in in the file licence.txt.
*
*  ____  __.________ __________     ._____________ ________   
* |    |/ _|\______ \\______   \    |   \______   \\_____  \  
* |      <   |    |  \|    |  _/    |   ||       _/ /  / \  \ 
* |    |  \  |    `   \    |   \    |   ||    |   \/   \_/.  \
* |____|__ \/_______  /______  /____|___||____|_  /\_____\ \_/
*         \/        \/       \/_____/           \/        \__>
*
*******************************************************************************
*/
#include <linux/init.h>           /* Macros used to mark up functions e.g. __init __exit */
#include <linux/module.h>         /* Core header for loading LKMs into the kernel */
#include <linux/kernel.h>         /* Contains types, macros, functions for the kernel */
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/slab.h>           /* kmalloc */
#include <linux/interrupt.h>	  /* We want an interrupt */
#include <asm/io.h>
#include <asm/irq_vectors.h>

/* Keyboard IRQ on intel arch */
#define LKM_KDB_IRQ_NUM 1
/* Workqueue name */
#define LKM_KDB_IRQ_WQ_NAME "lkm_kdb_irq"
/*
 * The keyboard controller has three 8-bit registers involved in communication with the CPU: its input buffer,
 * that can be written by the CPU by writing port 0x60 or port 0x64; its output buffer,
 * that can be read by the CPU by reading from port 0x60; and the status register, that can be read by the CPU by reading from port 0x64.
 *
 * The keyboard controller is provided with some RAM, for example 32 bytes, that can be accessed by the CPU.
 * The most important part of this RAM is byte 0, the Controller Command Byte (CCB).
 * It can be read/written by writing 0x20/0x60 to port 0x64 and then reading/writing a data byte from/to port 0x60.
 *
 * see: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html
 */
#define LKM_KDB_STATUS   0x64
#define LKM_KDB_SCANCODE 0x60


/** Make the structure to be passed to the workqueue handler**/
typedef struct {
    struct work_struct w;
    unsigned char scancode;
} lkm_kdb_irq_task_t;

/** Declare the workqueue struct **/
static struct workqueue_struct *lkm_kdb_irq_wq = NULL;

/** kdb_irq_t instance **/
lkm_kdb_irq_task_t *work;


/**
 * This will get called by the kernel as soon as it's safe to do everything normally allowed by kernel modules.
 */
static void lkm_kdb_irq_got_char(struct work_struct *w) {
  lkm_kdb_irq_task_t *work = container_of(w, lkm_kdb_irq_task_t, w);
  int scancode = work->scancode & 0x7F;
  char released = work->scancode & 0x80 ? 1 : 0;
  printk(KERN_INFO "Scan Code %x %s.\n",
	 scancode, released ? "Released" : "Pressed");
}

/**
 * Reads the relevant information from the keyboard IRQ handler and then puts the non RT part into the work queue.
 * This will be run when the kernel considers it safe.
 */
irqreturn_t lkm_kdb_irq_handler(int irq, void *dev_id) {
  static unsigned char scancode;
  unsigned char status;

  /* Read keyboard status */
  status = inb(LKM_KDB_STATUS);
  scancode = inb(LKM_KDB_SCANCODE);

  work->scancode = scancode;
  queue_work(lkm_kdb_irq_wq, &work->w);
  return IRQ_HANDLED;
}

/**
 * Initialize the module - register the IRQ handler 
 * @brief The LKM initialization function - register the IRQ handler.
 * @return returns 0 if successful
 */
static int __init lkm_kdb_irq_init(void) {

  lkm_kdb_irq_wq = create_workqueue(LKM_KDB_IRQ_WQ_NAME);
  work = (lkm_kdb_irq_task_t *)kmalloc(sizeof(lkm_kdb_irq_task_t), GFP_KERNEL);
  if (work) {
    INIT_WORK(&work->w, lkm_kdb_irq_got_char);
  }
  /*
   * Request IRQ 1 (keyboard IRQ), and register the callback function to the associated IRQ handler.
   * SA_SHIRQ: Means we're willing to have other handlers on this IRQ.
   * SA_INTERRUPT: Can be used to make the handler into a fast interrupt.
   */
  return request_irq(LKM_KDB_IRQ_NUM, /* The number of the keyboard IRQ on PCs */
		     lkm_kdb_irq_handler, /* module handler */
		     IRQF_SHARED, /* Means we're willing to have other handlers on this IRQ. */
		     "lkm_kdb_irq_handler", /* Shortname displayed into  /proc/interrupts. */
		     (void*)work); /* This parameter can point to anything but it shouldn't be NULL, 
						finally it's important to pass the same pointer value to the free_irq function. */
}


/**
 * @brief The LKM cleanup function.
 */
static void __exit lkm_kdb_irq_exit(void) {  
  /* cleanup workqueue resourses */
  flush_workqueue(lkm_kdb_irq_wq);
  destroy_workqueue(lkm_kdb_irq_wq);
  kfree((void *)work);
  /* 
   * In this case, the instruction below it's totally useless, 
   * because we can't restore the previous keyboard handler, and that the computer should be rebooted.
   */
  free_irq(LKM_KDB_IRQ_NUM, NULL);
}

/****************************************************
 *    _____             .___    .__           .__       .__  __   
 *   /     \   ____   __| _/_ __|  |   ____   |__| ____ |__|/  |_ 
 *  /  \ /  \ /  _ \ / __ |  |  \  | _/ __ \  |  |/    \|  \   __\
 * /    Y    (  <_> ) /_/ |  |  /  |_\  ___/  |  |   |  \  ||  |  
 * \____|__  /\____/\____ |____/|____/\___  > |__|___|  /__||__|  
 *         \/            \/               \/          \/    
 *****************************************************/
/** 
 * @brief A module must use the module_init() module_exit() macros from linux/init.h, which 
 * identify the initialization function at insertion time and the cleanup function (as listed above)
 */
module_init(lkm_kdb_irq_init);
module_exit(lkm_kdb_irq_exit);

/* module infos */
/* some work_queue related functions are just available to GPL licensed Modules */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Keidan (Kevin Billonneau)");
MODULE_DESCRIPTION("Simple LKM keyboard IRQ handler.");
