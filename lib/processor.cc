#include "processor.h"


bool IProcessor::Initialize(Promise* promise)
{
	if (!pipeline_)
	{
		pipeline_ = gst_pipeline_new((const gchar*)uname().c_str());
	}

	if (pipeline_) {
		promise->resolve();
		return true;
	}
	else {
		promise->reject("create pipeline failed.");
		return false;
	}
}


void IProcessor::Destroy(Promise* promise)
{

}