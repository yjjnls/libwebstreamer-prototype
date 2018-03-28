

#ifndef _LIBWEBSTREAMER_WEBSTREAMER_H_
#define _LIBWEBSTREAMER_WEBSTREAMER_H_

#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"
#include "promise.h"
#include <map>

#include "rtsptestserver.h"

//static GThread   *_main_thread = NULL;
//static GMainLoop *_main_loop = NULL;;
//static GMainContext *_main_context = NULL;








//bool webstreamer_initialize(void);
//void webstreamer_terminate(void);

#define ERRORMSG(name,message)\
   "{\"name\": \"" name "\", \"message\": \"" message "\"}"




//forward
template<typename... Args>
struct ProcessorFactory;

//baic
template<typename First, typename... Rest>
struct ProcessorFactory<First, Rest...>
{
	//enum { value = Sum<First>::value + Sum<Rest...>::value };
	static IProcessor* Instantiate(const std::string& type, const std::string& name, WebStreamer* ws)
	{
		if (type == First::CLASS_NAME()) {
			return new First(name,ws);
		}
		else 
		{
			return ProcessorFactory<Rest...>::Instantiate(type, name);
		}
	}

};

//end
template<typename Last>
struct ProcessorFactory<Last>
{
	static IProcessor* Instantiate(const std::string& type, const std::string& name,WebStreamer* ws)
	{
		if (type == Last::CLASS_NAME() ) {
			return new Last(name,ws);
		}
		return NULL;
	}
};





class RTSPService;


class WebStreamer {

public:
	WebStreamer();
	bool Initialize(const nlohmann::json* option, std::string& error);
	void Terminate();

	std::string InitRTSPServer(const nlohmann::json* option);

	void Exec(Promise* promise);
	static GMainLoop*    main_loop;
	static GMainContext* main_context;

	GstRTSPServer* RTSPServer() { return rtsp_server_; }
protected:
	typedef ProcessorFactory< RTSPTestServer > Factory;

	void CreateProcessor(Promise* promise);
	void DestroyProcessor(Promise* promise);
	void ForwardPromiseToProcessor(Promise* promise);

	void OnPromise(Promise* promise);
	static gboolean OnPromise(gpointer user_data);

	inline IProcessor* GetProcessor(const std::string& name, const std::string& type)
	{
		return GetProcessor(name + "@" + type);
	}

	inline IProcessor* GetProcessor(const std::string& uname)
	{
		std::map<std::string, IProcessor*>::iterator it = processors_.find(uname);
		return   (it == processors_.end()) ? NULL : it->second;

	}

	RTSPService* CreateRTSPService(bool onvif = false);

private:
	//GThread*      main_thread_;
	//GMainLoop*    main_loop_;
	//GMainContext* main_context_; g_main_context_push_thread_default
	//GAsyncQueue*  main_start_queue_; 

	GstRTSPServer*       rtsp_server_;//create an global shared server
	GstRTSPSessionPool*  rtsp_session_pool_;
	std::map<std::string, IProcessor*> processors_;
};

#endif