#ifndef _LIBWEBSTREAMER_FRAMEWORK_ENDPOINT_
#define _LIBWEBSTREAMER_FRAMEWORK_ENDPOINT_

#include <string>

//#include "app.h"



class IApp;
class IEndpoint
{
public:
	IEndpoint(IApp* app, const std::string& name);
	~IEndpoint();

	IApp* app() { return app_; }
	const std::string& name() { return name_; }

    virtual bool initialize(Promise *promise)
    {
        return true;
    }
    virtual void terminate()
    {
    }

protected:
	IApp* app_;
	std::string name_;

};
#endif