#include <linux/init.h>           
#include <linux/module.h>         
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <linux/uaccess.h>          
#define  DEVICE_NAME "nixieChar"    
#define  CLASS_NAME  "nixie"

#include "nixieGPIO.c"

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Bernard Lee");    
MODULE_DESCRIPTION("Linux driver for Nixie tubes via SPI");  
MODULE_VERSION("0.1");            

static int    majorNumber;                  
static char   message[256] = {0};           
static short  size_of_message;              
static int    numberOpens = 0;              
static struct class*  nixiecharClass  = NULL; 
static struct device* nixiecharDevice = NULL; 

static int spiOutputHr;
static int spiOutputMin;
int digitToHex(char input, int start);


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
static int __init nixiechar_init(void){

   printk(KERN_INFO "NixieChar: Initializing PD14 & PD15 communication bus\n");
   init_gpio();
   
   printk(KERN_INFO "NixieChar: Initializing the NixieChar LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "NixieChar failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "NixieChar: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   nixiecharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(nixiecharClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(nixiecharClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "NixieChar: device class registered correctly\n");

   // Register the device driver
   nixiecharDevice = device_create(nixiecharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(nixiecharDevice)){               // Clean up if there is an error
      class_destroy(nixiecharClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(nixiecharDevice);
   }
   printk(KERN_INFO "NixieChar: device class created correctly\n"); // Made it! device was initialized
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit nixiechar_exit(void){
   device_destroy(nixiecharClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(nixiecharClass);                          // unregister the device class
   class_destroy(nixiecharClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   // close_gpio();
   printk(KERN_INFO "NixieChar: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "NixieChar: Device has been opened %d time(s)\n", numberOpens);
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
      printk(KERN_INFO "NixieChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "NixieChar: Failed to send %d characters to the user\n", error_count);
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
             

   if(len >= 3 && (buffer[2] == ':' || buffer[2] == ' ')){
         int valueHr1 = digitToHex(buffer[0], 2);
         int valueHr0 = digitToHex(buffer[1], 9);
         int valueMin1 = digitToHex(buffer[3], 5);
         int valueMin0 = digitToHex(buffer[4], 9);
         
         spiOutputHr = 0;
         spiOutputMin = 0;

         spiOutputHr = spiOutputHr | (valueHr1 << 10);
         printk(KERN_INFO "NixieChar: Output1: %x\n", spiOutputHr);
         spiOutputHr = spiOutputHr | (valueHr0);
         printk(KERN_INFO "NixieChar: Output2: %x\n", spiOutputHr);
         spiOutputMin = spiOutputMin | (valueMin1 << 10);
         spiOutputMin = spiOutputMin | (valueMin0);

         if(buffer[2] == ':'){
            spiOutputHr = spiOutputHr | 0x2000;
         }

         printk(KERN_INFO "NixieChar: Output to SPI: %x \t %x\n", spiOutputHr, spiOutputMin);
         printk(KERN_INFO "NixieChar: Hr Output %x \t %x\n", (valueHr1 << 10), valueHr0);
         printk(KERN_INFO "NixieChar: Hr Output %x \t %x\n", (valueMin1 << 10), valueMin0);

         transfer_spi(spiOutputMin, 16);
         transfer_spi(spiOutputHr, 16);
                 
   }

   printk(KERN_INFO "NixieChar: Received %zu characters from the user\n", len);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "NixieChar: Device successfully closed\n");
   return 0;
}

int digitToHex(char input, int start){
    return 0x01 << (start - ((int)input - '0'));
}


/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(nixiechar_init);
module_exit(nixiechar_exit);
