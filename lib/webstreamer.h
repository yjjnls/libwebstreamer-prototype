

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
	static IProcessor* Instantiate(const std::string& type, const std::string& name)
	{
		if (type == First::CLASS_NAME()) {
			return new First(name);
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
	static IProcessor* Instantiate(const std::string& type, const std::string& name)
	{
		if (type == Last::CLASS_NAME() ) {
			return new Last(name);
		}
		return NULL;
	}
};








class WebStreamer {

public:
	bool Initialize();
	void Terminate();
	void Exec(Promise* promise);
	static GMainLoop*    main_loop;
	static GMainContext* main_context;
protected:
	typedef ProcessorFactory< RTSPTestServer > Factory;

	void CreateProcessor(Promise* promise);
	void DestroyProcessor(Promise* promise);

	void OnPromise(Promise* promise);
	static gboolean OnPromise(gpointer user_data);

private:
	//GThread*      main_thread_;
	//GMainLoop*    main_loop_;
	//GMainContext* main_context_; g_main_context_push_thread_default
	//GAsyncQueue*  main_start_queue_; 
	std::map<std::string, IProcessor*> processors_;
};

#endif