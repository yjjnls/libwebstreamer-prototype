#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"
#include "promise.h"

#include "webstreamer.h"

//TODO: lock for global share MainLoop
static GThread      *_main_thread = NULL;
//GstRTSPServer* WebStreamer::rtsp_server = NULL;;
GMainLoop*     WebStreamer::main_loop = NULL;;
GMainContext*  WebStreamer::main_context = NULL;

using json = nlohmann::json;

class WebStreamerInitialization
{
public:
	WebStreamerInitialization( WebStreamer* ws,const json* opt)
		: webstreamer(ws)
		, option(opt)
	{
		queue = g_async_queue_new();
	}

	~WebStreamerInitialization() {
		webstreamer = NULL;
		g_async_queue_unref(queue);
	}
	WebStreamer* webstreamer;
	GAsyncQueue* queue; //for sync usage
	const json*  option;
	std::string  error;

};
static gboolean main_loop_init(WebStreamerInitialization *wsi)
{
	//g_return_val_if_fail(msg_queue, FALSE);
	const json* j = wsi->option;
	std::string error = wsi->webstreamer->InitRTSPServer(wsi->option);
	if (!error.empty())
	{
		
		g_async_queue_push(wsi->queue, g_strdup(error.c_str()));
	}
	g_async_queue_push(wsi->queue, g_strdup("ready"));
	return G_SOURCE_REMOVE;
}

static gpointer mainloop(WebStreamerInitialization *wsi)
{
	WebStreamer::main_context = g_main_context_new();
	WebStreamer::main_loop = g_main_loop_new(WebStreamer::main_context, FALSE);
	g_main_context_push_thread_default(WebStreamer::main_context);

	
	GSource* idle_source = g_idle_source_new();
	g_source_set_callback(idle_source, (GSourceFunc)main_loop_init, wsi, NULL);
	g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
	g_source_attach(idle_source, WebStreamer::main_context);


	g_main_loop_run(  WebStreamer::main_loop);
	g_main_loop_unref(WebStreamer::main_loop);

	g_main_context_pop_thread_default(WebStreamer::main_context);


	return NULL;
}



////http://blog.sina.com.cn/s/blog_4919705a0100brbr.html



WebStreamer::WebStreamer()
	: rtsp_server_(NULL)
{

}
bool WebStreamer::Initialize(const nlohmann::json* option, std::string& error)
{
	gst_init(NULL, NULL);

	WebStreamerInitialization* wsi = new WebStreamerInitialization(this,option);

	_main_thread = g_thread_new("webstreamer_main_loop", (GThreadFunc)mainloop, wsi);

	char* p = (char*)g_async_queue_pop(wsi->queue);
	
	return true;

}
void WebStreamer::Terminate()
{
	if (WebStreamer::main_loop)
	{
		g_main_loop_quit(WebStreamer::main_loop);
		gst_deinit();
		WebStreamer::main_loop  = NULL;
	}
}

gboolean WebStreamer::OnPromise(gpointer user_data)
{
	Promise* promise = (Promise*)user_data;
	WebStreamer* This = promise->webstreamer();
	This->OnPromise(promise);
	return G_SOURCE_REMOVE;
}

void WebStreamer::Exec(Promise* promise)
{
	GSource *source;

	source = g_idle_source_new();
	promise->SetWebStreamer(this);
	g_source_set_callback(source, WebStreamer::OnPromise, promise, NULL);
	g_source_set_priority(source, G_PRIORITY_DEFAULT);
	g_source_attach(source, WebStreamer::main_context);

}

void WebStreamer::OnPromise(Promise *promise)
{
	const json& j = promise->param();

	std::string action = j["action"];
	if (action == "create") {
		CreateProcessor(promise);
	}
//	else if (action == "destry") {
//		DestroyProcessor(promise);
//	}
	else {
		const std::string& name = j["name"];
		const std::string& type = j["type"];
		IProcessor* processor = GetProcessor(name, type);
		if (!processor) {
			promise->reject("processor not exists.");
			return;
		}
		processor->On(promise);
		return;
	}

}


void WebStreamer::CreateProcessor(Promise* promise)
{
	const json& j = promise->param();
	std::string name = j["name"];
	std::string type = j["type"];
	std::string uname = name + "@" + type;

	IProcessor* processor = GetProcessor(name, type);
	if (processor)
	{
		promise->reject(uname + " was an exist processor.");
		return;
	}

	processor = Factory::Instantiate(type, name, this);
	if (!processor)
	{
		promise->reject("type not supported");
		return;
	}
	

	if( processor->Initialize(promise) )
	{
		processors_[uname] = processor;
	}
	else
	{
		delete processor;
	}
}
void WebStreamer::DestroyProcessor(Promise* promise) {
	promise->resolve();
	delete promise;

}


std::string WebStreamer::InitRTSPServer(const nlohmann::json* option)
{
	//not start rtsp server
	if (!option || option->find("rtsp_server") == option->cend())
	{
		return "";
	}
	const nlohmann::json& opt = *option;

	rtsp_server_ = gst_rtsp_server_new();
	if (!rtsp_server_) {
		return "create RTSP server failed.";
	}

	gint port = 554;
	auto rtsp_server = opt["rtsp_server"];
	json::const_iterator it = rtsp_server.find("port");
	if (it != rtsp_server.cend())
	{
		port = rtsp_server["port"];
	}
	char s[128];
	
	sprintf(s, "%d", port);
	gst_rtsp_server_set_service(rtsp_server_, s);

	gint max_sessions = rtsp_server["max_sessions"];
	rtsp_session_pool_ = gst_rtsp_session_pool_new();
	gst_rtsp_session_pool_set_max_sessions(rtsp_session_pool_, max_sessions);

	gst_rtsp_server_set_session_pool(rtsp_server_, rtsp_session_pool_);

	gst_rtsp_server_attach(rtsp_server_, WebStreamer::main_context);


	return "";//success
}
