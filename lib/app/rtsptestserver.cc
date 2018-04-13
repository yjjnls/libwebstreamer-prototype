#include "webstreamer.h"
#include "rtsptestserver.h"
#include <endpoint/rtspservice.h>
using json = nlohmann::json;




class RTSPTestService : public IRTSPService
{
public:
	RTSPTestService(IApp* app, const std::string& name)
		:IRTSPService(app, name, RTSPServer::RFC7826)
	{}

};



















bool RTSPTestServer::Initialize(Promise* promise)
{
	rtspservice_ = new RTSPTestService(this, "test");
	return true;
}

void RTSPTestServer::On(Promise* promise)
{
	const json& j = promise->meta();
	std::string action = j["action"];
	if (action == "startup")
	{
		Startup(promise);
	}
	else if (action == "stop")
	{
		Stop(promise);
    }
    else
    {
        promise->reject("Action: " + action + " is not supported!");
    }
}

void RTSPTestServer::Startup(Promise* promise)
{
	const json& j = promise->data();

	const std::string& launch = j["launch"];	
	const std::string& path = j["path"];
	
	RTSPTestService* rtsp = static_cast<RTSPTestService*>(rtspservice_);
	rtsp->Launch(path, launch,NULL,NULL);
	promise->resolve();


}


void RTSPTestServer::Stop(Promise* promise)
{
	RTSPTestService* rtsp = static_cast<RTSPTestService*>(rtspservice_);
	rtsp->Stop();
	promise->resolve();


}



void RTSPTestServer::Destroy(Promise* promise)
{
    if (!rtspservice_)
    {
        delete rtspservice_;
    }
}
