// Base headers
#include <linux/init.h>           // Mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#include <linux/mutex.h>          // Required for the mutex functionality
#include <linux/stat.h>           // Contains flags for mudule params

// Hash headers
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/string.h>


#define  DEVICE_NAME "hyagoDev"   // The device will appear at /dev/hyagoDev using this value
#define  CLASS_NAME  "crypto"     // The device class -- this is a character device driver
#define  HASH_LENGTH 20
#define  CRYPTO_BUFFER_SIZE 32

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hyago & Co.");

static char  *cryptoKey = "key_default";    ///< Key to cipher/decipher a message
static int    majorNumber;                  ///< Store the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static struct class*  cryptoClass  = NULL;  ///< The device-driver CLASS struct pointer
static struct device* cryptoDevice = NULL;  ///< The device-driver DEVICE struct pointer

static DEFINE_MUTEX(crypto_mutex);        ///< Macro to declare a new mutex

module_param(cryptoKey, charp, S_IRUGO); ///< Macro to module params

/// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/// Prototype helper functions
static char genHash(char *hashMessage, int sizeMessage);
static void encrypt (u8 *pkey, char *msg);
static void decrypt (u8 *pkey, char *msg);

/**
 * Devices are represented as file structure in the kernel. The file_operations structure from
 * /linux/fs.h lists the callback functions that you wish to associated with your file operations
 * using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
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
static int __init crypto_init(void){
    printk(KERN_INFO "Crypto: Initializing the Crypto Module\n");
    
    // Try to dynamically allocate a major number for the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "Crypto failed to register a major number\n");
        return majorNumber;
    }
    
    printk(KERN_INFO "Crypto: registered correctly with major number %d\n", majorNumber);
    
    // Register the device class
    cryptoClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(cryptoClass)){           // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(cryptoClass);     // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "Crypto: device class registered correctly\n");
    
    // Register the device driver
    cryptoDevice = device_create(cryptoClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(cryptoDevice)){          // Clean up if there is an error
        class_destroy(cryptoClass);      // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(cryptoDevice);
    }
    printk(KERN_INFO "Crypto: device class created correctly\n"); // Made it! device was initialized
    mutex_init(&crypto_mutex);          // Initialize the mutex dynamically
    return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit crypto_exit(void) {
    mutex_destroy(&crypto_mutex);                       // destroy the dynamically-allocated mutex
    device_destroy(cryptoClass, MKDEV(majorNumber, 0)); // remove the device
    class_unregister(cryptoClass);                      // unregister the device class
    class_destroy(cryptoClass);                         // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);         // unregister the major number
    printk(KERN_INFO "Crypto: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
    
    if(!mutex_trylock(&crypto_mutex)){                  // Try to acquire the mutex (returns 0 on fail)
        printk(KERN_ALERT "Crypto: Device in use by another process");
        return -EBUSY;
    }
    
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
    
    if (error_count==0){           // success!
        printk(KERN_INFO "Crypto: Sent %d characters to the user\n", size_of_message);
        return (size_of_message=0); // clear the position to the start and return 0
    }
    else {
        printk(KERN_INFO "Crypto: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;      // Failed -- return a bad address message (i.e. -14)
    }
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using message[x] = buffer[x]
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    char option = buffer[0];
    u8 *key[CRYPTO_BUFFER_SIZE];
    
    memcpy(key, cryptoKey, sizeof(cryptoKey));
    sprintf(message, "%s", buffer);   // appending received string with its length
    memmove(message, message+1, strlen(message)); // Remove command from string
    size_of_message = strlen(message);                 // store the length of the stored message
    
    printk(KERN_INFO "Crypto: Message -> %s\n", message);
    switch (option) {
        case 'c':
	    encrypt(key, message);
            printk(KERN_INFO "Parte do Samuel\n");
            break;
        case 'd':
	    decrypt(key, message);
            printk(KERN_INFO "Parte do Rubens\n");
            break;
        default:
            genHash(message, strlen(message));
            break;
    }
    
    printk(KERN_INFO "Crypto: Received %zu characters from the user\n", len);
    return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
    mutex_unlock(&crypto_mutex);                      // release the mutex (i.e., lock goes up)
    printk(KERN_INFO "Crypto: Device successfully closed\n");
    return 0;
}

// Helper Functions
static char genHash(char *hashMessage, int sizeMessage) {
    struct scatterlist sg;
    struct crypto_hash *tfm;
    struct hash_desc desc;
    unsigned char output[HASH_LENGTH];
    unsigned char buf[sizeMessage];
    
    memcpy(buf, hashMessage, sizeMessage); // Copy message to a buffer
    memset(output, 0x00, HASH_LENGTH); // fill output with 0
    
    tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(tfm)) {
        printk(KERN_ERR "Crypto: tfm allocation failed\n");
        return '0';
    }
    
    desc.tfm = tfm;
    desc.flags = 0;
    
    crypto_hash_init(&desc);
    sg_init_one(&sg, buf, sizeMessage);
    crypto_hash_update(&desc, &sg, sizeMessage);
    crypto_hash_final(&desc, output);
    
    int i;
    unsigned char strOutput[(HASH_LENGTH*2) + 1];
    
    sprintf(strOutput, "%02x", output[i]);
    for (i = 1; i < HASH_LENGTH; i++) {
        sprintf(strOutput + strlen(strOutput), "%02x", output[i]);
    }
    
    printk(KERN_INFO "HASH: %s\n",strOutput); // Print hash created
    sprintf(message, "%s", strOutput); // Change message text
    size_of_message = strlen(message); // Store the length of the stored message
    
    crypto_free_hash(tfm);
    
    return '0';
}


/* 
Funcao de crypto
*/
static void encrypt (u8 *pkey, char *msg) {
	struct crypto_cipher *tmf;
        u8 dest[CRYPTO_BUFFER_SIZE];
	u8 dest2[CRYPTO_BUFFER_SIZE];
	char src[CRYPTO_BUFFER_SIZE];
	int i = 0;

	sprintf(src, "%s", msg); // Change message text
	tmf = crypto_alloc_cipher("aes", 4, 32);
        crypto_cipher_setkey(tmf, pkey, 32);

        crypto_cipher_encrypt_one(tmf, dest, src);
        crypto_cipher_encrypt_one(tmf, &dest[16], &src[16]);

	printk(KERN_INFO "CRYPT: %s\n", dest); // Print hash created
    	sprintf(message, "%s", dest); // Change message text
    	size_of_message = strlen(message); // Store the length of the stored message
	
	crypto_cipher_decrypt_one(tmf, dest2, dest);
        crypto_cipher_decrypt_one(tmf, &dest2[16], &dest[16]);

	for(i = 0; i < sizeof(dest2); i++) {
		pr_info("%c", dest2[i]);
	}

	crypto_free_cipher(tmf);
}

/*
Função de decrypto
*/
static void decrypt (u8 *pkey, char *buffer){
	struct crypto_cipher *tmf;
        u8 dest[CRYPTO_BUFFER_SIZE];
        u8 dest2[CRYPTO_BUFFER_SIZE];

	memcpy(dest, buffer, sizeof(buffer));

	tmf = crypto_alloc_cipher("aes", 4, 32);
        crypto_cipher_setkey(tmf, pkey, 32);

        crypto_cipher_decrypt_one(tmf, dest2, dest);
        crypto_cipher_decrypt_one(tmf, &dest2[16], &dest[16]);

	crypto_free_cipher(tmf);

	printk(KERN_INFO "DECRYPT: %s\n", dest2); // Print hash created
    	sprintf(message, "%s", dest); // Change message text
    	size_of_message = strlen(message); // Store the length of the stored message
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(crypto_init);
module_exit(crypto_exit);
