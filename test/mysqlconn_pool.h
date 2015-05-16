#ifndef MYSQLCONN_POOL_HEADER
#define MYSQLCONN_POOL_HEADER

#include <deque>
#include <sstream>
#include <string>
#include <mysql++/mysql++.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>

class mysqlconn_pool : boost::noncopyable
{
public:
  mysqlconn_pool(int cnt);
  ~mysqlconn_pool();
  unsigned size();
  bool execute(const std::string& sql);
  bool getuuid(std::string* id);
  bool pingdb();
  bool initconn(boost::shared_ptr<mysqlpp::Connection> conn);
  bool select_example();

private:
  friend class mysqlconn_helper;
  void addnewconn(boost::shared_ptr<mysqlpp::Connection> conn);
  bool popnewconn(boost::shared_ptr<mysqlpp::Connection>* conn);
  void newconn_threadfunc();
  bool get(boost::shared_ptr<mysqlpp::Connection>* conn);
  void back(boost::shared_ptr<mysqlpp::Connection> conn, bool broken = false);

private:
  unsigned conncount_;

  std::deque< boost::shared_ptr<mysqlpp::Connection> > conns_;
  boost::shared_mutex connslock_;

  std::deque< boost::shared_ptr<mysqlpp::Connection> > newconns_;
  boost::shared_mutex newconnslock_;

  boost::shared_ptr<boost::thread> newconnsthread_;
};

#endif
