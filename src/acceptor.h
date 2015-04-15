#ifndef ACCEPTOR_HEADER
#define ACCEPTOR_HEADER

#include <map>
#include <string>
#include <boost/noncopyable.hpp>

#include "connection.h"
#include "socketops.h"
#include "pack1.h"

class eventloop;

class acceptor : boost::noncopyable
{
public:
	acceptor(eventloop* loop, const netaddr& listenaddr, const std::string& name, bool reuseport);
	~acceptor();
	void listen();
	bool islistenning() const {return listenning_;}

private:
	std::string new_conn_name();
	void handle_accept();

	void handle_read(time_t receivetime);
	void handle_write();
	void handle_close();
	void handle_error();
	void handle_inpack1(inpack1* ipack);

private:
	eventloop* loop_;
	int acceptsockfd_;
	connection acceptconn_;
	bool listenning_;
	int idlefd_;

	std::string hostport_;
	std::string name_;
	int nextconnid_;
	std::map<std::string, connectionptr> conns_;
};

#endif

