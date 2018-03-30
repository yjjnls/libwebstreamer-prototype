
#ifndef _LIBWEBSTREAMER_MULTIFILE_REPORT_H_
#define _LIBWEBSTREAMER_MULTIFILE_REPORT_H_


#include <framework/app.h>
class AVAnalyzer : public IApp
{
public:
	APP(AVAnalyzer)


		AVAnalyzer(const std::string& name, WebStreamer* ws)
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

private:

	GstElement * pipeline_;


};


#endif //!_LIBWEBSTREAMER_RTSP_TEST_SERVER_H_