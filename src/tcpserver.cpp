
#include "tcpserver.h"
#include "timer.h"
#include "llist.h"
#include "log.h"
#include <time.h>
#include <assert.h>
#include "socketops.h"

#define EVENT_TOTAL_COUNT	100000
// 10w ������
#define MAX_DESCRIPTORS     100000

#ifdef WIN32 
#pragma comment(lib,"ws2_32.lib")
#else
#include <signal.h>
#endif

//��̬��Ա
bool TcpServer::m_bRun = true;			//�Ƿ���Լ�������

TcpServer::TcpServer()
{
#ifdef WIN32
	m_maxfd = 0;
	FD_ZERO(&m_rset);
	FD_ZERO(&m_wset);
#endif // WIN32   
    m_count_fd = 0;
    m_fd_index = 0;
    fds = NULL;
}

TcpServer::~TcpServer()
{	
    socketops::myclose(m_listen_fd);

#ifndef WIN32
    socketops::myclose(m_epoll_fd);
    free(m_epev_arr);
#endif
    free(fds);  
}

#ifndef WIN32
void TcpServer::SigHandle(int signum)
{
    if(signum == SIGTERM || signum == SIGUSR1 || signum == SIGKILL)	//��ֹ����
    {
        log_error("recv signal: %d  will kill down \n",signum);
        TcpServer::m_bRun = false;  
    } 
}
#endif

bool TcpServer::InitSocket(int listen_port)
{
	__INIT_NET_ENVIR__
	m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listen_fd == INVALID_SOCKET)
		return false;

    socketops::set_reuse(m_listen_fd);
	socketops::set_nonblock(m_listen_fd);

    int ret = socketops::mylisten(m_listen_fd,listen_port);
    if(ret < 0)
        return false;

	if( !_InitEvent())
        return false;

	log_debug("Server start running, listen port:%d\n", listen_port);
	return true;	
}

bool TcpServer::Run()
{
#ifdef WIN32
	return _StartUpWin();
#else
	return _StartUpLin();
#endif
}

bool TcpServer::_InitEvent()
{
    fds =  (TcpHandler**)malloc(MAX_DESCRIPTORS * sizeof(void*));
    memset(fds,0,MAX_DESCRIPTORS * sizeof(void*));

#ifdef WIN32
	FD_SET(m_listen_fd,&m_rset);
	m_maxfd = (m_listen_fd > m_maxfd) ? m_listen_fd : m_maxfd;
#else
	struct rlimit rl;
	int nfiles = MAX_DESCRIPTORS;
	if (getrlimit(RLIMIT_NOFILE, &rl) == 0 &&
		rl.rlim_cur != RLIM_INFINITY) {
			nfiles = rl.rlim_cur - 1;
	}
    if( nfiles > MAX_DESCRIPTORS )
        nfiles = MAX_DESCRIPTORS;

	//log_print(nfiles);

	log_debug("epoll create files:%d\n", nfiles);
	if (-1 == (m_epoll_fd = epoll_create(nfiles)))
		return false;
	
    // ��listen����ETģʽ������socket���д���ʽ��ETģʽЧ��һ��
	struct epoll_event ev;
	ev.data.fd = m_listen_fd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listen_fd, &ev);

	m_epev_arr = (struct epoll_event*)malloc(EVENT_TOTAL_COUNT * sizeof(struct epoll_event));

    //ע���źŴ�����
    signal(SIGTERM, TcpServer::SigHandle);    	
    signal(SIGUSR1,TcpServer::SigHandle);
	signal(SIGKILL,TcpServer::SigHandle);
#endif 
	return true;
}


bool TcpServer::_StartUpWin()
{
#ifdef WIN32
	struct timeval tv = {1, 000000};
	while(m_bRun)
	{
		m_tmp_rset = m_rset;
		m_tmp_wset = m_wset;
		int ready = select(m_maxfd+1, &m_tmp_rset, &m_tmp_wset, NULL, &tv);				
       
		if(ready == -1)
		{
			int err = WSAGetLastError();
			if (err != EINTR)
			{
				return false;
			}	
		}
		else if (ready == 0)	
		{
            run_timer();
			continue;
		}
		else
		{
            run_timer();

			if (FD_ISSET(m_listen_fd, &m_tmp_rset)) 
			{
				handle_accept();
				--ready;		
			}
            for (int i=0; ready > 0 && i< m_maxfd; i++)
            {
                TcpHandler* p_socket = fds[i];
                if(p_socket == NULL)
                {
                    continue;
                }                
                if(FD_ISSET(p_socket->GetFd(), &m_tmp_rset))
                {
                    --ready;
                    if (p_socket->handle_read() == -1)
                    {	
                        handle_close(p_socket);
                        continue;
                    }
                }
                if(FD_ISSET(p_socket->GetFd(), &m_tmp_wset))
                {
                    --ready;
                    if (p_socket->handle_output() == -1)
                    {
                        handle_close(p_socket);
                    }
                }
            }
		}
	}
#endif // WIN32
	return true;
}

bool TcpServer::_StartUpLin()
{
#ifndef WIN32
    int loop_times = 0;
    const int timer_check_point = 10; //�ﵽָ��ѭ����������ʼtimer���

	while(m_bRun) 
	{
		int res = epoll_wait(m_epoll_fd, m_epev_arr, EVENT_TOTAL_COUNT, 100);
		if ( res < 0) {
            //��ʱ�˳�������ᵼ�� CPU 100% 
			if (EINTR == errno)
				continue;
			log_debug("epoll_wait return false, errno = %d\n", errno);
			break;
		}
		else if (0 == res) { //timeout	
            loop_times = 0;
			run_timer();
		}
        else {
            //���һ����������ʼ��鶨ʱ���ĳ�ʱ�¼�
            if (++loop_times >= timer_check_point) {
                loop_times = 0;
                //������ʱ������
                run_timer();
            }
        }
		
		for(int i=0; i<res; i++)
		{
			if(m_epev_arr[i].data.fd == m_listen_fd)
			{
				handle_accept();
                continue;
			}
            int fd = (uint32_t)m_epev_arr[i].data.u64; /* mask out the lower 32 bits */
            uint32 index = (uint32_t)(m_epev_arr[i].data.u64 >> 32);
            TcpHandler* s = fds[fd];
            if( s == 0 || s->get_fd_index() != index )
            {                      
                continue;       // epoll returned invalid fd 
            }
            if( m_epev_arr[i].events & ( EPOLLHUP | EPOLLERR ))
            {    
                handle_close(s);
                continue;
            }
			else if(m_epev_arr[i].events & EPOLLIN )//��
			{				
                if( s->handle_read() == -1 )
				{
					handle_close(s);
					continue;
				}
                if( s->Writable() )
                    WantWrite(s);
			}
			else if( m_epev_arr[i].events & EPOLLOUT )//д
			{
				if( s->handle_output() == -1 )
				{
					handle_close(s);
					continue;
				}
                if( !s->Writable() )
                    WantRead(s);
			}
		}
	}
#endif 
	return true;
}

int TcpServer::handle_accept()
{
	SOCKET conn_fd;
    do 
    {
        if((conn_fd = socketops::myaccept(m_listen_fd)) == INVALID_SOCKET)
        {
            break;
        }
        socketops::set_socketbuf(conn_fd,16*1024);
        if(socketops::set_nonblock(conn_fd) < 0)
        {
            log_error("SetNonblock faild \n");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }
        if(socketops::set_keepalive(conn_fd) < 0)
        {
            log_error("set_keepalive faild \n");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }	
        
        TcpHandler* sh = AllocSocketHandler(conn_fd);
        if(sh == NULL)
        {
            log_error("sh is null \n");
            socketops::myclose(conn_fd);
            assert(false);
            continue;
        }
        AddSocket(sh);
        sh->handle_OnConnected();
    } while(conn_fd > 0);

	return 0;
}

void TcpServer::handle_close(TcpHandler* pHandler)
{
    assert(pHandler != NULL);
    pHandler->handle_close();

    RemoveSocket(pHandler);

    if(pHandler->GetNeedDel())
    {
        delete pHandler;
        pHandler = NULL;
    }
}

TcpHandler* TcpServer::AllocSocketHandler(SOCKET sock_fd)
{
	TcpHandler* sh = CreateHandler();
	if(sh != NULL)
	{
        sh->SetNeedDel(true);
		sh->SetFd(sock_fd);		
		sh->server(this);
	}
	return sh;
}
bool TcpServer::DisConnect(TcpHandler * pSocketHandler)
{
    log_debug("disconnect \n");
    handle_close(pSocketHandler);
 	return true;
}
bool TcpServer::reg(TcpHandler *pHandler)
{
    if(pHandler == NULL)
        return false;

    AddSocket(pHandler);

    pHandler->server(this);
    pHandler->handle_OnConnected();	

	return true;
}

void TcpServer::AddSocket(TcpHandler * s)
{
    m_count_fd++;
    m_fd_index++;

    s->set_fd_index(m_fd_index);

    //log_debug("Add one fd:%d ,Cur fds��%d \n",s->GetFd(),m_count_fd);

    assert( s->GetFd() < MAX_DESCRIPTORS );
    assert(fds[s->GetFd()] == 0);
    fds[s->GetFd()] = s;
    
#ifdef WIN32
    FD_SET(s->GetFd(), &m_rset);
    m_maxfd = (s->GetFd() > m_maxfd) ? s->GetFd() : m_maxfd;    
#else
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));

    /* store the generation counter in the upper 32 bits, the fd in the lower 32 bits */
    ev.data.u64 = (uint64_t)(uint32)(s->GetFd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLIN;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, s->GetFd(), &ev);
#endif
}

void TcpServer::RemoveSocket(TcpHandler * s)
{
    m_count_fd--;
    //log_debug("delete one fd: %d ,cur fds: %d \n",s->GetFd(),m_count_fd);

    assert(fds[s->GetFd()] == s);
    fds[s->GetFd()] = 0;

#ifdef WIN32
    FD_CLR(s->GetFd(), &m_rset);
    FD_CLR(s->GetFd(), &m_wset);
#else
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->GetFd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events =  EPOLLOUT | EPOLLIN;

    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, s->GetFd(), &ev);
#endif
    socketops::myclose(s->GetFd());
}

void TcpServer::WantWrite(TcpHandler * s)
{
#ifndef WIN32
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->GetFd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLOUT ;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, s->GetFd(), &ev);
#endif
}
void TcpServer::WantRead(TcpHandler * s)
{
#ifndef WIN32
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.u64 = (uint64_t)(uint32)(s->GetFd()) | ((uint64_t)(uint32)(s->get_fd_index()) << 32);

    ev.events = EPOLLIN ;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, s->GetFd(), &ev);
#endif
}
