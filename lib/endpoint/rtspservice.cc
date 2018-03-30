
#include <webstreamer.h>
#include "rtspservice.h"
#include <gst/rtsp-server/rtsp-onvif-server.h>
IRTSPService::IRTSPService(IApp* app, const std::string& name, RTSPServer::Type type)
	:IEndpoint(app,name)
	,factory_(NULL)
	
{
	server_ = app->webstreamer().GetRTSPServer(type);

}

IRTSPService::~IRTSPService()
{

}


static  void Notify(gpointer      data, GObject      *where_the_object_was)
{
	g_print(" data : %x\n", data);
	g_print(" where_the_object_was : %x\n", where_the_object_was);
}
bool IRTSPService::Launch(const std::string& path, const std::string& launch,
	GCallback media_constructed, GCallback media_configure)
{
	GstRTSPServer* server =server_->server();
	GstRTSPMountPoints *mount_points = gst_rtsp_server_get_mount_points(server);

	factory_ = gst_rtsp_media_factory_new();
	/* if you want multiple clients to see the same video, set the shared property to TRUE */
	gst_rtsp_media_factory_set_shared(factory_, TRUE);

	gst_rtsp_media_factory_set_launch(factory_, launch.c_str());
	if (media_constructed)
	{
		g_signal_connect(factory_, "media-constructed", (GCallback)media_constructed, (gpointer)(this));
	}

	if (media_configure)
	{
		g_signal_connect(factory_, "media-configure", (GCallback)media_configure, (gpointer)(this));
	}


	gst_rtsp_mount_points_add_factory(mount_points, path.c_str(), factory_);
	g_object_unref(mount_points);

	GST_DEBUG("[rtsp-server] %s launched to %s",name_.c_str(),path.c_str());
	path_ = path;
	g_object_weak_ref(G_OBJECT(factory_), Notify, factory_);
	return true;
}


bool IRTSPService::Stop()
{
	if (factory_)
	{
		GstRTSPServer* server = server_->server();
		GstRTSPMountPoints *mount_points = gst_rtsp_server_get_mount_points(server);
		
		gst_rtsp_mount_points_remove_factory(mount_points, path_.c_str());
		g_object_unref(mount_points);

		GST_FIXME("mount point may be held, can not be cleanup at once.");
		
		factory_ = NULL;
	}
	return true;
}