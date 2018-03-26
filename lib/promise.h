

#ifndef _LIBWEBSTREAMER_PROMISE_H_
#define _LIBWEBSTREAMER_PROMISE_H_

#include <gst/gst.h>
#include "node_plugin_interface.h"
#include "nlohmann/json.hpp"

class WebStreamer;
class IProcessor;
class Promise
{
public:
	Promise(void* iface, const void* context, nlohmann::json param)
		: iface_((node_plugin_interface_t *)iface)
		, context_(context)
		, param_(param)
		, responsed_(false)
		, processor_(nullptr)
		, webstreamer_(nullptr)
	{
	}

	void resolve(nlohmann::json param) {
		if (responsed_) {
			return;//response repated
		}

		std::string retval = param.dump();
		iface_->call_return(iface_, context_, 
			retval.c_str(), retval.size(), 0, NULL, NULL);
	}

	void resolve() {
		if (responsed_) {
			return;//response repated
		}

		iface_->call_return(iface_, context_,NULL,0, 0, NULL, NULL);
	}

	void reject(const std::string& message) {
		if (responsed_) {
			return;//response repated
		}
		iface_->call_return(iface_, context_, message.c_str(),message.size(), 1, NULL, NULL);
	}

	const nlohmann::json& param() const
	{
		return this->param_;
	}

	IProcessor* processor() { return processor_; }

	WebStreamer* webstreamer() {return webstreamer_;}
protected:
	void SetWebStreamer(WebStreamer* ws) {
		webstreamer_ = ws;
	}
	void SetProcessor(IProcessor* processor) {
		processor_ = processor;
	}
	friend class WebStreamer;

private:

	node_plugin_interface_t * iface_;
	const void*               context_;
	nlohmann::json            param_;

	bool                      responsed_;
	WebStreamer*              webstreamer_;
	IProcessor*               processor_;


};

#endif//!_LIBWEBSTREAMER_PROMISE_H_