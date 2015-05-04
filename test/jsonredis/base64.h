#ifndef BASE64_HEADER
#define BASE64_HEADER

#include <stdlib.h>
#include <string>

class Base64
{
public:
  static const unsigned char* Alpha64Map;
  static unsigned int GetEncode64Length(unsigned int len);
  static unsigned int GetDecode64Length(unsigned int len);
  static bool Encode64(unsigned char *dest, const unsigned char *data, unsigned int max_dest, unsigned int byte_len);
  static bool Decode64(unsigned char *dest, const unsigned char *b64data, unsigned int max_dest, unsigned int byte_len = 0);
};

inline std::string base64encode(const std::string& str)
{
  unsigned enlen = Base64::GetEncode64Length(str.size());
  unsigned char* dest = (unsigned char*)malloc(enlen+1);
  memset(dest, 0, enlen+1);
  if (!Base64::Encode64(dest, (const unsigned char*)str.c_str(), enlen+1, str.size()))
  {
    free(dest);
    return ""; 
  }
  std::string ret((char*)dest);
  free(dest);
  return ret;
}

inline std::string base64decode(const std::string& str)
{
  unsigned delen = Base64::GetDecode64Length(str.size());
  unsigned char* dest = (unsigned char*)malloc(delen+1);
  memset(dest, 0, delen+1);
  if (!Base64::Decode64(dest, (const unsigned char*)str.c_str(), delen+1, str.size()))
  {
    free(dest);
    return ""; 
  }
  std::string ret((char*)dest);
  free(dest);
  return ret;
}

#endif

