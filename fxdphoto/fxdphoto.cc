#include <fx.h>
#include "res/foxres.h"
#include <filesystem>
#include <vector>
#include <algorithm>

FXIcon* ico_open;
FXIcon* ico_left;
FXIcon* ico_right;


FXImage* loadImage(FXApp* app, const FXString& path) {
	FXIconSource iconsrc(app);
	return iconsrc.loadImageFile(path);
}

class PhotoWin : public FXMainWindow {
	FXDECLARE(PhotoWin)
protected:
	PhotoWin() {}
private:
	FXToolBar* tb_main;
	FXPacker* frm_imgvw;
	FXImageView* ivw_imgvw;
	std::vector<std::filesystem::path> files;
	int current = 0;
	FXImage* img_orig = NULL;
	int lastW = 0, lastH = 0;
	void fitImage();

public:
	long onCmdHello(FXObject*, FXSelector, void*);
	long onCmdArrow(FXObject*, FXSelector, void*);
	void layout() override;
public:
	// message ids
	enum {
		ID_OPEN = FXMainWindow::ID_LAST,
		ID_LEFT,
		ID_RIGHT,
		ID_LAST
	};

public:
	PhotoWin(FXApp* a);

	virtual void create();

	virtual ~PhotoWin();
};

// message map
FXDEFMAP(PhotoWin) PhotoWinMap[] = {
	FXMAPFUNC(SEL_COMMAND, PhotoWin::ID_OPEN, PhotoWin::onCmdHello),
	FXMAPFUNC(SEL_COMMAND, PhotoWin::ID_LEFT,  PhotoWin::onCmdArrow),
	FXMAPFUNC(SEL_COMMAND, PhotoWin::ID_RIGHT, PhotoWin::onCmdArrow),
};

FXIMPLEMENT(PhotoWin, FXMainWindow, PhotoWinMap, ARRAYNUMBER(PhotoWinMap))

PhotoWin::PhotoWin(FXApp *a) : FXMainWindow(a, "Photo Viewer", NULL, NULL,
		DECOR_ALL, 0,0,640,480) {

	tb_main = new FXToolBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);

	new FXButton(tb_main, "", ico_open, this, ID_OPEN, FRAME_RAISED|BUTTON_TOOLBAR);

	new FXSeparator(tb_main, SEPARATOR_GROOVE); //seperator
	new FXButton(tb_main, "", ico_left, this, ID_LEFT, FRAME_RAISED|BUTTON_TOOLBAR); //left arrow
	new FXButton(tb_main, "", ico_right, this, ID_RIGHT, FRAME_RAISED|BUTTON_TOOLBAR); //right arrow
	//new FXSeparator(tb_main, SEPARATOR_GROOVE); //seperator

	frm_imgvw = new FXPacker(this, FRAME_NORMAL|LAYOUT_FILL);
	frm_imgvw->setBackColor(getApp()->getBackColor());

	ivw_imgvw = new FXImageView(frm_imgvw, NULL, NULL, 0, LAYOUT_FILL);

	//ivw_imgvw->setBackColor(getApp()->getBaseColor());
}

PhotoWin::~PhotoWin() {
	delete img_orig;
	delete ivw_imgvw->getImage();
}

void PhotoWin::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

// button handler
long PhotoWin::onCmdHello(FXObject*, FXSelector, void*) {
	FXString filestr = FXFileDialog::getOpenFilename(this,"File Open","","*");
	if (filestr.empty()) return 0;

	FXImage* img = loadImage(getApp(), filestr);

	if (!img) {
		FXMessageBox::error(this, MBOX_OK, "Error", "Invalid or unsupported image.");
		return 1;
	}

	delete img_orig;
	img_orig = img;
	fitImage();

	FXString filename = FXPath::name(filestr);
	setTitle(filename + " - Photo Viewer");

	files.clear();

	namespace fs = std::filesystem;

	for (auto& f : fs::directory_iterator(fs::path(filestr.text()).parent_path())) { //get more images for arrow
		auto ext = f.path().extension().string();

		if (f.is_regular_file() &&
				(ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
				 ext == ".bmp" || ext == ".gif")) //This could be expanded at one point. i think.
			files.push_back(f.path());
	}

	std::sort(files.begin(), files.end());

	auto it = std::find(files.begin(), files.end(), fs::path(filestr.text()));
	if (it != files.end())
		current = int(it - files.begin());

	return 1;
}

long PhotoWin::onCmdArrow(FXObject*, FXSelector sel, void*) {
	if (files.empty()) return 1;

	int step = FXSELID(sel) == ID_RIGHT ? 1 : -1; //right is +1 left is -1
	current = (current + step + files.size()) % files.size(); //wrap to other end

	FXString path(files[current].string().c_str());

	FXImage* img = loadImage(getApp(), path);
	if (!img) return 1;

	delete img_orig;
	img_orig = img;
	fitImage();

	setTitle(FXPath::name(path) + " - Photo Viewer");

	return 1;
}

void PhotoWin::fitImage() {
	if (!img_orig) return;

	int w = img_orig->getWidth();
	int h = img_orig->getHeight();

	double scale = std::min({1.0,
			double(std::max(1, ivw_imgvw->getWidth() - 4)) / w,
			double(std::max(1, ivw_imgvw->getHeight() - 4)) / h});

	FXImage* img = new FXImage(getApp(), NULL, IMAGE_OWNED, w, h);
	//std::copy(img_orig->getData(), img_orig->getData() + w * h, img->getData());
	memcpy(img->getData(), img_orig->getData(), w * h*sizeof(FXColor));

	if (scale < 1.0)
		img->scale(std::max(1, int(w * scale)),
				std::max(1, int(h * scale)), 1);

	img->create();

	FXImage* old = ivw_imgvw->getImage();
	ivw_imgvw->setImage(img);
	delete old;
}

void PhotoWin::layout() {
	FXMainWindow::layout();

	int w = ivw_imgvw->getWidth();
	int h = ivw_imgvw->getHeight();

	if (w == lastW && h == lastH) return;

	lastW = w;
	lastH = h;
	fitImage();
}

int main(int argc, char *argv[]) {
	FXApp application("FXDPhoto", "FXDesk");
	application.init(argc, argv);

	ico_open = new FXGIFIcon(&application, res_ico_open);
	ico_left = new FXGIFIcon(&application, res_ico_left);
	ico_right = new FXGIFIcon(&application, res_ico_right);

	new PhotoWin(&application);

	application.create();
	return application.run();
}
