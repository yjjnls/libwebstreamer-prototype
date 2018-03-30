#include "webstreamer.h"
#include "avanalyzer.h"
using json = nlohmann::json;







/* receive spectral data from element message */
static gboolean
message_handler(GstBus * bus, GstMessage * message, gpointer data)
{
	AVAnalyzer* This = static_cast<AVAnalyzer*>(data);
	This->OnMessage(bus, message);

	return TRUE;
}




bool AVAnalyzer::Initialize(Promise* promise)
{
	
	promise->resolve();
	return true;
}

void AVAnalyzer::On(Promise* promise)
{
//	const json& j = promise->param();
//	std::string action = j["action"];
//	if (action == "startup")
//	{
//		Startup(promise);
//	}
//	else if (action == "stop")
//	{
//		Stop(promise);
//	}
}

void AVAnalyzer::Startup(Promise* promise)
{
//	GError* error = NULL;
//	const json& j = promise->param();
//	std::string action = j["action"];
//	if (j.find("content") == j.cend())
//	{
//		promise->reject("null param.");
//		return;
//	}
//
//	auto opt = j["content"];
//
//
//	const std::string& launch = opt["launch"];
//	/* Build the pipeline */
//	pipeline_ = gst_pipeline_new("pipeline"); 
//	GstElement* bin = gst_parse_launch(launch.c_str(), &error);
//	gst_bin_add(GST_BIN(pipeline_), bin);
//
//	//spectrum
//	if (opt.find("spectrum") != opt.end())
//	{
//		Spectrum(opt["spectrum"]);
//	}
//	gst_element_set_state(pipeline_, GST_STATE_PLAYING);
//
//	GstBus* bus = gst_element_get_bus(pipeline_);
//	//GstBus* bus2 = gst_element_get_bus(bin);
//	gst_bus_add_watch(bus, message_handler, this);
//	gst_object_unref(bus);
//	//gst_element_set_bus(bin, bus);
//	//gst_bus_add_watch(bus, message_handler, this);
//
//	promise->resolve();
//

}


void AVAnalyzer::Stop(Promise* promise)
{
//	const json& j = promise->param();
//	std::string action = j["action"];
//
//	promise->resolve();


}



void AVAnalyzer::Destroy(Promise* promise)
{

}


void AVAnalyzer::Spectrum(const nlohmann::json::value_type& j)
{
	GstElement* spectrum = gst_bin_get_by_name(GST_BIN(pipeline_), "spectrum");

	guint bands     = j["bands"];
	gint threshold = j["threshold"];
	guint interval  = j["interval"];
	
	g_object_set(G_OBJECT(spectrum), "bands", bands, "threshold", threshold, //"interval", interval,
			"post-messages", TRUE, "message-phase", TRUE, NULL);

}


void AVAnalyzer::OnMessage(GstBus * bus, GstMessage * message)
{
	if (message->type == GST_MESSAGE_ELEMENT) {
		const GstStructure *s = gst_message_get_structure(message);
		const gchar *name = gst_structure_get_name(s);
		GstClockTime endtime;

		if (strcmp(name, "spectrum") == 0) {
			const GValue *magnitudes;
			const GValue *phases;
			const GValue *mag, *phase;
			gdouble freq;
			guint i;

			if (!gst_structure_get_clock_time(s, "endtime", &endtime))
				endtime = GST_CLOCK_TIME_NONE;

			g_print("New spectrum message, endtime %" GST_TIME_FORMAT "\n",
				GST_TIME_ARGS(endtime));

			magnitudes = gst_structure_get_value(s, "magnitude");
			phases = gst_structure_get_value(s, "phase");
#define AUDIOFREQ 32000 
			for (i = 0; i < 128; ++i) {
				freq = (gdouble)((AUDIOFREQ / 2) * i + AUDIOFREQ / 4) / 128;
				mag = gst_value_list_get_value(magnitudes, i);
				phase = gst_value_list_get_value(phases, i);

				if (mag != NULL && phase != NULL) {
					g_print("band %d (freq %g): magnitude %f dB phase %f\n", i, freq,
						g_value_get_float(mag), g_value_get_float(phase));
				}
			}
			g_print("\n");
		}
	}
}
