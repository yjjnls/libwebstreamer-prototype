#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-session-pool.h>
#include <stdio.h>










static gboolean timeout (GstRTSPServer * server)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);

  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}

/* Process keyboard input */
static gboolean handle_keyboard(GIOChannel *source, GIOCondition cond, GMainLoop *loop) {
	gchar *str = NULL;

	if (g_io_channel_read_line(source, &str, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (str[0] == 'q')
			g_main_loop_quit(loop);
	}
	g_free(str);
	return TRUE;
}

int main (int argc, char *argv[])
{
  GMainLoop *loop = NULL;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;
  GstRTSPSessionPool *session;
    
  gst_init (&argc, &argv);

  GMainContext *context;

  //context = g_main_context_new();
  //add_source(context);
  /* Add a keyboard watch so we get notified of keystrokes */

  loop = g_main_loop_new (NULL, FALSE);

  GIOChannel *io_stdin;
#ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd(fileno(stdin));
#else
  io_stdin = g_io_channel_unix_new(fileno(stdin));
#endif
  g_io_add_watch(io_stdin, G_IO_IN, (GIOFunc)handle_keyboard, loop);


  session = gst_rtsp_session_pool_new();
  gst_rtsp_session_pool_set_max_sessions  (session, 255);
  
  server = gst_rtsp_server_new ();

  
  mounts = gst_rtsp_server_get_mount_points (server);

  
  factory = gst_rtsp_media_factory_new ();

  gst_rtsp_media_factory_set_launch(factory, argv[1]);
      //"( videotestsrc is-live=1 ! x264enc ! rtph264pay name=pay0 pt=96 )");
 
  
  gst_rtsp_media_factory_set_shared (factory, TRUE);

  
  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);
  
  g_object_unref (mounts);

  gst_rtsp_server_attach (server, NULL); 
    
  //g_timeout_add_seconds (20, (GSourceFunc) timeout, server);

  g_print ("stream ready at rtsp://127.0.0.1:8554/test\n");
  g_main_loop_run (loop);
  g_io_channel_unref(io_stdin);
  return 0;
}