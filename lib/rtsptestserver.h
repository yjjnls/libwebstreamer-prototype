
#ifndef _LIBWEBSTREAMER_RTSP_TEST_SERVER_H_
#define _LIBWEBSTREAMER_RTSP_TEST_SERVER_H_
#include "processor.h"

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


	RTSPTestServer(const std::string& name )
		: IProcessor( name )
	{
		
	}
};


#endif //!_LIBWEBSTREAMER_RTSP_TEST_SERVER_H_