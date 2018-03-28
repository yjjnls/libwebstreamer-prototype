#include <stdio.h>
#include <gst/gst.h>
#include "node_plugin_interface.h"
#include "webstreamer.h"

#include "nlohmann/json.hpp"
#include "promise.h"
#include <exception>
static WebStreamer* _webstreamer = NULL;

#define __VERSION__ "0.1.1"


static void init(const void *self, const void *data, size_t size, void(*cb)(const void *self, int status, char *msg))
{
	nlohmann::json j;
	if (data && size)
	{
		try {

			j = nlohmann::json::parse(std::string((const char*)data, size));
		}
		catch (std::exception& ){
			cb(self, 1, ERRORMSG("init", "invalid option format(should be json)."));
			return;
		}
	}

	std::string error;
	_webstreamer = new WebStreamer();
	if (!_webstreamer->Initialize(j.is_null() ? NULL :&j ,error))
	{
		cb(self, 1, ERRORMSG("init","gstreamer initialize failed.") );
		return;
	}

	//TODO:
	//data is JSON string (utf8)
	//do as your needs
	if (data)
	{
		//do parse 
		//printf("init param <%s>\n", (char*)data);
	}
//	printf("gstreamer init!\n");
	
	//owr_init(libwebstreamer_main_context);
	//owr_run_in_background();
//	printf("gstreamer init done.\n");



	if (cb)
	{
		cb(self, 0, ">>>>>Initialize done!<<<<<");
		//error callback
		// cb(self, 1 ,"Initalize error!");
	}
}
#include <iostream>


static void call(const void *self, const void *context,
	const void *data, size_t size)
{
	
	nlohmann::json obj;
	if (data && size) {
		obj = nlohmann::json::parse( std::string((const char*)data, (std::size_t)size));
	}
	Promise* promise = new Promise((void*)self, context, obj);
	_webstreamer->Exec(promise);
	promise = nullptr;

	//static int counter = 0;
	//node_plugin_interface_t *iface = (node_plugin_interface_t *)self;
	//if (iface->call_return)
	//{
	//	int status = 0;
	//	char retval[256]="HELLO";
	//
	//	try
	//	{
	//		std::string expr((const char *)data, size);
	//		int result = calculator::eval(expr);
	//		sprintf(retval, "%d", result);
	//	}
	//	catch (calculator::error &e)
	//	{
	//		strcpy(retval, e.what());
	//		status = 1;
	//	}
	//
	//	iface->call_return(self, context, retval, strlen(retval) + 1, 0, NULL, NULL);
	//}
	//
	//counter++;
	//if (iface->notify)
	//{
	//	char log[256];
	//	sprintf(log, "* %d request has been procced.", counter);
	//	iface->notify(self, log, strlen(log) + 1, NULL, NULL);
	//}
}

static void terminate(const void *self, void(*cb)(const void *self, int status, char *msg))
{
//	owr_quit();
//	printf("gstreamer quite.\n");
	_webstreamer->Terminate();
	if (cb)
	{
		cb(self, 0, ">>>>>Terminate done!<<<<<");
		//error callback
		// cb(self, 1 ,"Terminate error!");
	}
}

NODE_PLUGIN_IMPL(__VERSION__, init, call, terminate)