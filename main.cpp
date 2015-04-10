#include <unistd.h>
#include <string>
#include <boost/noncopyable.hpp>

#include "timer.h"
#include "PacketBase.h"
#include "TcpServer.h"
#include "SocketServer.h"
#include "SocketHandler.h"

/* client send first */
enum {
	cmd_login = 0,
	cmd_sysbroadcast = 1,
	cmd_updatecfg = 2,

	cmd_heartbeat = 3,
	cmd_sayhello = 4,
};

/* server send first */
enum {
	cmd_forced_logout = 1000,
};

/* timer id */
enum {
	timer_heartbeat = 1,
	timer_update = 2,
};

/* timer interval */
enum {
	interval_heartbeat = 20,
	interval_update = 6,
};

static const std::string ANONYMOUS_KEY = "{0A2CF896-AD56-4574-8419-B318A1ED7803}";  

class user : boost::noncopyable
{
public:
	user(int uid, SocketHandler* ph):id_(uid),sockethandler_(ph) {}
	bool init() { timetrack_=time(NULL); return true; }
	int update_time_track() { timetrack_=time(NULL); return 0; }
	time_t get_time_track() { return timetrack_; }
	void set_id(int i) { id_ = i; }
	int get_id() { return id_; }
	void set_sockethandler(SocketHandler* pHandler) { sockethandler_ = pHandler; }
	SocketHandler* get_sockethandler() { return sockethandler_; }
	void sendmsg(NETOutputPacket* outp) {
		if (sockethandler_) {
			sockethandler_->Send(outp);
		}
	}
private:
	int id_;
	SocketHandler* sockethandler_;
	time_t timetrack_;
};

class user_manager : boost::noncopyable
{
private:
	user_manager() {}
	~user_manager() {}
public:
	static user_manager& ref() {
		static user_manager um;
		return um;
	}
	bool is_exist(int uid) {
		std::map<int,user*>::iterator it = users_.find(uid);
		if (it!=users_.end()) { return true; }
		return false;
	}
	user* get_user(int uid) {
		std::map<int,user*>::iterator it = users_.find(uid);
		if (it!=users_.end()) { return it->second; }
		return NULL;
	}
	bool add_user(user* u) {
		if (is_exist(u->get_id())) { return false; }
		return users_.insert(std::make_pair(u->get_id(),u)).second;
	}
	bool del_user(int uid) {
		std::map<int,user*>::iterator it = users_.find(uid);
		if (it!=users_.end()) {
			users_.erase(it);
			return true;
		}
		return false;
	}
	void sendtoall(NETOutputPacket* outp) {
		std::map<int,user*>::iterator beg(users_.begin()), end(users_.end());
		for (; beg!=end; ++beg) {
			beg->second->sendmsg(outp);
		}
	}
	bool sendto(int uid, NETOutputPacket* outp) {
		user* u = get_user(uid);
		if (u) {
			u->sendmsg(outp);
			return true;
		}
		return false;
	}
	std::map<int,user*>& get_users() { return users_; }
private:
	std::map<int,user*> users_;
};

class myserver : public SocketServer, public TimerOutEvent, boost::noncopyable
{
public:
	myserver() {
		timer_heartbeat_.SetTimeEventObj(this,timer_heartbeat);
		timer_update_.SetTimeEventObj(this,timer_update);
	}

	virtual ~myserver() {

	}

	bool init() {
		timer_heartbeat_.StartTimer(interval_heartbeat);
		timer_update_.StartTimer(interval_update);
		return true;
	}

	virtual int ProcessPacket(NETInputPacket* pPacket, SocketHandler* pHandler, DWORD dwSessionID) {
		const uint16 cmd = pPacket->GetCmdType();
		switch (cmd)
		{
		case cmd_login:
			return handle_login(pPacket, pHandler);
		case cmd_sysbroadcast:
			return handle_sysbroadcast(pPacket, pHandler);
		case cmd_updatecfg:
			return handle_updatecfg(pPacket, pHandler);
		default:
			break;
		}

		user* u = get_user(pHandler);
		if (u==NULL) { return -1; }

		switch (cmd)
		{
		case cmd_heartbeat:
			return u->update_time_track();
		case cmd_sayhello:
			return handle_sayhello(u,pPacket);
		default:
			break;
		}
		return 0;
	}

	virtual void OnConnect(SocketHandler* pHandler) {
		log_debug("%s connected", pHandler->GetAddr().c_str());
	}

	virtual void OnDisconnect(SocketHandler* pHandler) {
		log_debug("%s disconnected", pHandler->GetAddr().c_str());
		user* u = get_user(pHandler);
		if (u!=NULL) {
			clear_user(u);
		}
	}

	virtual int ProcessOnTimerOut(int Timerid) {
		switch (Timerid)
		{
		case timer_heartbeat:
			handle_timer_heartbeat();
			timer_heartbeat_.StartTimer(interval_heartbeat);
			break;
		case timer_update:
			handle_timer_update();
			timer_update_.StartTimer(interval_update);
			break;
		default:
			break;
		}
		return 0;
	}

	user* get_user(SocketHandler* pHandler) {
		if (pHandler) {
			void* ptr = pHandler->GetUserData();
			if (ptr) {
				return reinterpret_cast<user*>(ptr);
			}
		}
		return NULL;
	}

	int clear_user(user* u) {
		// other cleanup
		user_manager::ref().del_user(u->get_id());
		u->get_sockethandler()->SetUserData(NULL);
		delete u;
		return 0;
	}

	user* check_relogin(const int uid, SocketHandler* pHandler) {
		user* u = user_manager::ref().get_user(uid);
		if (u) {
			// send here?
			NETOutputPacket outpacket;
			outpacket.Begin(cmd_forced_logout);
			outpacket.WriteInt(uid);
			outpacket.WriteInt(1); // re login error code
			outpacket.End();
			u->sendmsg(&outpacket);

			SocketHandler* pHandler_old = u->get_sockethandler();
			if (pHandler == pHandler_old) {
				log_error("repeat login msg, uid:%d",u->get_id());
				return u;
			}

			log_error("login from another socket, uid:%d",u->get_id());
			clear_user(u);

			if (pHandler_old != NULL) {
				// parent method
				DisConnect(pHandler_old);
			}

			return NULL;
		}
		return u;
	}

	user* login(int uid, SocketHandler* ph) {
		user* u = new user(uid,ph);
		if (u) {
			if (!u->init()) {
				delete u;
				u = NULL;
				return NULL;
			}
			user_manager::ref().add_user(u);
			ph->SetUserData(u);
		}
		return u;
	}

	int handle_login(NETInputPacket* packet, SocketHandler* pHandler) {
		log_debug("%s login", pHandler->GetAddr().c_str());
		if (pHandler->GetUserData()) { return -1; }
		const int uid = packet->ReadInt();
		// any other fields?
		user* u = check_relogin(uid, pHandler);
		if (u == NULL) {
			if ((u = login(uid, pHandler)) == NULL) {
				NETOutputPacket outp;
				outp.Begin(cmd_login);
				outp.WriteInt(-1); // init failure
				outp.End();
				pHandler->Send(&outp);
				return -1;
			}
		} else {
			return -1;
		}
		NETOutputPacket outp;
		outp.Begin(cmd_login);
		outp.WriteInt(0); // success
		// any other fields?
		outp.End();
		pHandler->Send(&outp);
		return 0;
	}

	int handle_sysbroadcast(NETInputPacket* packet, SocketHandler* pHandler) {
		log_debug("%s sysboradcast", pHandler->GetAddr().c_str());
		if (packet->ReadString() == ANONYMOUS_KEY) {
			const int btype = packet->ReadInt();
			const std::string bcontent = packet->ReadString();
			NETOutputPacket outp;
			outp.Begin(cmd_sysbroadcast);
			outp.WriteInt(btype);
			outp.WriteString(bcontent);
			outp.End();
			user_manager::ref().sendtoall(&outp);
			return 0;
		}
		return -1;
	}

	int handle_updatecfg(NETInputPacket* packet, SocketHandler* pHandler) {
		log_debug("%s updatecfg", pHandler->GetAddr().c_str());
		if (packet->ReadString() == ANONYMOUS_KEY) {
			// update...
			return 0;
		}
		return -1;
	}

	int handle_sayhello(user* u, NETInputPacket* packet) {
		int to = packet->ReadInt();
		std::string content = packet->ReadString();
		log_debug("user %d say %s to %d", u->get_id(), content.c_str(), to);
		NETOutputPacket outp;
		outp.Begin(cmd_sayhello);
		outp.WriteInt(u->get_id());
		outp.WriteString(content);
		outp.End();
		u->sendmsg(&outp);
		return 0;
	}

	int handle_timer_heartbeat() {
		time_t now(time(NULL));
		std::vector<user*> noresponse_users;
		std::map<int,user*>& users = user_manager::ref().get_users();
		std::map<int,user*>::iterator beg(users.begin()),end(users.end());
		for (; beg!=end; ++beg) {
			if ( ( now - beg->second->get_time_track() ) > 30 ) {
				noresponse_users.push_back(beg->second);
			}
		}
		for (unsigned i=0; i<noresponse_users.size(); ++i) {
			user* u = noresponse_users[i];
			if (u) {
				SocketHandler* sh = u->get_sockethandler();
				clear_user(u);
				if (sh) {
					// make sure it is safe later
					DisConnect(sh);
				}
			}
		}
		return 0;
	}

	int handle_timer_update() {
		// update...
		return 0;
	}

private:
	TimerEvent timer_heartbeat_;
	TimerEvent timer_update_;
};

myserver* global_myserver() {
	static myserver* s = NULL;
	if (s == NULL) {
		s = new myserver();
	}
	return s;
}

int main(int argc, char** argv) {
	
	init_timer();

	init_log("Log", "./");
	set_log_level(7);

	if (global_myserver()->InitSocket(9999) == false)
	{
		log_error("cannot init socket 99999");
		return -1;
	}

	if (!global_myserver()->init())
	{
		log_error("cannot init server");
		return -2;
	}

	global_myserver()->Run();

	//global_myserver()->ShutDown();

	delete global_myserver();
	
	return 0;

}

