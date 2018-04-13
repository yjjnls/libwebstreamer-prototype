
#include <endpoint/rtspclient.h>
using json = nlohmann::json;

RtspClient::RtspClient(IApp *app, const std::string &name)
    : IEndpoint(app, name)
{
}

RtspClient::~RtspClient()
{
}

bool RtspClient::initialize(Promise *promise)
{
    GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category
    GST_DEBUG_CATEGORY_INIT(my_category, "webstreamer", 2, "libWebStreamer");
    try
    {
        const json &j = promise->data();
        const std::string &url = j["url"];
        rtspsrc_ = gst_element_factory_make("rtspsrc", "rtspsrc");
        g_object_set(G_OBJECT(rtspsrc_), "location", url.c_str(), NULL);
        const std::string video_codec = j["video_codec"];
        app()->video_encoding() = video_codec;
        const std::string audio_codec = j["audio_codec"];
        app()->audio_encoding() = audio_codec;
    }
    catch (...)
    {
    }
    return add_to_pipeline();
}


GstPadProbeReturn RtspClient::on_monitor_data(GstPad *pad, GstPadProbeInfo *info, gpointer rtspclient)
{
    static int count = 0;
    RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    auto pipeline = rtsp_client->pipeline_owner().lock();

    // printf(".%d", GST_STATE(pipeline->pipeline()));
    printf(".");
    return GST_PAD_PROBE_OK;
}
void RtspClient::on_rtspsrc_pad_added(GstElement *src, GstPad *src_pad, gpointer rtspclient)
{
    RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    GstCaps *caps = gst_pad_query_caps(src_pad, NULL);
    GstStructure *stru = gst_caps_get_structure(caps, 0);
    const GValue *media_type = gst_structure_get_value(stru, "media");

    auto pipeline = rtsp_client->app();

    if (g_str_equal(g_value_get_string(media_type), "video"))
    {
        if (!pipeline->video_encoding().empty())
        {
            GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT_CAST(rtsp_client->rtpdepay_video_), "sink");
            GstPadLinkReturn ret = gst_pad_link(src_pad, sink_pad);
            g_warn_if_fail(ret == GST_PAD_LINK_OK);
            gst_object_unref(sink_pad);
            // gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, rtspclient, NULL);
        }
    }
    else if (g_str_equal(g_value_get_string(media_type), "audio"))
    {
        if (!pipeline->audio_encoding().empty())
        {
            GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT_CAST(rtsp_client->rtpdepay_audio_), "sink");
            GstPadLinkReturn ret = gst_pad_link(src_pad, sink_pad);
            g_warn_if_fail(ret == GST_PAD_LINK_OK);
            gst_object_unref(sink_pad);
            // gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, rtspclient, NULL);
        }
    }
    else
    {
        g_warn_if_reached();
    }
}
gboolean RtspClient::on_rtspsrc_select_stream(GstElement *src, guint stream_id, GstCaps *stream_caps, gpointer rtspclient)
{
    RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    GstStructure *stru = gst_caps_get_structure(stream_caps, 0);
    std::string media_type(gst_structure_get_string(stru, "media"));
    if (media_type == "video")
    {
        if (!rtsp_client->app()->video_encoding().empty())
            return TRUE;
    }
    if (media_type == "audio")
    {
        if (!rtsp_client->app()->audio_encoding().empty())
            return TRUE;
    }
    if (gst_structure_has_field(stru, "a-recvonly"))
    {
    }
    return FALSE;
}
bool RtspClient::add_to_pipeline()
{
    auto pipeline = app();
    static int added = 0;
    if (added++ == 0)
    {
        gst_bin_add(GST_BIN(pipeline->pipeline()), rtspsrc_);
        g_signal_connect(rtspsrc_, "pad-added", (GCallback)on_rtspsrc_pad_added, this);
        g_signal_connect(rtspsrc_, "select-stream", (GCallback)on_rtspsrc_select_stream, this);
    }

    if (!pipeline->video_encoding().empty())
    {
        VideoEncodingType video_codec = get_video_encoding_type(pipeline->video_encoding());
        switch (video_codec)
        {
            case VideoEncodingType::H264:
                rtpdepay_video_ = gst_element_factory_make("rtph264depay", "depay");
                parse_video_ = gst_element_factory_make("h264parse", "parse");
                break;
            case VideoEncodingType::H265:
                rtpdepay_video_ = gst_element_factory_make("rtph265depay", "depay");
                parse_video_ = gst_element_factory_make("h265parse", "parse");
                break;
            default:
                GST_WARNING("[rtsp-client] Invalid Video Codec!");
                return false;
        }
        g_warn_if_fail(rtpdepay_video_ && parse_video_);

        g_warn_if_fail(app()->pipeline() != NULL);
        gst_bin_add_many(GST_BIN(app()->pipeline()), rtpdepay_video_, parse_video_, NULL);
        g_warn_if_fail(gst_element_link(rtpdepay_video_, parse_video_));
        GST_DEBUG("[rtsp-client] configured video: %s", pipeline->video_encoding().c_str());

        // g_signal_connect(rtspsrc_, "new-manager", (GCallback)on_get_new_rtpbin, this);
    }
    if (!pipeline->audio_encoding().empty())
    {
        AudioEncodingType audio_codec = get_audio_encoding_type(pipeline->audio_encoding());
        switch (audio_codec)
        {
            case AudioEncodingType::PCMA:
                rtpdepay_audio_ = gst_element_factory_make("rtppcmadepay", "audio-depay");
                break;
            case AudioEncodingType::PCMU:
                rtpdepay_audio_ = gst_element_factory_make("rtppcmudepay", "audio-depay");
                break;
            case AudioEncodingType::OPUS:
                rtpdepay_audio_ = gst_element_factory_make("rtpopusdepay", "audio-depay");
                break;
            default:
                GST_WARNING("[rtsp-client] Invalid Audio Codec!");
                return false;
        }

        g_warn_if_fail(rtpdepay_audio_);
        gst_bin_add_many(GST_BIN(app()->pipeline()), rtpdepay_audio_, NULL);
        GST_DEBUG("[rtsp-client] configured audio: %s", pipeline->audio_encoding().c_str());        
    }

    return true;
}

void RtspClient::terminate()
{
    gst_bin_remove_many(GST_BIN(app()->pipeline()), rtspsrc_, rtpdepay_video_, parse_video_, NULL);
    gst_bin_remove_many(GST_BIN(app()->pipeline()), rtpdepay_audio_, NULL);
}

bool RtspClient::remove_from_pipeline()
{
    gst_bin_remove_many(GST_BIN(pipeline_owner().lock()->pipeline()), rtspsrc_, rtpdepay_video_, parse_video_, NULL);
    gst_bin_remove_many(GST_BIN(pipeline_owner().lock()->pipeline()), rtpdepay_audio_, NULL);
    return true;
}