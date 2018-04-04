#include <stdio.h>
#include <gst/gst.h>
#include "plugin_interface.h"
#include "webstreamer.h"

#include "nlohmann/json.hpp"
#include <exception>
static WebStreamer* _webstreamer = NULL;

#define __VERSION__ "0.1.1"


static void init(const void*  iface,
	const void*               context,
	const plugin_buffer_t*    data,
	plugin_callback_fn        callback)
{
	plugin_interface_t* self = (plugin_interface_t *)iface;

	nlohmann::json j;
	if (data)
	{
		try {

			j = nlohmann::json::parse(std::string((const char*)data->data, data->size));
		}
		catch (std::exception& ){
			plugin_buffer_t err;
			plugin_buffer_string_set(&err, 
				const_error_msg("init", "invalid option format(should be json)."));
			callback(self, context, 1, &err);
			return;
		}
	}

	std::string error;
	_webstreamer = new WebStreamer();
	if (!_webstreamer->Initialize(j.is_null() ? NULL :&j ,error))
	{
		plugin_buffer_t err;
		plugin_buffer_string_set(&err,
			const_error_msg("init", "gstreamer initialize failed."));
		callback(self, context, 1, &err);
		return;
	}

	if (callback)
	{
		callback(self, context, 0, NULL);
	}
}
#include <iostream>

#include <exception>


static void call(const void* iface,
	const void*               context,
	plugin_buffer_t*          data,
	plugin_buffer_t*          meta,
	plugin_callback_fn        callback)
{
	plugin_interface_t* self = (plugin_interface_t *)iface;

	if (!meta || !meta->data || !meta->size)
	{
		plugin_buffer_t err;
		plugin_buffer_string_set(&err,
			const_error_msg("call", "action not specified."));
		callback(self, context, 1, &err);
		return;
	}
	nlohmann::json jmeta;
	try {
		const char* begin = (const char*)meta->data;
		const char* end = begin + meta->size;

		jmeta = nlohmann::json::parse(begin,end);
	}
	catch (std::exception&) {
		plugin_buffer_t err;
		plugin_buffer_string_set(&err,
			const_error_msg("call", "invalid meta json string."));
		callback(self, context, 1, &err);
		return;
	}

	nlohmann::json jdata;
	try {
		if (data && data->data && data->size) {
			const char* begin = (const char*)data->data;
			const char* end = begin + data->size;
			jdata = nlohmann::json::parse(begin,end);
		}
	}
	catch (std::exception&) {
		plugin_buffer_t err;
		plugin_buffer_string_set(&err,
			const_error_msg("call", "invalid data json string."));
		callback(self, context, 1, &err);
		return;
	}

	Promise* promise = new Promise((void*)self, context,callback, jmeta,jdata);
	_webstreamer->Exec(promise);
	promise = nullptr;

}

static void terminate(const void* iface,
	const void*               context,
	plugin_callback_fn        callback)
{
	plugin_interface_t* self = (plugin_interface_t *)iface;

	_webstreamer->Terminate();
	if (callback)
	{
		callback(self, context,0,NULL);
	}
}

PLUGIN_INTERFACE(__VERSION__, init, call, terminate)