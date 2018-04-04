#include "app.h"
#include "../webstreamer.h"

bool IApp::Initialize(Promise* promise)
{
	if (!pipeline_)
	{
		pipeline_ = gst_pipeline_new((const gchar*)uname().c_str());
	}

	if (pipeline_) {
		promise->resolve();
		return true;
	}
	else {
		promise->reject("create pipeline failed.");
		return false;
	}
}


void IApp::Destroy(Promise* promise)
{

}


void IApp::Notify(const nlohmann::json& data, const nlohmann::json& meta)
{
	const std::string& jdata = data.dump();
	const std::string& jmeta = meta.dump();
	plugin_buffer_t d;
	plugin_buffer_t m;
	plugin_buffer_string_set(&d, jdata.c_str());
	plugin_buffer_string_set(&m, jmeta.c_str());
	this->webstreamer_->Notify(&d, &m);
}
