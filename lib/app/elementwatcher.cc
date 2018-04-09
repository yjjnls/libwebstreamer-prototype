#include "webstreamer.h"
#include "elementwatcher.h"
using json = nlohmann::json;






/* receive spectral data from element message */
static gboolean
message_handler(GstBus * bus, GstMessage * message, gpointer data)
{
	ElementWatcher* This = static_cast<ElementWatcher*>(data);
	This->OnMessage(bus, message);

	return TRUE;
}




bool ElementWatcher::Initialize(Promise* promise)
{
	
	promise->resolve();
	return true;
}

void ElementWatcher::On(Promise* promise)
{
	const json& j = promise->meta();
	std::string action = j["action"];
	if (action == "startup")
	{
		Startup(promise);
	}
	else if (action == "stop")
	{
		Stop(promise);
	}
}

void ElementWatcher::Startup(Promise* promise)
{
	GError* error = NULL;
	
	const json& j = promise->data();
	const std::string& launch = j["launch"];
	auto  elements = j["elements"];

	/* Build the pipeline */
	pipeline_ = gst_pipeline_new("pipeline"); 
//	char lst[] = ""\
//		"audiotestsrc ! audioconvert !spectrum name=spectrum ! spectrascope ! autovideosink";
	GstElement* bin = gst_parse_launch(launch.c_str(), &error);
	gst_bin_add(GST_BIN(pipeline_), bin);

	for (json::iterator it = elements.begin(); it != elements.end(); ++it) {
		std::string& name = it.key();
		auto& val = it.value();
		auto& type = val["type"];

		GstElement* element = gst_bin_get_by_name(GST_BIN(pipeline_), name.c_str());
		if (element) {
			if (type == "spectrum") {
				g_object_set(G_OBJECT(element), "post-messages", TRUE, "message-phase", TRUE, NULL);

			}
			g_object_unref(element);
		}
		else
		{
			//FIXME: release the created resources.
			promise->reject("can not get element " + name);
			return;

		}
	}
	
	this->watch_list_ = elements;
	gst_element_set_state(pipeline_, GST_STATE_PLAYING);

	GstBus* bus = gst_element_get_bus(pipeline_);
	gst_bus_add_watch(bus, message_handler, this);
	gst_object_unref(bus);

	promise->resolve();


}


void ElementWatcher::Stop(Promise* promise)
{
//	const json& j = promise->param();
//	std::string action = j["action"];
//
//	promise->resolve();


}



void ElementWatcher::Destroy(Promise* promise)
{

}


void ElementWatcher::Spectrum(const nlohmann::json::value_type& j)
{
	GstElement* spectrum = gst_bin_get_by_name(GST_BIN(pipeline_), "spectrum");

	guint bands = 128;// j["bands"];
	gint threshold = -80;// j["threshold"];
	//guint interval  = j["interval"];
	
	
	g_object_set(G_OBJECT(spectrum), "bands", bands, "threshold", threshold, //"interval", interval,
			"post-messages", TRUE, "message-phase", TRUE, NULL);

}


void ElementWatcher::OnMessage(GstBus * bus, GstMessage * message)
{
	if (message->type == GST_MESSAGE_ELEMENT) {
		const GstStructure *s = gst_message_get_structure(message);
		const gchar *name = gst_structure_get_name(s);

		for (json::iterator it = watch_list_.begin(); it != watch_list_.end(); ++it) {
			std::string& ename = it.key();
			auto& val = it.value();
			auto& type = val["type"];

			if (ename != it.key())
				continue;
			if( type == "spectrum")
			{
				OnSpectrum(ename, val, s);

			}

		}
#if 0
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
			guint n = gst_value_list_get_size(magnitudes);
#define AUDIOFREQ 32000 
			for (i = 0; i < 128; ++i) {
				freq = (gdouble)((AUDIOFREQ / 2) * i + AUDIOFREQ / 4) / 128;
				mag = gst_value_list_get_value(magnitudes, i);
				phase = gst_value_list_get_value(phases, i);

				if (mag != NULL && phase != NULL) {
					g_print("****  band %d (freq %g): magnitude %f dB phase %f\n", i, freq,
						g_value_get_float(mag), g_value_get_float(phase));
				}
			}
			g_print("\n");
		}
#endif
	}
}

void ElementWatcher::OnSpectrum(const std::string& name, 
	const nlohmann::json::value_type& value, const GstStructure * s)
{
	GstClockTime endtime;
	const GValue *magnitudes;
	const GValue *phases;
	const GValue *mag, *phase;
	guint size;

	if (!gst_structure_get_clock_time(s, "endtime", &endtime))
		endtime = GST_CLOCK_TIME_NONE;

	magnitudes = gst_structure_get_value(s, "magnitude");
	phases = gst_structure_get_value(s, "phase");
	size = gst_value_list_get_size(magnitudes);
	json data;


	json::array_t mags;
	for (guint i = 0; i < size; ++i) {

		mag = gst_value_list_get_value(magnitudes, i);
		phase = gst_value_list_get_value(phases, i);
		mags.push_back(g_value_get_float(mag));
		//if (mag != NULL && phase != NULL) {
		//	g_print("****  band %d (freq %g): magnitude %f dB phase %f\n", i, freq,
		//		g_value_get_float(mag), g_value_get_float(phase));
		//}
	}
	data["name"] = name;
	data["endtime"] = endtime;
	data["magnitude"] = mags;

	json meta;
	meta["topic"] = "spectrum";
	meta["origin"] = this->uname();

	Notify(data, meta);

}
