
#ifndef _LIBWEBSTREAMER_PROCESSOR_H_
#define _LIBWEBSTREAMER_PROCESSOR_H_
#include <string>

#include <gst/gst.h>
#include "promise.h"

class WebStreamer;
class IProcessor
{
public:
	class IProcessor(const std::string& name,WebStreamer* ws)
		: name_(name)
		, pipeline_(NULL)
		, webstreamer_(ws)

	{

	}

	virtual bool Initialize(Promise* promise);
	virtual void Destroy(Promise* promise);

	virtual void On(Promise* promise) = 0;

	virtual const char* type() const = 0;
	virtual std::string uname() = 0;
protected:

	std::string name_;
	GstElement* pipeline_;
	WebStreamer* webstreamer_;

};

#endif//_LIBWEBSTREAMER_PROCESSOR_H_