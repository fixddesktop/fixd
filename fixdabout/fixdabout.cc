#include <fx.h>
#include "res/foxres.h"

FXImage* img_about;

class AboutWindow : public FXMainWindow {
	FXDECLARE(AboutWindow)
protected:
	AboutWindow() {}
private:
	FXTabBook* tabbook;
	FXPacker* general_cont;
	FXPacker* txt_frm;
public:
	enum {
		ID_HELLO = FXMainWindow::ID_LAST,
		ID_LAST
	};
public:
	AboutWindow(FXApp* a);

	virtual void create();
	virtual ~AboutWindow();
};

FXIMPLEMENT(AboutWindow, FXMainWindow, NULL, 0)

AboutWindow::AboutWindow(FXApp *a) : FXMainWindow(a, "About FiXD", NULL, NULL, DECOR_TITLE|DECOR_CLOSE|DECOR_BORDER|DECOR_STRETCHABLE, 0,0,0,0, 8,8,8,8, 8,8) {
	new FXButton(this, "&Close", NULL, getApp(), FXApp::ID_QUIT, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_RIGHT|LAYOUT_SIDE_BOTTOM, 0,0,0,0, 16,16,2,2);
	new FXImageFrame(this, img_about, FRAME_NORMAL|LAYOUT_CENTER_X);

	new FXLabel(this, "The FiXD Desktop Environment", NULL, LAYOUT_CENTER_X, 0,0,0,0, 2,2,2,0);

	tabbook = new FXTabBook(this, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 0,0,0,0);

	new FXTabItem(tabbook, "General");

	general_cont = new FXPacker(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK);
	txt_frm = new FXPacker(general_cont, LAYOUT_FILL|FRAME_NORMAL, 0,0,0,0, 0,0,0,0);

	FXText* txt = new FXText(txt_frm, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL);
	txt->setText("FiXD is a Desktop Environment using the fOX Toolkit.");

}

AboutWindow::~AboutWindow() {
}

void AboutWindow::create() {
	FXMainWindow::create();
	show(PLACEMENT_SCREEN);
}


int main(int argc, char *argv[]) {
	FXApp application("About", "FiXDDesktop");
	application.init(argc, argv);

	img_about = new FXGIFImage(&application, res_img_about);

	new AboutWindow(&application);
	application.create();
	return application.run();
}
