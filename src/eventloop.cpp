#include "eventloop.h"

eventloop::eventloop()
:looping_(false),
 quit_(false),
 eventhandling_(false),
 poller_(new myepoll()),
 currentactiveconn_(NULL)
{

}

eventloop::~eventloop()
{

}

void eventloop::loop()
{
	assert(!looping_);
	looping_ = true;
	quit_ = false;
	while (!quit_)
	{
		activeconns_.clear();
		pollreturntime_ = poller_->poll(10000, &activeconns_);
		eventhandling_ = true;
		std::vector<connection*>::iterator it(activeconns_.begin());
		for (; it != activeconns_.end(); ++it)
		{
			currentactiveconn_ = *it;
			currentactiveconn_->handle_event(pollreturntime_);
		}
		currentactiveconn_ = NULL;
		eventhandling_ = false;
	}
	looping_ = false;
}

