

#ifndef _LIBWEBSTREAMER_WEBSTREAMER_H_
#define _LIBWEBSTREAMER_WEBSTREAMER_H_








bool webstreamer_initialize(void);
void webstreamer_terminate(void);

#define ERRORMSG(name,message)\
   "{\"name\": \"" name "\", \"message\": \"" message "\"}"



#endif