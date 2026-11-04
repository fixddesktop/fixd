// This example can be built with: qmake && make

#include <fx.h>
#include "FXDVideo.hh"

//#include "qtexample.h"

FXApp* a;

static void wakeup(void *ctx);

class VideoWindow : public FXMainWindow {
	FXDECLARE(VideoWindow)
	friend void wakeup(void *ctx);
protected:
	VideoWindow() {}
private:
	FXMenuBar* menubar;
	FXMenuPane* mn_file;
	FXCanvas* canvas;
	FXStatusBar* statusbar;
	FXDVideo* video;
public:
	long onCmdNew(FXObject*, FXSelector, void*);
	long onCmdOpen(FXObject*, FXSelector, void*);
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
};

FXIMPLEMENT(VideoWindow, FXMainWindow, VideoWindowMap, ARRAYNUMBER(VideoWindowMap))

VideoWindow::VideoWindow(FXApp* app) : FXMainWindow(app, "MPV in FOX Demo",
		NULL, NULL, DECOR_ALL, 0,0,600,400) {
	//a = app;

	statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
	menubar = new FXMenuBar(this, LAYOUT_FILL_X|LAYOUT_SIDE_TOP);

	mn_file = new FXMenuPane(this);
	new FXMenuTitle(menubar, "&File", NULL, mn_file);

	new FXMenuCommand(mn_file, "&New\tCtl+N", NULL, this, ID_NEW);
	new FXMenuCommand(mn_file, "&Open...\tCtl+O", NULL, this, ID_OPEN);
	new FXMenuSeparator(mn_file);
	new FXMenuCommand(mn_file, "Quit\tCtl+Q", NULL, getApp(), FXApp::ID_QUIT);

	FXPacker* videofrm = new FXPacker(this, LAYOUT_FILL|FRAME_NORMAL, 0,0,0,0, 0,0,0,0);
	video = new FXDVideo(videofrm, this, ID_MPV, LAYOUT_FILL);
}

void VideoWindow::create() {

	show();
	FXMainWindow::create();
}





long VideoWindow::onCmdOpen(FXObject* obj, FXSelector, void* ptr)
{
	puts("hi");
	FXString filestr = FXFileDialog::getOpenFilename(this,"Open Video","","*");
	if (filestr.empty()) return 0;
	
    video->loadVideo(filestr.text());
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
