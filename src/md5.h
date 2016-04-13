
#ifndef _MD5_H_
#define _MD5_H_

#ifndef _MD5_C_
#define MD5_EXT extern 
#else 
#define MD5_EXT
#endif 


#ifndef MAX_READ_LINE
#define MAX_READ_LINE 1024
#endif



typedef struct  
{  
    unsigned int count[2];  
    unsigned int state[4];  
    unsigned char buffer[64];     
}MD5_CTX;  
   
                           
#define F(x,y,z) ((x & y) | (~x & z))  
#define G(x,y,z) ((x & z) | (y & ~z))  
#define H(x,y,z) (x^y^z)  
#define I(x,y,z) (y ^ (x | ~z))  
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))  
#define FF(a,b,c,d,x,s,ac) \
          { \
          a += F(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define GG(a,b,c,d,x,s,ac) \
          { \
          a += G(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define HH(a,b,c,d,x,s,ac) \
          { \
          a += H(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define II(a,b,c,d,x,s,ac) \
          { \
          a += I(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }                                      
MD5_EXT void MD5Init(MD5_CTX *context);  
MD5_EXT void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);  
MD5_EXT void MD5Final(MD5_CTX *context,unsigned char digest[16]);  
MD5_EXT void MD5Transform(unsigned int state[4],unsigned char block[64]);  
MD5_EXT void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);  
MD5_EXT void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);  
MD5_EXT int CalcFileMD5(const char *pathname, char *md5);
MD5_EXT int CalcStringMD5(const char *string, char *md5);


#endif  




