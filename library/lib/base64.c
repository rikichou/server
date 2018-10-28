

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static const char * s_base64Char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * base64Encode( const unsigned char * binData, int binLength, char * base64)
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binLength ; i += 3 )
    {
        current = (binData[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = s_base64Char[(int)current];

        current = ( (unsigned char)(binData[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binLength )
        {
            base64[j++] = s_base64Char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(binData[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = s_base64Char[(int)current];

        current = ( (unsigned char)(binData[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binLength )
        {
            base64[j++] = s_base64Char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(binData[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = s_base64Char[(int)current];

        current = ( (unsigned char)binData[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = s_base64Char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64Decode( const char * base64, unsigned char * binData, int binLength)
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {   
        /* trim space */

        while (isspace(base64[i]))
        {
            i ++;    
        }
        
        if (base64[i] == '\0')
        {
            break;
        }
    
        memset( temp, 0xFF, sizeof(temp) );
                
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( s_base64Char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( s_base64Char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( s_base64Char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( s_base64Char[k] == base64[i+3] )
                temp[3]= k;
        }

        if (j >= binLength)
            break;
        binData[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        
        if ( base64[i+2] == '=' )
            break;

        if (j >= binLength)
            break;        
        binData[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        if (j >= binLength)
            break;
        binData[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }

    if (j < binLength)
    {
        binData[j] = 0;
    }
    
    return j;
}
