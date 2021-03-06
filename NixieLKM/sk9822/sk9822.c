#include <linux/init.h>           
#include <linux/module.h>         
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <linux/uaccess.h>          

#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>

static struct hrtimer htimer;
static ktime_t kt_period;

#define  DEVICE_NAME "sk9822"    
#define  CLASS_NAME  "WS2812B"

#include "lightGPIO.c"

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Bernard Lee");    
MODULE_DESCRIPTION("Linux driver for WS2812B");  
MODULE_VERSION("0.1");            

static int    majorNumber;                  
static char   message[256] = {0};           
static short  size_of_message;              
static short  callback_count;
static int    numberOpens = 0;              
static struct class*  lightingModClass  = NULL; 
static struct device* lightingModDevice = NULL; 

#define STATE_OFF 0
#define STATE_BOOT_UP 1
#define STATE_BOOT_UP_2 2
#define STATE_BOOT_SUCCESS 3
#define STATE_BOOT_IDLE 4
#define STATE_BOOT_FAILED -1

static int state_of_light = STATE_BOOT_UP_2;


static void timer_init(void);
static void timer_cleanup(void);
static enum hrtimer_restart timer_function(struct hrtimer * timer);


// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);


static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init lightingMod_init(void){

   printk(KERN_INFO "LightingMod: Initializing PG6 and PG7 communication bus\n");
   init_gpio();
   printk(KERN_INFO "LightingMod: Initializing the LightingMod LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "LightingMod failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "LightingMod: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   lightingModClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(lightingModClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(lightingModClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "LightingMod: device class registered correctly\n");

   // Register the device driver
   lightingModDevice = device_create(lightingModClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(lightingModDevice)){               // Clean up if there is an error
      class_destroy(lightingModClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(lightingModDevice);
   }
   printk(KERN_INFO "LightingMod: device class created correctly\n"); // Made it! device was initialized

   printk(KERN_INFO "LightingMod: Timer ISR setup...\n"); // Made it! device was initialized
   timer_init();
   printk(KERN_INFO "LightingMod: Done\n"); // Made it! device was initialized
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit lightingMod_exit(void){
   device_destroy(lightingModClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(lightingModClass);                          // unregister the device class
   class_destroy(lightingModClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   close_gpio();
   timer_cleanup();
   printk(KERN_INFO "LightingMod: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "LightingMod: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "LightingMod: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "LightingMod: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){   
   size_of_message = strlen(message);             
   printk(KERN_INFO "LightingMod: Received %zu characters from the user: %d %d\n", len, buffer[0], buffer[1]);


   if(buffer[0] == 49){
      state_of_light = STATE_BOOT_UP;
      setAll(0,0,0);
      printk(KERN_INFO "LightingMod: State Change: Code %d\n", state_of_light);
   }  else if(buffer[0] == 50){
      setAll(0,0,0);
      state_of_light = STATE_BOOT_UP_2;
      printk(KERN_INFO "LightingMod: State Change: Code %d\n", state_of_light);
   } else if(buffer[0] == 51){
      setAll(0,0,0);
      state_of_light = STATE_BOOT_SUCCESS;
      printk(KERN_INFO "LightingMod: State Change: Code %d\n", state_of_light);
   } else if(buffer[0] == 52){
      setAll(0,0,0);
      state_of_light = STATE_BOOT_FAILED;
      printk(KERN_INFO "LightingMod: State Change: Code %d\n", state_of_light);
   } else {
      setAll(0,0,0);
      state_of_light = STATE_OFF;
      printk(KERN_INFO "LightingMod: State Change: Code %d\n", state_of_light);
   }

   return len;

}
  

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "LightingMod: Device successfully closed\n");
   return 0;
}

static void timer_init(void)
{
    kt_period = ktime_set(0, 30000000); //seconds,nanoseconds
    hrtimer_init (& htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    htimer.function = timer_function;
    hrtimer_start(& htimer, kt_period, HRTIMER_MODE_REL);
}

static void timer_cleanup(void)
{
    hrtimer_cancel(& htimer);
}

int initCall = 0;

static enum hrtimer_restart timer_function(struct hrtimer * timer)
{
    if(state_of_light == STATE_BOOT_UP){
       loading2();
    } else if(state_of_light == STATE_BOOT_UP_2){
       loading();
    } else if(state_of_light == STATE_BOOT_SUCCESS){
      fadeIn(100,0,0);
       state_of_light = STATE_BOOT_IDLE;
    } else if(state_of_light == STATE_BOOT_FAILED){
       pulsing();
    } else if(state_of_light == STATE_BOOT_IDLE){
       setAll(100, 0, 0);
    } else {
       setAll(0,0,0);
    }
    
    hrtimer_forward_now(timer, kt_period);

    return HRTIMER_RESTART;
}


/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(lightingMod_init);
module_exit(lightingMod_exit);
