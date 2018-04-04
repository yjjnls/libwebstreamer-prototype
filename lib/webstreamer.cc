#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"


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



WebStreamer::WebStreamer(plugin_interface_t* iface)
	: rtsp_session_pool_(NULL)
	, pool_clean_id_(0)
	, iface_(iface)
{
	for (int i=0; i < RTSPServer::SIZE ;i++)
	{
		rtspserver_[i] = nullptr;
	}

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
	if (pool_clean_id_)
	{
		g_source_remove(pool_clean_id_);
		pool_clean_id_ = 0;
	}

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
	const json& j = promise->meta();

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
		IApp* app = GetApp(name, type);
		if (!app) {
			promise->reject("processor not exists.");
			return;
		}
		app->On(promise);
		return;
	}

}


void WebStreamer::CreateProcessor(Promise* promise)
{
	const json& j = promise->meta();
	std::string name = j["name"];
	std::string type = j["type"];
	std::string uname = name + "@" + type;

	IApp* app = GetApp(name, type);
	if (app)
	{
		promise->reject(uname + " was an exist processor.");
		return;
	}

	app = Factory::Instantiate(type, name, this);
	if (!app)
	{
		promise->reject("type not supported");
		return;
	}
	

	if(app->Initialize(promise) )
	{
		apps_[uname] = app;
	}
	else
	{
		delete app;
		promise->reject("app initialize failed.");
		return;
	}
}
void WebStreamer::DestroyProcessor(Promise* promise) {
	promise->resolve();
	delete promise;

}

static gboolean pool_cleanup(GstRTSPSessionPool* *pool)
{
	if (*pool)
	{
		gst_rtsp_session_pool_cleanup(*pool);
		return G_SOURCE_CONTINUE;
	}
	return G_SOURCE_REMOVE;

}

std::string WebStreamer::InitRTSPServer(const nlohmann::json* option)
{
	std::string error;
	//not start rtsp server
	if (!option || option->find("rtsp_server") == option->cend())
	{
		return "";
	}
	const nlohmann::json& opt = *option;
	auto rtsp_server = opt["rtsp_server"];

	gint max_sessions = rtsp_server["max_sessions"];
	rtsp_session_pool_ = gst_rtsp_session_pool_new();
	gst_rtsp_session_pool_set_max_sessions(rtsp_session_pool_, max_sessions);


	gint port = 554;
	json::const_iterator it = rtsp_server.find("port");
	if (it != rtsp_server.cend())
	{
		port = rtsp_server["port"];
		RTSPServer* server = new RTSPServer(RTSPServer::RFC7826);
		if (!server) 
		{
			error = "create RTSP Server failed ";
			goto _failed;
		}

		if (!server->Initialize(rtsp_session_pool_, port))
		{
			error = "initialize RTSP Server failed ";
			goto _failed;
		}
		rtspserver_[RTSPServer::RFC7826] = server;
	}

	it   = rtsp_server.find("onvif_port");
	if (it != rtsp_server.cend())
	{
		port = rtsp_server["onvif_port"];
		RTSPServer* server = new RTSPServer(RTSPServer::ONVIF);
		if (!server)
		{
			error = "create Onvif RTSP Server failed ";
			goto _failed;
		}

		if (!server->Initialize(rtsp_session_pool_, port))
		{
			error = "initialize Onvif RTSP Server failed ";
			goto _failed;
		}
		rtspserver_[RTSPServer::ONVIF] = server;
	}


	pool_clean_id_=g_timeout_add_seconds(10, (GSourceFunc)pool_cleanup, &rtsp_session_pool_);

	return "";//success
_failed:
	for (int i = 0; i < RTSPServer::SIZE; i++)
	{
		if (rtspserver_[i])
		{
			rtspserver_[i]->Destroy();
			delete rtspserver_[i];
			rtspserver_[i] = nullptr;
		}
	}

	if (rtsp_session_pool_)
	{
		g_object_unref(rtsp_session_pool_);
		rtsp_session_pool_ = NULL;
	}
	return error;
}


#include "plugin_interface.h"
void WebStreamer::Notify(plugin_buffer_t* data, plugin_buffer_t* meta)
{
	if (!this->iface_ || !this->iface_->notify || (!data && !meta) )
	{
		return;
	}

	iface_->notify(iface_,data, meta);

}
