#include <iostream>
#include "afc.h"

typedef std::string String;

class Remote : public IAfcRemote {
public:
	Remote(AfcJsonClient &c) : IAfcRemote(c) {
	}
	uint createref(uint id, String title, String address, 
			uint port, uint expire) {
		AfcRpc<uint, uint, String, String, uint, uint> rpc(m_conn, "createref");
		return rpc(id, title, address, port, expire);
	}
	uint readref(uint id, String title, String address) {
		AfcTrace("id = " << id);
		AfcRpc<uint, uint, String, String> rpc(m_conn, "readref");
		return rpc(id, title, address);
	}
	uint updateref(uint id, String title) {
		AfcTrace("id = " << id);
		AfcRpc<uint, uint, String> rpc(m_conn, "updateref");
		return rpc(id, title);
	}
	uint deleteref(uint id, String title) {
		AfcTrace("id = " << id);
		AfcRpc<uint, uint, String> rpc(m_conn, "deleteref");
		return rpc(id, title);
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
	
    AfcArgInteger RefId(argc, argv, 'I', 101);
    AfcArgString RefStr(argc, argv, 'R', "hello");
    AfcArgString Address(argc, argv, 'A', "127.0.0.1");
    AfcArgInteger Port(argc, argv, 'P', 5000);
	 
	 AfcArgBool createref(argc, argv, 'c');
	 AfcArgBool readref(argc, argv, 'r');
	 AfcArgBool updateref(argc, argv, 'u');
	 AfcArgBool deleteref(argc, argv, 'd');

    AfcJsonClient conn(ip, port);
	 Remote r(conn);

	 if(createref)
	 	r.createref(RefId, RefStr, Address, Port, 3600);
	 String address;
	if(readref) {
		r.readref(RefId, RefStr, address);
		AfcTrace("ip = " << address);
	}
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

