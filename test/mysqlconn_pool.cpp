#include "mysqlconn_pool.h"
#include <boost/make_shared.hpp>

class mysqlconn_helper : boost::noncopyable
{
public:
  mysqlconn_helper(boost::shared_ptr<mysqlpp::Connection>* conn, mysqlconn_pool* holder)
    :conn_(conn), holder_(holder), gotconn_(true)
  {
    if (!holder_->get(conn))
    {
      gotconn_ = false;
    }
  }

  ~mysqlconn_helper()
  {
    if (!connstate())
    {
      return;
    }
    if ((*conn_)->ping())
    {
      holder_->back(*conn_, false);
    }
    else
    {
      holder_->back(*conn_, true);
    }
  }

  bool connstate()
  {
    return gotconn_;
  }


private:
  boost::shared_ptr<mysqlpp::Connection>* conn_;
  mysqlconn_pool* holder_;
  bool gotconn_;
};

mysqlconn_pool::mysqlconn_pool(int cnt) : conncount_(cnt)
{
  for (size_t i = 0; i < conncount_; ++i)
  {
    conns_.push_back(boost::make_shared<mysqlpp::Connection>());
    if (!initconn(conns_[i]))
    {
      abort();
    }
  }
  newconnsthread_ = boost::make_shared<boost::thread>(boost::bind(&mysqlconn_pool::newconn_threadfunc, this));
  if (!newconnsthread_)
  {
    abort();
  }
}

mysqlconn_pool::~mysqlconn_pool()
{

}

unsigned mysqlconn_pool::size()
{
  boost::shared_lock<boost::shared_mutex> lock(connslock_);
  return conns_.size();
}

bool mysqlconn_pool::execute(const std::string& sql)
{
  boost::shared_ptr<mysqlpp::Connection> conn;
  int retrycnt = 5;
  while (retrycnt--)
  {
    try
    {
      mysqlconn_helper connhelper(&conn, this);
      if (!connhelper.connstate())
      {
        return false;
      }
      mysqlpp::Query query = conn->query();
      if (!query.exec(sql))
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    catch (const std::exception& e)
    {
      // TODO log
    }
  }
  return false;
}

bool mysqlconn_pool::getuuid(std::string* id)
{
  boost::shared_ptr<mysqlpp::Connection> conn;
  std::stringstream ssql;
  ssql << "select UUID() as id";
  int retrycnt = 5;
  while (retrycnt--)
  {
    try
    {
      mysqlconn_helper connhelper(&conn, this);
      if (!connhelper.connstate())
      {
        return false;
      }
      mysqlpp::Query query = conn->query(ssql.str());
      mysqlpp::StoreQueryResult res = query.store();
      assert(res);
      mysqlpp::StoreQueryResult::const_iterator it;
      for (it = res.begin(); it != res.end(); ++it)
      {
        const mysqlpp::Row& row = *it;
        *id = row["id"].c_str();
      }
      return true;
    }
    catch (const mysqlpp::Exception& e)
    {
      // TODO
    }
  }
  return false;
}

bool mysqlconn_pool::pingdb()
{
  boost::shared_ptr<mysqlpp::Connection> conn;
  mysqlconn_helper connhelper(&conn, this);
  if (!connhelper.connstate())
  {
    return false;
  }
  return true;
}

bool mysqlconn_pool::initconn(boost::shared_ptr<mysqlpp::Connection> conn)
{
  int retrycnt = 5;
  try
  {
    while (retrycnt--)
    {
      conn->set_option(new mysqlpp::ReconnectOption(true));
      if (conn->connect("db","ip","user","pass")) // TODO
      {
        std::stringstream ssql;
        ssql << "set names utf8";
        if (!conn->query(ssql.str()).execute())
        {
          return false;
        }
        return true;
      }
    }
  }
  catch (const mysqlpp::Exception& e)
  {
    // TODO
  }
  return false;
}

bool mysqlconn_pool::select_example()
{
  std::stringstream ssql;
  ssql << "select * from testdb";
  boost::shared_ptr<mysqlpp::Connection> conn;
  int retrycnt = 5;
  while (retrycnt--)
  {
    try
    {
      mysqlconn_helper connhelper(&conn, this);
      if (!connhelper.connstate())
      {
        return false;
      }
      mysqlpp::Query query = conn->query(ssql.str());
      mysqlpp::StoreQueryResult res = query.store();
      assert(res);
      mysqlpp::StoreQueryResult::const_iterator it;
      for (it = res.begin(); it != res.end(); ++it)
      {
        const mysqlpp::Row& row = *it;
        // TODO
      }
      return true;
    }
    catch (const mysqlpp::Exception& e)
    {
      // TODO
    }
  }
  return false;
}

void mysqlconn_pool::addnewconn(boost::shared_ptr<mysqlpp::Connection> conn)
{
  boost::unique_lock<boost::shared_mutex> lock(newconnslock_);
  newconns_.push_back(conn);
}

bool mysqlconn_pool::popnewconn(boost::shared_ptr<mysqlpp::Connection>* conn)
{
  boost::unique_lock<boost::shared_mutex> lock(connslock_);
  if (newconns_.empty())
  {
    return false;
  }
  else
  {
    *conn = newconns_.front();
    newconns_.pop_front();
    return true;
  }
}

void mysqlconn_pool::newconn_threadfunc()
{
  while (true)
  {
    boost::shared_ptr<mysqlpp::Connection> conn;
    if (!popnewconn(&conn))
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
      // TODO condition variable
    }
    else
    {
      if (!initconn(conn))
      {
        // TODO if u dont deal with this situation, connections will be getting fewer
      }
      else
      {
        boost::unique_lock<boost::shared_mutex> lock(connslock_);
        conns_.push_back(conn);
      }
    }
  }
}

bool mysqlconn_pool::get(boost::shared_ptr<mysqlpp::Connection>* conn)
{
  boost::unique_lock<boost::shared_mutex> lock(connslock_);
  if (conns_.empty())
  {
    return false;
  }
  *conn = conns_.front();
  conns_.pop_front();
  return true;
}

void mysqlconn_pool::back(boost::shared_ptr<mysqlpp::Connection> conn, bool broken)
{
  if (broken)
  {
    boost::shared_ptr<mysqlpp::Connection> newconn = boost::make_shared<mysqlpp::Connection>();
    addnewconn(newconn);
    return;
  }
  boost::unique_lock<boost::shared_mutex> lock(connslock_);
  conns_.push_back(conn);
}
