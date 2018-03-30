

#ifndef _LIBWEBSTREAMER_WEBSTREAMER_H_
#define _LIBWEBSTREAMER_WEBSTREAMER_H_

#include <stdio.h>
#include <gst/gst.h>

#include "nlohmann/json.hpp"

#include <map>

#include "app/rtsptestserver.h"
#include "app/avanalyzer.h"

#include "framework/rtspserver.h"

#define ERRORMSG(name,message)\
   "{\"name\": \"" name "\", \"message\": \"" message "\"}"




//forward
template<typename... Args>
struct AppFactory;

//baic
template<typename First, typename... Rest>
struct AppFactory<First, Rest...>
{
	//enum { value = Sum<First>::value + Sum<Rest...>::value };
	static IApp* Instantiate(const std::string& type, const std::string& name, WebStreamer* ws)
	{
		if (type == First::CLASS_NAME()) {
			return new First(name,ws);
		}
		else 
		{
			return AppFactory<Rest...>::Instantiate(type, name,ws);
		}
	}

};

//end
template<typename Last>
struct AppFactory<Last>
{
	static IApp* Instantiate(const std::string& type, const std::string& name,WebStreamer* ws)
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

	RTSPServer* GetRTSPServer(RTSPServer::Type type = RTSPServer::RFC7826)
	{
		if (type >= RTSPServer::SIZE)
		{
			return NULL;
		}
		return rtspserver_[type];
	}
protected:
	typedef AppFactory< RTSPTestServer, AVAnalyzer
	> Factory;

	void CreateProcessor(Promise* promise);
	void DestroyProcessor(Promise* promise);
//	void ForwardPromiseToProcessor(Promise* promise);

	void OnPromise(Promise* promise);
	static gboolean OnPromise(gpointer user_data);

	inline IApp* GetApp(const std::string& name, const std::string& type)
	{
		return GetApp(name + "@" + type);
	}

	inline IApp* GetApp(const std::string& uname)
	{
		std::map<std::string, IApp*>::iterator it = apps_.find(uname);
		return   (it == apps_.end()) ? NULL : it->second;

	}

//	RTSPService* CreateRTSPService(bool onvif = false);
	


private:
	//GThread*      main_thread_;
	//GMainLoop*    main_loop_;
	//GMainContext* main_context_; g_main_context_push_thread_default
	//GAsyncQueue*  main_start_queue_; 

//	GstRTSPServer*       rtsp_server_;//create an global shared server
	RTSPServer*          rtspserver_[RTSPServer::SIZE];
	GstRTSPSessionPool*  rtsp_session_pool_;
	guint pool_clean_id_;
	std::map<std::string, IApp*> apps_;
};

#endif