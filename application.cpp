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

#include <exception>
#include <iostream>
#include <cxxtools/application.h>
#include <cxxtools/signal.h>
#include <cxxtools/timer.h>
#include <cxxtools/eventloop.h>
#include <cxxtools/net/tcpstream.h>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <cxxtools/posix/fork.h>

class MyApp : public cxxtools::Application
{
protected:
	cxxtools::Signal<> signal;
	cxxtools::Signal<int> signalNo;
	log_define("myApp.cl")
	int m_value;

public:
	MyApp(int argc, char** argv);
	bool method(int eventNo)
	{
		log_warn("warn message " << __LINE__ << " event number " << eventNo);
		return true;
	}
	char constMethod() const
	{
		log_warn("warn message " << __LINE__);
		return 'r';
	}

	static std::string staticMethod()
	{
		log_warn("warn message " << __LINE__);
		return "test";
	}
	void triggerEvents()
	{
		signal.send();
		signalNo.send(1234);
	}
};

MyApp::MyApp(int argc, char** argv) : cxxtools::Application(argc, argv) 
{
	cxxtools::Arg<std::string> ip(argc, argv, 'i');
	cxxtools::Arg<unsigned short> port(argc, argv, 'p', 1234);
	cxxtools::Arg<unsigned> bufsize(argc, argv, 'b', 8192);
	cxxtools::Arg<bool> listen(argc, argv, 'l');
	cxxtools::Arg<bool> read_reply(argc, argv, 'r');

	std::cout << "Application ctor" << std::endl;
	signal.connect( cxxtools::slot(*this, &MyApp::constMethod) );
	signal.connect( cxxtools::slot(&MyApp::staticMethod) );
	signalNo.connect( cxxtools::slot(*this, &MyApp::method) );
}

int main(int argc, char* argv[])
{
	try
	{
		log_init();
		cxxtools::posix::Fork fork;
		MyApp theApp(argc, argv);

		if (fork.parent()) {
			theApp.run();
			fork.wait();
		} else {
			::sleep(1);
			theApp.triggerEvents();
			// you could have timer events for socket events
			//cxxtools::EventLoop loop;
			//cxxtools::Timer intervalTimer(&loop);
			//intervalTimer.start(1000);
			//cxxtools::connect(intervalTimer.timeout, &MyApp::staticMethod);
			//loop.run();
		}
/*
		if (listen)
		{
			// I'm a server

			// listen to a port
			cxxtools::net::TcpServer server(ip.getValue(), port);

			// accept a connetion
			cxxtools::net::iostream worker(server, bufsize);

			// copy to stdout
			std::cout << worker.rdbuf();
		}
		else
		{
			// I'm a client

			// connect to server
			cxxtools::net::iostream peer(ip, port, bufsize);

			// copy stdin to server
			peer << std::cin.rdbuf() << std::flush;

			if (read_reply)
				// copy answer to stdout
				std::cout << peer.rdbuf() << std::flush;
		}
*/
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

