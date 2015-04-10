#ifndef __POP_DEFINE_H_
#define __POP_DEFINE_H_


#ifndef WIN32
#include <sys/types.h>
#endif

#define MAX_QUERY_LEN 2048


#if _WIN32
typedef __int64            int64;
typedef long               int32;
typedef short              int16;
typedef signed char        int8;
typedef unsigned __int64   uint64;
typedef unsigned long      uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
#else
typedef __int64_t   int64;
typedef __int32_t   int32;
typedef __int16_t   int16;
typedef __int8_t    int8;
typedef __uint64_t  uint64;
typedef __uint32_t  uint32;
typedef __uint16_t  uint16;
typedef __uint8_t   uint8;
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef unsigned char	BYTE;

#ifndef WIN32
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned short	WORD;
typedef void*           HANDLE;
typedef unsigned int    UINT;
typedef void*			PVOID;
typedef unsigned char   BOOLEAN;
typedef void			VOID;
typedef	unsigned int*	PULONG;
typedef UCHAR* PUCHAR;
typedef long			LONG;
typedef DWORD ADAPTER_HANDLE;
typedef char			CHAR;
typedef unsigned char	BYTE;
typedef long BOOL;
typedef unsigned long   LRESULT;
typedef long HRESULT;
typedef char TCHAR;
typedef unsigned char _TUCHAR;
typedef const TCHAR* LPCTSTR;
typedef TCHAR* LPTSTR;
typedef CHAR* LPSTR;
typedef const CHAR* LPCSTR;
typedef int SOCKET;
typedef unsigned long	netip_t;		//网络字节序的ip
typedef unsigned long	hostip_t;		//主机字节序的ip
typedef unsigned short	netport_t;		//网络字节序的port
typedef unsigned short	hostport_t;		//主机字节序的port
typedef unsigned char	byte_t;
typedef byte_t byte;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL				*PBOOL;
typedef BOOL	            *LPBOOL;
typedef BYTE				*PBYTE;
typedef BYTE	            *LPBYTE;
typedef int		            *PINT;
typedef int		            *LPINT;
typedef WORD				*PWORD;
typedef WORD	            *LPWORD;
typedef long	            *LPLONG;
typedef DWORD				*PDWORD;
typedef DWORD				*LPDWORD;
typedef void	            *LPVOID;
typedef const void			*LPCVOID;
typedef int                 INT;
typedef unsigned int        *PUINT;
typedef long				LONG_PTR;
typedef unsigned long		ULONG_PTR;

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef u_short n_short;		/* short as received from the net */
typedef u_long	n_long;			/* long as received from the net */
typedef	u_long	n_time;	   /* ms since 00:00 GMT, byte rev */

#define HIWORD(l) ((WORD)((DWORD_PTR)(l) >> 16))
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define MAKEWORD(low,high) (((unsigned short)((unsigned char)low))|(((unsigned short)((unsigned char)high))<<8))

#else
    typedef int socklen_t;
#endif //#ifndef PLAT_WIN


#endif

