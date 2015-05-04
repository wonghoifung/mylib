#include "base64.h"
#include <string.h>
#include <stdio.h>

const unsigned char* Base64::Alpha64Map = (const unsigned char*)"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

unsigned int Base64::GetEncode64Length(unsigned int len)
{
  return (8*len+5)/6;
}

unsigned int Base64::GetDecode64Length(unsigned int len)
{
  return (len * 6)/8;
}

bool Base64::Encode64(unsigned char *dest, const unsigned char *data, unsigned int max_dest, unsigned int byte_len)
{
  unsigned char sixbits;
  unsigned char fresh_bits_holder=0;

  unsigned int req_length = GetEncode64Length(byte_len);
  int bits_left= (6*req_length) - (8*byte_len);

  if(max_dest<=req_length)
    return false;
	
  while(1)
  {
    if(bits_left<6)
    {
      sixbits = (fresh_bits_holder << (6-bits_left)) & 0x3F;

      if(byte_len>0)
      {
        fresh_bits_holder=*data;
        --byte_len;
        ++data;

        sixbits |= fresh_bits_holder >> ( 8-(6-bits_left) );

        bits_left= 8 + bits_left - 6;
      }
      else if(bits_left<=0)
      {
        *dest=0;
        break;
      }
    }
    else
    {
      sixbits = (fresh_bits_holder >> (bits_left-6)) & 0x3F;
      bits_left -= 6;
      fresh_bits_holder >>= 6;
    }

    *dest++ = Alpha64Map[sixbits];
  }

  return true;
}

bool Base64::Decode64(unsigned char *dest, const unsigned char *b64data, unsigned int max_dest, unsigned int byte_len)
{
  unsigned int len = (byte_len? byte_len : strlen((const char*)b64data));
  unsigned int nbytes = GetDecode64Length(len);

  int bits_left= (8*nbytes) - (6*len);
  unsigned int bit_holder=0;

  if(max_dest < nbytes)
    return false;
	
  while( nbytes > 0)
  {
    while(bits_left<8)
    {
      bit_holder <<= 6;
			
      {
        unsigned char sixbits;

        if(*b64data<46)
          return false;
        else if(*b64data<=57)
          sixbits = *b64data - 46;
        else if(*b64data<65)
          return false;
        else if(*b64data<=90)
          sixbits = *b64data - 53;
        else if(*b64data<97)
          return false;
        else if(*b64data<=122)
          sixbits = *b64data - 59;
        else
          return false;

        bit_holder |= sixbits;
        bits_left += 6;
        ++b64data;
      }
				
    }

    bits_left -= 8;
    *dest = (unsigned char)(bit_holder>>bits_left);

    ++dest;
    --nbytes;
  }

  return true;
}


