/*  _    _____ ____ 
   / \  |  ___/ ___|
  / _ \ | |_ | |    
 / ___ \|  _|| |___ 
/_/   \_\_|   \____|

*/
#ifndef __AFC__
#define __AFC__

#include <exception>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cxxtools/application.h>
#include <cxxtools/signal.h>
#include <cxxtools/timer.h>
#include <cxxtools/eventloop.h>
#include <cxxtools/net/tcpstream.h>
#include <cxxtools/net/udp.h>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <cxxtools/posix/fork.h>
#include <cxxtools/inifile.h>
#include <cxxtools/iniparser.h>
#include <cxxtools/posix/pipestream.h>
#include <cxxtools/pipe.h>
#include <cxxtools/condition.h>
#include <cxxtools/posix/commandoutput.h>
#include <cxxtools/posix/daemonize.h>
#include <cxxtools/split.h>
#include <cxxtools/http/client.h>
#include <cxxtools/thread.h>
#include <cxxtools/threadpool.h>
#include <cxxtools/remoteprocedure.h>
#include <cxxtools/json/rpcclient.h>
#include <cxxtools/json/rpcserver.h>
#include <mqueue.h>
#include <time.h>

typedef cxxtools::Application AfcApplication;
typedef cxxtools::Arg<std::string> AfcArgString;
typedef cxxtools::Arg<bool> AfcArgBool;
typedef cxxtools::Arg<unsigned int> AfcArgInteger;
typedef cxxtools::EventLoop AfcEventLoop;
typedef cxxtools::Timer AfcTimer;
typedef cxxtools::Signal<int> AfcSignal;
typedef cxxtools::posix::Fork AfcFork;
typedef cxxtools::net::TcpServer AfcServer;
typedef cxxtools::net::iostream AfcClient;
typedef cxxtools::net::UdpReceiver AfcReceiver;
typedef cxxtools::net::UdpSender AfcSender;
typedef cxxtools::IniFile AfcConfig;
typedef cxxtools::MutexLock AfcLock;
typedef cxxtools::Mutex AfcMutex;
typedef cxxtools::Condition AfcCondition;
typedef cxxtools::posix::Pipestream AfcPipeStream;
typedef cxxtools::posix::Pipe AfcPipe;
typedef cxxtools::posix::CommandOutput AfcCmd;
typedef cxxtools::Timespan AfcTimeSpan;
typedef cxxtools::http::Client AfcHTTPClient;
typedef cxxtools::Connectable AfcObject;
typedef cxxtools::Thread AfcThread;
typedef cxxtools::json::RpcClient AfcJsonClient;
typedef cxxtools::json::RpcServer AfcJsonServer;

#define AfcRpc cxxtools::RemoteProcedure
typedef AfcRpc<std::string, std::string, std::string> AfcRpcString;
typedef AfcRpc<double, double, double, double> AfcRpcInteger;
typedef AfcRpc<uint, uint> AfcRpcProcedure;

//typedef cxxtools::split AfcSplit;

#define _RPC(XTYPE, NAME, _VARA, ...) \
{ XTYPE rpc(m_conn, ""#NAME""); \
return rpc( _VARA, ##__VA_ARGS__ ); }

#define AFCEXPORT(_VARA) virtual _VARA = 0
#define AFCIMPORT(_VARA, _VARB) _VARA *_VARB
#define AfcInit(_VARA) log_init(_VARA)
#define AfcInfo(_VARA) log_info("(" << __FUNCTION__ << ":" << __LINE__ << ") " << _VARA)
#define AfcWarn(_VARA) log_warn("(" << __FUNCTION__ << ":" << __LINE__ << ") " << _VARA)
#define AfcError(_VARA) log_error("(" << __FUNCTION__ << ":" << __LINE__ << ") " << _VARA)
#define AfcFatal(_VARA) log_fatal("(" << __FUNCTION__ << ":" << __LINE__ << ") " << _VARA)
#define AfcDebug(_VARA) log_debug("(" << __FUNCTION__ << ":" << __LINE__ << ") " << _VARA)
#define AfcTrace(_VARA) std::cout << __FILE__ << ":" << __LINE__ << " -- " << _VARA  << std::endl << std::flush
#define AfcLogInit(_VARA) log_define(_VARA)

#define AfcSleep(_VARA) cxxtools::Thread::sleep(_VARA)
#define AfcSlot cxxtools::slot
#define AfcTimerSet(_VARA, _VARB, ...) \
	addTimer(_VARA, _VARB); \
	cxxtools::connect(m_afcTimer[_VARB].timeout, *this, ##__VA_ARGS__); \
	addEventTimer(m_afcTimer[_VARB])
#define AfcDaemonize(_VARA, _VARB) if(_VARA) cxxtools::posix::daemonize(_VARB) 

typedef struct tagAfcPipeMessage
{
	uint magic;
	uint id;
	uint size;
	uint type;
} AfcPipeMessage;

class IAfcPiper : protected AfcPipe {
	uint m_magicNum;
protected:
	void payloadRead(char *msg, uint size) {
		if(size <= 0)
			return;
		read(msg, size);
	};
	void payloadWrite(char *msg, uint size) {
		if(size <= 0)
			return;
		write(msg, size);
	};
public:
	IAfcPiper() : AfcPipe(), m_magicNum(0xdead) {
	};
	void msgWrite(uint id, uint size = 0, char *msg = 0, uint type = 0) {
		AfcPipeMessage data;
		data.magic = m_magicNum;
		data.id = id;
		data.size = size;
		data.type = type;
		write((char *)&data, sizeof(data));
		payloadWrite(msg, size);
	};
	void msgRead(uint &id, uint &size, char *msg, uint &type) {
		if(msg == 0 || out().eof())
			return;
		id = 0;
		AfcPipeMessage data;
		read((char *)&data, sizeof(data));
		if(data.magic != m_magicNum)
			return;
		id = data.id;
		size = data.size;
		type = data.type;
		payloadRead(msg, size);
	};
};

class IAfcTimer {
protected:
	std::map<uint, AfcTimer> m_afcTimer;
public:
	void addTimer(uint delay, uint id) {
		m_afcTimer.insert(std::map<uint, AfcTimer>::value_type(id, AfcTimer()));
		m_afcTimer[id].start(delay);
	};
	void getTimer(uint id) {};
	void clearTimer(uint id) {};
};

class IAfcEventApplication : public AfcApplication {
protected:
	AfcEventLoop m_afcLoop;
	virtual void init() {
	};
	virtual void deinit() {
	};
public:
	virtual int run() {
		init();
		m_afcLoop.run();
		deinit();
		return 0;
	};
	void addEventTimer(AfcTimer& t) {
		m_afcLoop.add(t);
	};
};

class AfcThreadClass 
	: public cxxtools::DetachedThread, public IAfcPiper, public IAfcTimer
{
public:
	~AfcThreadClass() {
	};
protected:
	void run() {
		AfcTrace("AfcThread started");
	};
};

class AfcConsoleApplication
	: public IAfcPiper, public IAfcTimer, public IAfcEventApplication
{
private:
protected:
	log_define("AfcConsoleApplication");
	AfcConfig *p_conf;
public:
	AfcConsoleApplication(int &argc, char** argv) 
		: IAfcEventApplication() { //TODO: AfcApplication(int &argc, char** argv)
		AfcArgString conf(argc, argv, 'f', "/etc/test.conf");
		p_conf = new AfcConfig(conf);
	};
	~AfcConsoleApplication() {
		delete p_conf;
	};
};

class IAfcMQueue {
protected:
	mqd_t m_hFile;
	struct mq_attr mqAttr;
	std::string m_mqName;
	uint m_msgPrio;
	//ref: http://man7.org/linux/man-pages/man3/mq_notify.3.html
public:
	IAfcMQueue() {
		m_msgPrio = 1;
		m_hFile = 0;
	};
	~IAfcMQueue() {
		close();
	}
	void close() {
		if(m_hFile != 0)
			mq_close(m_hFile);
		m_hFile = 0;
	}
	bool send(const char *msg, uint len, const struct timespec *abs_timeout = NULL) {
		if(m_hFile == 0) // will block if mq full else set O_NONBLOCK
			return false;
		if(abs_timeout != NULL)
			return mq_timedsend(m_hFile, msg, len, m_msgPrio, abs_timeout) < 0 ? false : true;
		return mq_send(m_hFile, msg, len, m_msgPrio) < 0 ? false : true;
	}
	bool recv(char *msg, uint len, const struct timespec *abs_timeout = NULL) { // blocking call
		uint msgPrio;
		if(m_hFile == 0)
			return false;
		if(abs_timeout != NULL)
			return mq_timedreceive(m_hFile, msg, len, &msgPrio, abs_timeout) < 0 ? false : true;
		return mq_receive(m_hFile, msg, len, &msgPrio) < 0 ? false : true;
	}
	bool isopen() {
		return (m_hFile == 0) ? false : true;
	}
};

class AfcMQServer : public IAfcMQueue {
public: // create before client
	bool open(std::string &mqName) {
		m_mqName = mqName;
		mq_unlink(m_mqName.c_str());
		mqAttr.mq_flags = 0; // or O_NONBLOCK
		mqAttr.mq_maxmsg = 10;
		mqAttr.mq_msgsize = 1024;
		m_hFile = mq_open(m_mqName.c_str(), O_RDWR|O_CREAT, S_IWUSR|S_IRUSR, &mqAttr);
		return (m_hFile < 0) ? false : true;
	}
};

class AfcMQClient : public IAfcMQueue {
public:
	bool open(std::string &mqName) {
		m_mqName = mqName;
		m_hFile = mq_open(m_mqName.c_str(), O_RDWR);
		return (m_hFile < 0) ? false : true;
	}
};

class AfcStateMachine;
class AfcState {
public:
	virtual void enterState() {
	};
	virtual void exitState() {
	};
	virtual void eventState(void *e, AfcStateMachine *m) {
		//if(e->idle()) m->set(m->states[IDLE])
	};
};

class AfcStateMachine {
protected:
	AfcState *m_pCurrent;
public:
	AfcStateMachine(void *e /*initial state*/) {
	}
	~AfcStateMachine() {
	}
	void set(AfcState *p) {
		m_pCurrent->exitState();
		m_pCurrent = p;
		m_pCurrent->enterState();
	}
	void process(void *e) {
		m_pCurrent->eventState(e, this);
	};
};

class AfcRpcServer : public AfcJsonServer {
public:
	AfcEventLoop& m_loop;
public:
	AfcRpcServer(AfcEventLoop &loop, AfcArgString &ip, AfcArgInteger &port) 
		: m_loop(loop) 
		, AfcJsonServer(loop, ip, port) {
	};
	~AfcRpcServer() {
	};
	virtual int run() {
		m_loop.run();
		return 0;
	};
};

class IAfcRemote {
protected:
	AfcJsonClient m_conn;
public:
	IAfcRemote(AfcJsonClient &c) {
		m_conn = c;
	}
};


#endif //__AFC__
