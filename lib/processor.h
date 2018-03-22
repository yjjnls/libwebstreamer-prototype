
#ifndef _LIBWEBSTREAMER_PROCESSOR_H_
#define _LIBWEBSTREAMER_PROCESSOR_H_


class IProcessor
{
	virtual bool Initialize();
	virtual void Destroy();
};

#endif//_LIBWEBSTREAMER_PROCESSOR_H_