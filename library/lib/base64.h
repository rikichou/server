
#ifndef __BASE64_H__
#define __BASE64_H__
char * base64Encode( const unsigned char * bindata, int binlength, char * base64);
int base64Decode( const char * base64, unsigned char * binData, int binLength);

#endif  /* __BASE64_H__ */    

