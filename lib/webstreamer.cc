#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"



static GThread   *_main_thread = NULL;
static GMainLoop *_main_loop = NULL;;
static GMainContext *_main_context = NULL;


static gboolean running_callback(GAsyncQueue *queue)
{
	g_async_queue_push(queue, "ready");
	return G_SOURCE_REMOVE;
}

static gpointer mainloop(GAsyncQueue *queue)
{
	_main_context = g_main_context_new();
	_main_loop = g_main_loop_new(_main_context, FALSE);

	g_async_queue_push(queue, "ready");

	g_main_loop_run(_main_loop);
	g_main_loop_unref(_main_loop);

	return NULL;
}

/**
* :
*
* Creates a new thread and runs the libwegstreamer main-loop inside that thread.
* This function does not return until the thread has started and the mainloop is running.
*/
bool webstreamer_initialize(void)
{
	gst_init(NULL, NULL);
	
	GAsyncQueue *queue = g_async_queue_new();

	_main_thread = g_thread_new("webstreamer_main_loop", (GThreadFunc)mainloop, queue);

	g_async_queue_pop(queue);
	g_async_queue_unref(queue);
	return true;
}


void webstreamer_terminate(void)
{
	if (_main_loop)
	{
		g_main_loop_quit(_main_loop);
		gst_deinit();
		_main_loop = NULL;
	}

}