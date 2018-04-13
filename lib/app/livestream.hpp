
#ifndef _LIBWEBSTREAMER_APPLICATION_LIVESTREAM_HPP_
#define _LIBWEBSTREAMER_APPLICATION_LIVESTREAM_HPP_

// #include <gst/rtsp-server/rtsp-server.h>
// #include <gst/rtsp-server/rtsp-session-pool.h>

#include <framework/app.h>

// #define USE_AUTO_SINK 1
namespace libwebstreamer
{
namespace application
{
struct sink_link
{
    GstPad *tee_pad;
    GstElement *upstream_joint;
    void *pipeline;
    gboolean video_probe_invoke_control;
    gboolean audio_probe_invoke_control;

    sink_link(GstPad *pad, GstElement *joint, void *pipe)
        : upstream_joint(joint)
        , pipeline(pipe)
        , tee_pad(pad)
        , video_probe_invoke_control(FALSE)
        , audio_probe_invoke_control(FALSE)
    {
    }
};
class LiveStream : public IApp
{
public:
    APP(LiveStream)

    LiveStream(const std::string &name, WebStreamer *ws);
    ~LiveStream();
    void add_pipe_joint(GstElement *upstream_joint);
    void remove_pipe_joint(GstElement *upstream_joint);

    void On(Promise *promise);
    bool Initialize(Promise *promise);
    void Destroy(Promise *promise);

protected:
    void add_performer(Promise *promise);
    void Startup(Promise *promise);
    void Stop(Promise *promise);

    bool on_add_endpoint(IEndpoint *endpoint);
    virtual bool on_remove_endpoint(const std::shared_ptr<libwebstreamer::framework::Endpoint> endpoint);
    virtual bool MessageHandler(GstMessage *msg);

private:
    static GstPadProbeReturn on_tee_pad_remove_video_probe(GstPad *pad, GstPadProbeInfo *probe_info, gpointer data);
    static GstPadProbeReturn on_tee_pad_remove_audio_probe(GstPad *pad, GstPadProbeInfo *probe_info, gpointer data);
    static GstPadProbeReturn on_monitor_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    GstElement *video_tee_;
    GstElement *audio_tee_;
    IEndpoint *performer_;
    std::list<IEndpoint *> audiences_;

    std::list<sink_link *> sinks_;//all the request pad of tee, release when removing from pipeline

    GstPad *video_tee_pad_;
    GstElement *fake_video_queue_;
    GstElement *fake_video_sink_;
    GstPad *audio_tee_pad_;
    GstElement *fake_audio_queue_;
    GstElement *fake_audio_sink_;
#ifdef USE_AUTO_SINK
    GstElement *fake_video_decodec_;
    GstElement *fake_audio_decodec_;
    GstElement *fake_audio_convert_;
    GstElement *fake_audio_resample_;
#endif
};
}
}
#endif//!_LIBWEBSTREAMER_APPLICATION_LIVESTREAM_HPP_