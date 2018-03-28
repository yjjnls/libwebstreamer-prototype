#include "webstreamer.h"
#include "rtsptestserver.h"

using json = nlohmann::json;


bool RTSPTestServer::Initialize(Promise* promise)
{
	promise->resolve();
	return true;
}

void RTSPTestServer::On(Promise* promise)
{
	const json& j = promise->param();
	std::string action = j["action"];
	if (action == "startup")
	{
		Startup(promise);
	}
}

void RTSPTestServer::Startup(Promise* promise)
{
	const json& j = promise->param();
	std::string action = j["action"];
	if (j.find("content") == j.cend())
	{
		promise->reject("null param.");
		return;
	}

	auto opt = j["content"];

	//session_ = gst_rtsp_session_pool_new();
	//
	//gst_rtsp_session_pool_set_max_sessions(session_, 255);
	//
	//server_ = gst_rtsp_server_new();

	mounts_ = gst_rtsp_server_get_mount_points(webstreamer_->RTSPServer());

	factory_ = gst_rtsp_media_factory_new();

	const std::string& source = opt["source"];

	gst_rtsp_media_factory_set_launch(factory_, source.c_str());
	//"( videotestsrc is-live=1 ! x264enc ! rtph264pay name=pay0 pt=96 )");


	gst_rtsp_media_factory_set_shared(factory_, TRUE);

	const std::string& path = opt["path"];

	gst_rtsp_mount_points_add_factory(mounts_, path.c_str(), factory_);

	g_object_unref(mounts_);


	promise->resolve();


}




void RTSPTestServer::Destroy(Promise* promise)
{

}
