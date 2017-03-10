#ifdef USE_XML_RPC

#include "microscope.h"
#include "gci_utils.h"

#include <utility.h>

#include "XmlRpc.h"

#include <iostream>
#include <stdlib.h>


using namespace XmlRpc;

// The server
static XmlRpcServer s;

static int CVICALLBACK start_xml_rpc_server_thread(void *functionData)
{
	Microscope *ms = (Microscope *) functionData;
	int port = 63782;

	XmlRpc::setVerbosity(5);

	// Create the server socket on the specified port
	s.bindAndListen(port);

	// Enable introspection
	s.enableIntrospection(true);

	// Wait for requests indefinitely
	s.work(-1.0);

	return 0;
}

extern "C" {
void start_xml_rpc_server(Microscope *microscope)
{
	CmtScheduleThreadPoolFunction (gci_thread_pool(), start_xml_rpc_server_thread, microscope, NULL);
}
}

// No arguments, result is "Hello".
class MicroscopeName : public XmlRpcServerMethod
{
public:
  MicroscopeName(XmlRpcServer* s) : XmlRpcServerMethod("MicroscopeName", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
	Microscope *ms = microscope_get_microscope();
    result = UIMODULE_GET_NAME(ms);
  }

} microscopeName(&s);    // This constructor registers the method with the server


class MicroscopeUptime : public XmlRpcServerMethod
{
public:
  MicroscopeUptime(XmlRpcServer* s) : XmlRpcServerMethod("Uptime", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
	double uptime;
	char time[200] = "";
	Microscope *ms = microscope_get_microscope();

	microscope_get_uptime(ms, &uptime);
	seconds_to_friendly_time(uptime, time);
    result = time;
  }

} microscopeUptime(&s);    // This constructor registers the method with the server

#endif