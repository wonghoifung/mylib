#ifndef BOYAA_COMMON_H_20110313
#define BOYAA_COMMON_H_20110313

//������ͷ�ļ�,windows��linux�Ĳ�ͬ����
#ifdef WIN32
#define ioctl ioctlsocket

#else
//#define TCP_NODELAY 0x01
typedef int SOCKET;

#endif


#endif



