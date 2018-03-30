#include <stdio.h>
#include <gst/gst.h>
#include "node_plugin_interface.h"
#include "webstreamer.h"

#include "nlohmann/json.hpp"
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

	if (cb)
	{
		cb(self, 0, ">>>>>Initialize done!<<<<<");
	}
}
#include <iostream>

#include <exception>

static void call(const void *self, const void *context,
	const void *data, size_t size,
	const void *meta, size_t msize)
{
	node_plugin_interface_t* iface=(node_plugin_interface_t*)self;
	if (!meta || !msize)
	{
		iface->call_return(iface, context, "empty meta", 0, 0, NULL, NULL);
		return;
	}
	nlohmann::json jmeta;
	try {
		jmeta = nlohmann::json::parse(std::string((const char*)meta, (std::size_t)msize));
	}
	catch (std::exception&) {
		iface->call_return(iface, context, "invalid meta json string.", 0, 0, NULL, NULL);
		return;
	}

	nlohmann::json jdata;
	try {
		if (data && size) {
			jdata = nlohmann::json::parse(std::string((const char*)data, (std::size_t)size));
		}
	}
	catch (std::exception&) {
		iface->call_return(iface, context, "invalid data json string.", 0, 0, NULL, NULL);
		return;
	}

	Promise* promise = new Promise((void*)self, context, jmeta,jdata);
	_webstreamer->Exec(promise);
	promise = nullptr;

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