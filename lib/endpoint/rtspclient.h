
#ifndef _LIBWEBSTREAMER_ENDPOINT_RTSP_CLIENT_H_
#define _LIBWEBSTREAMER_ENDPOINT_RTSP_CLIENT_H_


#include <framework/endpoint.h>
#include <list>

class RtspClient : public IEndpoint
{
public:
    RtspClient(IApp *app, const std::string &name);
    ~RtspClient();
    virtual bool initialize(Promise *promise);
    virtual void terminate();
    bool add_to_pipeline();
    // virtual bool remove_from_pipeline();

private:
    static void on_rtspsrc_pad_added(GstElement *src, GstPad *src_pad, gpointer depay);
    static gboolean on_rtspsrc_select_stream(GstElement *src, guint stream_id, GstCaps *stream_caps, gpointer rtspclient);
    // static void on_rtp_time_out(GstElement *rtpbin, guint session, guint ssrc, gpointer user_data);
    // static void on_get_new_rtpbin(GstElement *rtspsrc, GstElement *manager, gpointer user_data);
    //to del
    static GstPadProbeReturn on_monitor_data(GstPad *pad, GstPadProbeInfo *info, gpointer rtspclient);

    GstElement *rtspsrc_;
    GstElement *rtpdepay_video_;
    GstElement *parse_video_;
    GstElement *rtpdepay_audio_;
};



#endif