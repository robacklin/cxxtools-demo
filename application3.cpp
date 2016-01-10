/*
 * Copyright (C) 2003 Tommi Maekitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cstdio>
#include <errno.h>
#include "afc.h"

class MyThread : public AfcThreadClass {
	AfcMQServer mqServer;
protected:
	void run() {
		AfcTrace("MyThread started");
		while(true) {
			char msg[2048];
			memset(msg, 0, 2048);
			AfcTrace("mqserver recv = " << mqServer.recv(msg, 2048));
			AfcTrace("MyThread working " << msg);
		}
	};
public:
	MyThread() : AfcThreadClass() {
		std::string mqName = "/mythread";
		AfcTrace("mqserver = " << mqServer.open(mqName));
	}
};

class MyApp : public AfcConsoleApplication
{
protected:
	AfcLogInit("MyApp");
	AfcSignal signal;
	AfcSignal signalNo;
	int m_value;
	AfcThread *p_afcThread;
	AfcMQClient mqClient;
	void init() {
		std::string mqName = "/mythread"; // create after mqServer
		AfcTrace("mqclient = " << mqClient.open(mqName));
	}
public:
	MyApp(int argc, char** argv);
	void onTimer() {
		char buffer[32] = {0};
		sprintf(buffer, "hello%d", m_value);
		if(++m_value < 3) {
			msgWrite(101, strlen(buffer), buffer);
			AfcWarn("timer - warn message " << buffer << " pipe sent");
		} else {
			msgWrite(0);
			AfcWarn("timer - warn message " << __LINE__);
		}
		AfcTrace("mqclient send = " << mqClient.send(buffer, strlen(buffer) ));
		return;
	}
	bool method(int eventNo) {
		AfcPipeMessage data = {0, 0, 0};
		char msg[1024];
		msgRead(data.id, data.size, msg, data.type); // blocking call
		if(data.id > 0) {
			AfcTrace("pipe message = " << msg);
		}
		AfcWarn("event - warn message " << __LINE__ << " event number = " << eventNo);
		return true;
	}
	char constMethod(int event) const {
		AfcWarn("event - warn message " << __LINE__ << " event=" << event);
		return 'r';
	}
	static std::string staticMethod(int event) {
		AfcWarn("event - warn message " << __LINE__);
		return "test";
	}
	void triggerEvents() {
		signal.send(0);
		signalNo.send(1234);
	}
};

MyApp::MyApp(int argc, char** argv) : AfcConsoleApplication(argc, argv) 
{
	AfcArgString ip(argc, argv, 'i');
	AfcArgInteger port(argc, argv, 'p', 1234);
	AfcArgBool lag(argc, argv, 'l');
	
	std::string str = p_conf->getValue("test", "value", "");
	AfcTrace(" here=" << str );

	m_value = 0;
	signal.connect( AfcSlot(*this, &MyApp::constMethod) );
	signal.connect( AfcSlot(&MyApp::staticMethod) );
	signalNo.connect( AfcSlot(*this, &MyApp::method) );

	AfcTimerSet(1000, 1, &MyApp::onTimer);
	if(lag) {
		AfcTimerSet(3500, 2, &MyApp::triggerEvents);
		p_afcThread = new MyThread();
		p_afcThread->create();
	}
	AfcTrace("Application ctor");

}

int main(int argc, char* argv[])
{
	try {
		AfcInit();
		MyApp theApp(argc, argv);
		return theApp.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

