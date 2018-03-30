

#ifndef _LIBWEBSTREAMER_PROMISE_H_
#define _LIBWEBSTREAMER_PROMISE_H_

#include <gst/gst.h>
#include "node_plugin_interface.h"
#include "nlohmann/json.hpp"

class WebStreamer;
class IApp;
class Promise
{
public:
	Promise(void* iface, const void* context, nlohmann::json jmeta, nlohmann::json jdata)
		: iface_((node_plugin_interface_t *)iface)
		, context_(context)
		, jmeta_(jmeta)
		, jdata_(jdata)
		, responsed_(false)
		, app_(nullptr)
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

	const nlohmann::json& data() const
	{
		return this->jdata_;
	}

	const nlohmann::json& meta() const
	{
		return this->jmeta_;
	}

	//IProcessor* processor() { return processor_; }
	IApp*        app() { return app_;  }
	WebStreamer* webstreamer() {return webstreamer_;}
protected:
	void SetWebStreamer(WebStreamer* ws) {
		webstreamer_ = ws;
	}
	void SetApp(IApp* app) {
		app_ = app;
	}
	friend class WebStreamer;

private:

	node_plugin_interface_t * iface_;
	const void*               context_;
	nlohmann::json            jdata_;
	nlohmann::json            jmeta_;
	bool                      responsed_;
	WebStreamer*              webstreamer_;
	IApp*                     app_;


};

#endif//!_LIBWEBSTREAMER_PROMISE_H_