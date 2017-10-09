
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
 
#define SHA1_LENGTH     20

 
/*void concatenate_string(char *original, char *add, char *result)
{
    while(*original)
    {
       *result = *original;
       original++;
       result++;
    }
    while(*add)
    {
       *result = *add;
       add++;
       result++;
    }
    *result = '\0';
}*/

char gerarhash(char *string,int tamString)
{

    struct scatterlist sg;
    struct crypto_hash *tfm;
    struct hash_desc desc;
    unsigned char output[SHA1_LENGTH];
    unsigned char buf[tamString];
    unsigned char hash[SHA1_LENGTH];
    char hashOut[SHA1_LENGTH];
   
    int i, j;
	
	
 
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
 
    //memset(buf, 'A', tamString);
    memcpy(buf, string, tamString);
    memset(output, 0x00, SHA1_LENGTH);
 
    tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(tfm)) {
    printk(KERN_ERR "daveti: tfm allocation failed\n");
    return 0;
    }
 
    desc.tfm = tfm;
    desc.flags = 0;

    //crypto_hash_init(&desc);
    //daveti: NOTE, crypto_hash_init is needed
    //for every new hasing!

    for (j = 0; j < 1; j++) {
    crypto_hash_init(&desc);
    sg_init_one(&sg, buf, tamString);
    crypto_hash_update(&desc, &sg, tamString);
    crypto_hash_final(&desc, output);

    for (i = 0; i < SHA1_LENGTH; i++) {

	printk(KERN_ERR "%02x", output[i]);
	sprintf(hash, "%02x", output[i]);
	printk(KERN_INFO "HASH:%s\n",hash);
	
	
	/*concatenate_string(hashOut,hash,hashOut);
	printk(KERN_INFO "HASH:%s\n",hashOut);*/
	
    }
    printk(KERN_INFO "\n---------------\n");
    memset(output, 0x00, SHA1_LENGTH);
    }



    crypto_free_hash(tfm);

    return 0;
}

static int __init sha1_init(void)
{	

	char *string = "Robson";
	char *result;
	result = gerarhash(string,strlen(string));
	//printk(KERN_INFO "HASH:%s\n",result);
}

static void __exit sha1_exit(void)
{
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
}

module_init(sha1_init);
module_exit(sha1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robson");

