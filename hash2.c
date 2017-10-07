#include <linuxernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>

#define SHA1_LENGTH     20
#define TAM     10



static char *string = "HyagoHirai"; 

void gerarhash(void)
{

    struct scatterlist sg;
    struct crypto_hash *tfm;
    struct hash_desc desc;
    unsigned char output[SHA1_LENGTH];
    unsigned char buf[10];
    int i, j;
 
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
    printk(KERN_INFO "MSG: %s\n",string);
    printk(KERN_INFO "MSG: %i\n",strlen(string)); 
    //memset(buf, 'A', 10);
    memcpy(buf,string,TAM);
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

    for (j = 0; j < 3; j++) {
    crypto_hash_init(&desc);
    sg_init_one(&sg, buf, TAM);
    crypto_hash_update(&desc, &sg, TAM);
    crypto_hash_final(&desc, output);

    for (i = 0; i < SHA1_LENGTH; i++) {
        printk(KERN_ERR "%02x", output[i]);
    }
    printk(KERN_INFO "\n---------------\n");
    memset(output, 0x00, SHA1_LENGTH);
    }

    crypto_free_hash(tfm);

    return 0;


}

static int __init sha1_init(void)
{
	gerarhash();
}

static void __exit sha1_exit(void)
{
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
}

module_init(sha1_init);
module_exit(sha1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robson");
