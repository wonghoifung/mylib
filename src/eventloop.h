#ifndef EVENTLOOP_HEADER
#define EVENTLOOP_HEADER

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "connection.h"
#include "myepoll.h"

class eventloop : boost::noncopyable
{
public:
	eventloop();
	~eventloop();
	void loop();
	void updateconn(connection* conn);
	void removeconn(connection* conn);

private:
	bool looping_; 
	bool quit_;
	bool eventhandling_;
	time_t pollreturntime_;
	boost::scoped_ptr<myepoll> poller_;
	std::vector<connection*> activeconns_;
	connection* currentactiveconn_;
};

#endif

