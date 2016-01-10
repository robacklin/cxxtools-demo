/*
 * Copyright (C) 2009,2011 Tommi Maekitalo
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

/*

	Cxxtools implements a rpc framework, which can run with 4 protocols.

	Xmlrpc is a simple xml based standard protocol. The spec can be found at
	http://www.xmlrpc.org/.

	Json rpc is a json based standard protocol. The sepc can be found at
	http://json-rpc.org. Json is less verbose than xml and therefore jsonrpc is a
	little faster than xmlrpc.

	Json rpc comes in cxxtools in 2 flavours: raw and http.

	And last but not least cxxtools has a own non standard binary protocol. It is
	faster than xmlrpc or jsonrpc but works only with cxxtools.

	This demo program implements a server, which runs all 4 flavours in a single
	event loop.

 */

#include "afc.h"

double add(double a1, double a2, double a3)
{
	AfcTrace("sum: " << a1);
	return a1 + a2 + a3;
}

class api {
protected:
	std::string m_tmp;
public:
	std::string echo(std::string message, std::string blah)
	{
		AfcTrace("class echo: " << message);
		m_tmp += message + "-";
		return m_tmp;
	};
	double clear(double a1, double a2, double a3)
	{
		m_tmp = "";
		return 0;
	};
};

class server : public AfcRpcServer {
	std::map<std::string, uint> m_mapReferene;
public:
	server(AfcEventLoop &loop, AfcArgString &ip, AfcArgInteger &port) 
		: AfcRpcServer(loop, ip, port) {
		registerMethod("echo", m_api, &api::echo);
		registerMethod("clear", m_api, &api::clear);
		registerMethod("waitdelay", *this, &server::waitdelay);
		registerMethod("addReference", *this, &server::addReference);
		registerMethod("removeReferene", *this, &server::removeReferene);
		registerFunction("add", add);
	}
	uint waitdelay(uint a)
	{
		AfcTrace("hello RPC");
		AfcSleep(2000);
		AfcTrace("hello RPC");
	};
	uint addReference(std::string item, uint tmp) {
		m_mapReferene[item] = tmp;
	}
	uint removeReferene(std::string item) {
		m_mapReferene.erase(item); 
	}
protected:
	api m_api;
};

////////////////////////////////////////////////////////////////////////
// main
//
int main(int argc, char* argv[])
{
	try {
		AfcInit();
		AfcArgString ip(argc, argv, 'i');
		AfcArgInteger port(argc, argv, 'p', 7002);
		AfcArgBool daemonize(argc, argv, 'd');
		AfcArgString pidfile(argc, argv, "--pidfile");

		AfcTrace("run rpcecho server \n"
			<< "json protocol on port " << port.getValue());

		AfcEventLoop loop;
		server o(loop, ip, port);

		// go to the background if requested
		AfcDaemonize(daemonize, pidfile);

		// now start the servers by running the event loop
		o.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

