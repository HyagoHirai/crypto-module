
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#define SHA1_LENGTH     20


char* gerarhash(char *string,int tamString)
{

    struct scatterlist sg;
    struct crypto_hash *tfm;
    struct hash_desc desc;
    unsigned char output[SHA1_LENGTH];
    unsigned char buf[tamString];
    char hash[SHA1_LENGTH];
    char hashOut[2000];
    int i, j;
	
   
    //memset(buf, 'A', tamString);
    memcpy(buf, string, tamString);
    memset(output, 0x00, SHA1_LENGTH);
 
    tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(tfm)) {
    printk(KERN_ERR "tfm allocation failed\n");
    return 0;
    }
 
    desc.tfm = tfm;
    desc.flags = 0;

    for (j = 0; j < 1; j++) {
    crypto_hash_init(&desc);
    sg_init_one(&sg, buf, tamString);
    crypto_hash_update(&desc, &sg, tamString);
    crypto_hash_final(&desc, output);

    for (i = 0; i < SHA1_LENGTH; i++) {

	sprintf(hash, "%02x", output[i]);
	strcat(hashOut,hash);
	//printk(KERN_ERR "%02x", output[i]);
	//printk(KERN_INFO "HASH:%s\n",hash);
	
    }
   }
    crypto_free_hash(tfm);

    return hashOut;
}

static int __init sha1_init(void)
{	

	char *string = "Sistema Operacional";
	char *result;
	result = gerarhash(string,strlen(string));
	printk(KERN_INFO "HASH:%s\n",result);
}

static void __exit sha1_exit(void)
{
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
}

module_init(sha1_init);
module_exit(sha1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robson");
