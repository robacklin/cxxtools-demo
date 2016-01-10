#include "afc.h"

typedef struct item_t {
	uint port;
} item;

typedef std::string String;
typedef std::map<String, item> mapItem;

class server : public AfcRpcServer {
	std::map<String, mapItem> m_mapReferene;
	//mapItem& m_mapItem;
public:
	server(AfcEventLoop &loop, AfcArgString &ip, AfcArgInteger &port) 
		: AfcRpcServer(loop, ip, port) {
		registerMethod("createref", *this, &server::createref);
		registerMethod("readref", *this, &server::readref);
		registerMethod("updateref", *this, &server::updateref);
		registerMethod("deleteref", *this, &server::deleteref);
	}
	uint createref(uint id, String title, String address, uint port, uint expire) {
		AfcTrace("id = " << id << " title = " << title);
		//m_mapReferene[item] = tmp;
	}
	uint readref(uint id, String title, String &address) {
		AfcTrace("id = " << id);
		address = "yellow";
		//m_mapReferene.erase(item); 
	}
	uint updateref(uint id, String title) {
		AfcTrace("id = " << id);
	}
	uint deleteref(uint id, String title) {
		AfcTrace("id = " << id);
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
		AfcArgBool daemonize(argc, argv, 'd');
		AfcArgString pidfile(argc, argv, "--pidfile");

		AfcTrace("run ds server \n"
			<< "json protocol on port " << port.getValue());

		AfcEventLoop loop;
		server o(loop, ip, port);

		AfcDaemonize(daemonize, pidfile);

		o.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

