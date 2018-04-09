
#ifndef _LIBWEBSTREAMER_ELEMENT_WATCHER_H_
#define _LIBWEBSTREAMER_ELEMENT_WATCHER_H_

#include <nlohmann/json.hpp>
#include <framework/app.h>
class ElementWatcher : public IApp
{
public:
	APP(ElementWatcher)


		ElementWatcher(const std::string& name, WebStreamer* ws)
		: IApp(name, ws)
		, pipeline_(NULL)
	{

	}

	void On(Promise* promise);
	bool Initialize(Promise* promise);
	void Destroy(Promise* promise);

	virtual void OnMessage(GstBus * bus, GstMessage * message);
protected:
	void Startup(Promise* promise);
	void Stop(Promise* promise);
	void Spectrum(const nlohmann::json::value_type& j);

	void OnSpectrum(const std::string& name, const nlohmann::json::value_type& value, const GstStructure * message);
private:

	GstElement * pipeline_;

	nlohmann::json watch_list_;
};


#endif //!_LIBWEBSTREAMER_RTSP_TEST_SERVER_H_