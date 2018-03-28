
#ifndef _LIBWEBSTREAMER_RTSP_TEST_SERVER_H_
#define _LIBWEBSTREAMER_RTSP_TEST_SERVER_H_
#include "processor.h"
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-session-pool.h>

#define PROCESSOR( klass )                       \
	static const char* CLASS_NAME() {            \
        return #klass;                           \
	};                                           \
                                                 \
	const char* type() const                     \
	{                                            \
		return CLASS_NAME();                     \
	}                                            \
                                                 \
	std::string uname()                          \
	{                                            \
		return this->name_ + "@" + this->type(); \
	}                                            


class RTSPTestServer : public IProcessor
{
public:
	PROCESSOR(RTSPTestServer)


	RTSPTestServer(const std::string& name, WebStreamer* ws )
		: IProcessor( name ,ws)
		//, server_(NULL)
		,mounts_(NULL)
		,factory_(NULL)
		//,session_(NULL)
	{
		
	}

	void On(Promise* promise);
	bool Initialize(Promise* promise);
	void Destroy(Promise* promise);
protected:
	void Startup(Promise* promise);
private:
	//GstRTSPServer*       server_;
	GstRTSPMountPoints*  mounts_;
	GstRTSPMediaFactory* factory_;
	//GstRTSPSessionPool * session_;
};


#endif //!_LIBWEBSTREAMER_RTSP_TEST_SERVER_H_