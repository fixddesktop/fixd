// This example can be built with: qmake && make

#include <fx.h>
#include <mpv/client.h>

//#include "qtexample.h"

FXApp* a;

static void wakeup(void *ctx);

class VideoWindow : public FXMainWindow {
	FXDECLARE(VideoWindow)
	friend void wakeup(void *ctx);
protected:
	VideoWindow() {}
private:
	mpv_handle* mpv;
	FXMenuBar* menubar;
	FXMenuPane* mn_file;
	FXCanvas* canvas;
	FXStatusBar* statusbar;
	FXGUISignal* sig;
public:
	long onCmdNew(FXObject*, FXSelector, void*);
	long onCmdOpen(FXObject*, FXSelector, void*);
	long onChoreMpv(FXObject*, FXSelector, void*);


	void handle_mpv_event(mpv_event*);
private:
	void runMpvSignal() { sig->signal(); }

public:
	enum {
		ID_NEW = FXMainWindow::ID_LAST,
		ID_OPEN,
		ID_MPV,
		ID_LAST
	};
public:
	VideoWindow(FXApp* a);

	virtual void create();
	virtual ~VideoWindow();
};

FXDEFMAP(VideoWindow) VideoWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND, VideoWindow::ID_NEW, VideoWindow::onCmdNew),
	FXMAPFUNC(SEL_COMMAND, VideoWindow::ID_OPEN, VideoWindow::onCmdOpen),

	FXMAPFUNC(SEL_IO_READ, VideoWindow::ID_MPV, VideoWindow::onChoreMpv),

};

FXIMPLEMENT(VideoWindow, FXMainWindow, VideoWindowMap, ARRAYNUMBER(VideoWindowMap))

static void wakeup(void *ctx)
{
	puts("hi");
	// This callback is invoked from any mpv thread (but possibly also
	// recursively from a thread that is calling the mpv API). Just notify
	// the Qt GUI thread to wake up (so that it can process events with
	// mpv_wait_event()), and return as quickly as possible.
	VideoWindow* win = (VideoWindow*)ctx;
	if (win != NULL) {
		win->runMpvSignal();
	}
	//
	//FXGUISignal* sig = new FXGUISignal(a, win, VideoWindow::ID_MPV);
	//sig->signal();
}

VideoWindow::VideoWindow(FXApp* app) : FXMainWindow(app, "MPV in FOX Demo",
		NULL, NULL, DECOR_ALL, 0,0,600,400) {
	//a = app;

	sig = new FXGUISignal(a, this, VideoWindow::ID_MPV);
	statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER|FRAME_RAISED);
	menubar = new FXMenuBar(this, LAYOUT_FILL_X|FRAME_RAISED|LAYOUT_SIDE_TOP);

	mn_file = new FXMenuPane(this);
	new FXMenuTitle(menubar, "&File", NULL, mn_file);

	new FXMenuCommand(mn_file, "&New\tCtl+N", NULL, this, ID_NEW);
	new FXMenuCommand(mn_file, "&Open...\tCtl+O", NULL, this, ID_OPEN);
	new FXMenuSeparator(mn_file);
	new FXMenuCommand(mn_file, "Quit\tCtl+Q", NULL, getApp(), FXApp::ID_QUIT);

	mpv = mpv_create();

	if (mpv == NULL) {
		fputs("Cannot create mpv instance!\n", stderr);
		exit(1);
	}

	canvas = new FXCanvas(this, NULL, 0, LAYOUT_FILL);

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpv, "input-default-bindings", "no");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpv, "input-vo-keyboard", "no");
	mpv_set_option_string(mpv, "input-cursor", "no"); 
	

    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(mpv, "info");

	mpv_set_wakeup_callback(mpv, wakeup, this);


	if (mpv_initialize(mpv) < 0) {
		fputs("Cannot initialize mpv!\n", stderr);
		exit(1);
	}
}

void VideoWindow::create() {

	show();
	FXMainWindow::create();

	FXlong raw_wid = (FXlong)(FXID)canvas->id();
#ifdef _WIN32
    // Truncate to 32-bit, as all Windows handles are. This also ensures
    // it doesn't go negative.
    FXulong wid = (FXuint)(FXuval)(raw_wid);
#else
    FXulong wid = raw_wid;
#endif


    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
}


void VideoWindow::handle_mpv_event(mpv_event *event)
{
	//if (!id()) return;
	
	static int i = 0;
	printf("event: %d\n", ++i);

	FXString str;
	//char str[256];
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                //std::stringstream ss;
                //ss << "At: " << time;
				str.format("At: %lf", time);
                statusbar->getStatusLine()->setText(str);
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
			str.format("Reconfig: %lldx%lld", w, h);
            statusbar->getStatusLine()->setText(str);
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

// This slot is invoked by wakeup() (through the mpv_events signal).
long VideoWindow::onChoreMpv(FXObject* obj, FXSelector, void* ptr)
{
	//return 1;
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
		printf("event id: %d\n", event->event_id);
        handle_mpv_event(event);
    }

	//a->addChore(this, VideoWindow::ID_MPV, NULL);

	return 1;
}

long VideoWindow::onCmdOpen(FXObject* obj, FXSelector, void* ptr)
{
	puts("hi");
	FXString filestr = FXFileDialog::getOpenFilename(this,"Open Video","","*");
	if (filestr.empty()) return 0;
	
    if (mpv) {
        const char *args[] = {"loadfile", filestr.text(), NULL};
        mpv_command_async(mpv, 0, args);
    }
	return 1;
}

long VideoWindow::onCmdNew(FXObject* obj, FXSelector, void* ptr)
{
	VideoWindow* vidwin = new VideoWindow(a);
	vidwin->create();
    //(new MainWindow())->show();
	return 1;
}


VideoWindow::~VideoWindow()
{
	delete mn_file;
	if (mpv)
        mpv_terminate_destroy(mpv);
}

int main(int argc, char *argv[])
{
	FXApp application("VideoWindow", "VideoWindow");
	application.init(argc, argv);

	a = &application;

	new VideoWindow(a);
	application.create();

	return application.run();
}
