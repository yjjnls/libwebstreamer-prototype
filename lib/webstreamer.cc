#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"
#include "promise.h"

#include "webstreamer.h"

//TODO: lock for global share MainLoop
static GThread      *_main_thread = NULL;
GMainLoop*     WebStreamer::main_loop = NULL;;
GMainContext*  WebStreamer::main_context = NULL;

using json = nlohmann::json;
//static gboolean running_callback(GAsyncQueue *queue)
//{
//	g_async_queue_push(queue, "ready");
//	return G_SOURCE_REMOVE;
//}


//static gboolean _promise_callback(Promise *promise)
//{
//
//	const json& j = promise->param();
//	std::string name = j["name"];
//	std::string type = j["type"];
//	std::string action = j["action"];
//	if (action == "create") {
//		printf("create processor!\n");
//		promise->resolve();
//		delete promise;
//		promise = NULL;
//	}
//	return G_SOURCE_REMOVE;
//}

static gpointer mainloop(GAsyncQueue *queue)
{
	WebStreamer::main_context = g_main_context_new();
	WebStreamer::main_loop = g_main_loop_new(WebStreamer::main_context, FALSE);

	g_async_queue_push(queue, "ready");

	g_main_loop_run(  WebStreamer::main_loop);
	g_main_loop_unref(WebStreamer::main_loop);

	return NULL;
}

/**
* :
*
* Creates a new thread and runs the libwegstreamer main-loop inside that thread.
* This function does not return until the thread has started and the mainloop is running.
*/
//bool webstreamer_initialize(void)
//{
//	gst_init(NULL, NULL);
//	
//	GAsyncQueue *queue = g_async_queue_new();
//
//
//	_main_thread = g_thread_new("webstreamer_main_loop", (GThreadFunc)mainloop, queue);
//
//	g_async_queue_pop(queue);
//	g_async_queue_unref(queue);
//	return true;
//}
//
//
//void webstreamer_terminate(void)
//{
//	if (_main_loop)
//	{
//		g_main_loop_quit(_main_loop);
//		gst_deinit();
//		_main_loop = NULL;
//	}
//
//}
//
//void webstreamer_post(Promise* promise) {
//	GSource *source;
//
//	source = g_idle_source_new();
//	g_source_set_callback(source, (GSourceFunc)_promise_callback, promise, NULL);
//	g_source_set_priority(source, G_PRIORITY_DEFAULT);
//	g_source_attach(source, _main_context);
//}
//
//
////http://blog.sina.com.cn/s/blog_4919705a0100brbr.html

bool WebStreamer::Initialize()
{
	gst_init(NULL, NULL);

	WebStreamer::main_context = g_main_context_new();
	WebStreamer::main_loop    = g_main_loop_new(WebStreamer::main_context, FALSE);

	GAsyncQueue*  queue = g_async_queue_new();
	_main_thread = g_thread_new("webstreamer_main_loop", (GThreadFunc)mainloop, queue);

	g_async_queue_pop(queue);
	g_async_queue_unref(queue);
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
	else if (action == "destry") {
		DestroyProcessor(promise);
	}
	else {

	}

}


void WebStreamer::CreateProcessor(Promise* promise)
{
	const json& j = promise->param();
	std::string uname = j["name"] + "@" + j["type"];

	//1.check exists ornot
	if (processors_.find(uname) != processors_.end())
	{
		promise->reject(uname + " was an exist processor.");
		return;
	}

	IProcessor* processor = Factory::Instantiate(j["type"], j["name"]);
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
