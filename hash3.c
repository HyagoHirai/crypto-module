#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

struct scatterlist sg; 
struct hash_desc desc; 

char *plaintext = "test"; 
size_t len = strlen(plaintext); 
u8 hashval[20]; 


int myarr[4] = { 1, 3, 3, 7 }; 
size_t len = sizeof(myarr); 

sg_init_one(&sg, plaintext, len); 
desc.tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC); 


crypto_hash_init(&desc); 
crypto_hash_update(&desc, &sg, len); 
crypto_hash_final(&desc, hashval); 


crypto_free_hash(desc.tfm); 
