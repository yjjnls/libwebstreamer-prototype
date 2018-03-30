
#ifndef _LIBWEBSTREAMER_ENDPOINT_RTSP_SERVICE_H_
#define _LIBWEBSTREAMER_ENDPOINT_RTSP_SERVICE_H_


#include <framework/endpoint.h>
#include <framework/rtspserver.h>

class IRTSPService : public IEndpoint
{
public:
	IRTSPService( IApp* app, const std::string& name, RTSPServer::Type type);
	~IRTSPService();

	virtual bool Launch(const std::string& path, const std::string& launch, 
		         GCallback media_constructed, GCallback media_configure);
	virtual bool Stop();
protected:
	GstRTSPMediaFactory * factory_;
	RTSPServer*           server_;
	std::string           path_;
};



#endif