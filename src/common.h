#ifndef BOYAA_COMMON_H_20110313
#define BOYAA_COMMON_H_20110313

//公共的头文件,windows与linux的不同定义
#ifdef WIN32
#define ioctl ioctlsocket

#else
//#define TCP_NODELAY 0x01
typedef int SOCKET;

#endif


#endif



