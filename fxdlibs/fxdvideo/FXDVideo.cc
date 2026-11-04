// This example can be built with: qmake && make

#include "FXDVideo.hh"


FXDEFMAP(FXDVideo) FXDVideoMap[] = {
	FXMAPFUNC(SEL_PAINT, 0, FXDVideo::onPaint),
	FXMAPFUNC(SEL_IO_READ, FXDVideo::ID_MPV, FXDVideo::onGuiSignalMpv),

};

FXIMPLEMENT(FXDVideo, FXWindow, FXDVideoMap, ARRAYNUMBER(FXDVideoMap))

void FXDVideo::onMpvWakeup(void *ctx) {
	FXDVideo* win = (FXDVideo*)ctx;
	if (win != NULL) win->sig->signal();
}

FXDVideo::FXDVideo() {
  flags|=FLAG_ENABLED|FLAG_SHOWN;
}

FXDVideo::FXDVideo(FXComposite* p, FXObject* tgt, FXSelector sel, 
		FXuint opts, FXint x, FXint y, FXint w, FXint h ):
	FXWindow(p, opts, x, y, w, h) {
	flags|=FLAG_ENABLED|FLAG_SHOWN;
	//a = app;

	sig = new FXGUISignal(getApp(), this, FXDVideo::ID_MPV);

	mpv = mpv_create();

	if (mpv == NULL) {
		fputs("Cannot create mpv instance!\n", stderr);
		exit(1);
	}

    mpv_set_option_string(mpv, "input-default-bindings", "no");

    mpv_set_option_string(mpv, "input-vo-keyboard", "no");
	mpv_set_option_string(mpv, "input-cursor", "no"); 
	

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(mpv, "info");

	mpv_set_wakeup_callback(mpv, onMpvWakeup, this);


	if (mpv_initialize(mpv) < 0) {
		fputs("Cannot initialize mpv!\n", stderr);
		exit(1);
	}
}

void FXDVideo::create() {
	FXWindow::create();

	FXlong raw_wid = (FXlong)(FXID)id();
#ifdef _WIN32
    // Truncate to 32-bit, as all Windows handles are. This also ensures
    // it doesn't go negative.
    FXulong wid = (FXuint)(FXuval)(raw_wid);
#else
    FXulong wid = raw_wid;
#endif


    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
}


void FXDVideo::onMpvEvent(mpv_event *event) {
	//if (!id()) return;
	
	static int i = 0;
	//printf("event: %d\n", ++i);

	FXString str;
	//char str[256];
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                FXlong time = (FXint)(*(double*)prop->data*1000.0);
				printf("time: %ld\n", time);
                //std::stringstream ss;
                //ss << "At: " << time;
				//str.format("At: %lf", time);
                //statusbar->getStatusLine()->setText(str);
				//statusbar->getStatusLine()->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&str);
            } else if (prop->format == MPV_FORMAT_NONE) {
                // The property is unavailable, which probably means playback
                // was stopped.
                //statusbar->getStatusLine()->setText(FXString::null);
            }
        } else if (strcmp(prop->name, "chapter-list") == 0 ||
                   strcmp(prop->name, "track-list") == 0)
        {
			(void)0;
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        // Retrieve the new video size.
        FXlong w, h;
        if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
            // imply a resize, and you should check yourself if the video
            // dimensions really changed.
            // mpv itself will scale/letter box the video to the container size
            // if the video doesn't fit.
            //std::stringstream ss;
            //ss << "Reconfig: " << w << " " << h;
			//str.format("Reconfig: %lldx%lld", w, h);
            //statusbar->getStatusLine()->setText(str);
			//statusbar->getStatusLine()->handle(this,FXSEL(SEL_COMMAND,ID_SETSTRINGVALUE),(void*)&str);
		}
        break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
        struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
        /*std::stringstream ss;
        ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
        append_log(QString::fromStdString(ss.str()));*/
		fprintf(stderr, "[ %s ] %s: %s\n", msg->prefix, msg->level, msg->text);
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

long FXDVideo::onGuiSignalMpv(FXObject* obj, FXSelector, void* ptr)
{
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
		//printf("event id: %d\n", event->event_id);
        onMpvEvent(event);
    }

	return 1;
}

long FXDVideo::onPaint(FXObject* obj, FXSelector, void* ptr) {
	FXDCWindow dc(this,(FXEvent*)ptr);
	dc.setForeground(FXRGB(0,0,0));
	dc.fillRectangle(0,0,width,height);
	return 1;
}


void FXDVideo::loadVideo(const FXString& str) {
    if (mpv) {
        const char *args[] = {"loadfile", str.text(), NULL};
        mpv_command_async(mpv, 0, args);
    }
}


FXDVideo::~FXDVideo()
{
	if (mpv) mpv_terminate_destroy(mpv);
}

