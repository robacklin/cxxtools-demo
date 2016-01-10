/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#include <iostream>
#include "afc.h"

class Remote : public IAfcRemote {
public:
	Remote(AfcJsonClient &c) : IAfcRemote(c) {
	}
	std::string echo(std::string message, std::string blah) {
		//AfcRpc<std::string, std::string, std::string> rpc(m_conn, "echo");
		_RPC(AfcRpcString, echo, message, blah);
	}
	double clear(double a1, double a2, double a3) {
		_RPC(AfcRpcInteger, clear, a1, a2, a3);
	}
	uint waitdelay(uint a) {
		_RPC(AfcRpcProcedure, waitdelay, a);
	}
	double add(double a1, double a2, double a3) {
		AfcRpc<double, double, double, double> rpc(m_conn, "add");
		return rpc(a1, a2, a3);
	}
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
	 AfcArgBool clear(argc, argv, 'c');

    AfcJsonClient conn(ip, port);
	 Remote r(conn);

	 AfcTrace("wait for rpc call to complete");
	 r.waitdelay(0);
	 AfcTrace("rpc call done");

	 if(clear)
		 r.clear(0, 0, 0);

    double sum = 0;
    for (int a = 1; a < argc; ++a) {
      sum = r.add(sum, atof(argv[a]), 0);
    	AfcTrace("sum=" << sum << " echo=" << 
				r.echo("Hello World", ""));
	 }

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

