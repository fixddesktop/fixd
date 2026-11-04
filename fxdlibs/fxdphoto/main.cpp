#include <fx.h>
#include <FXPNGIcon.h>
#include "icons.h"
#include <filesystem>
#include <vector>
#include <algorithm>

FXImage* loadImage(FXApp* app, const FXString& path) {
    FXIconSource iconsrc(app);
    return iconsrc.loadImageFile(path);
}

class PhotoWin : public FXMainWindow {
	FXDECLARE(PhotoWin)
protected:
	PhotoWin() {}
private:
	FXButton* btn;
	FXImageView* imageView;
	FXIcon* openIcon;
	FXIcon* leftIcon;
	FXIcon* rightIcon;
	std::vector<std::filesystem::path> files;
    int current = 0;
    FXImage* original = nullptr;
    int lastW = 0, lastH = 0;
    void fitImage();

public:
	long onCmdHello(FXObject*, FXSelector, void*);
	long onCmdArrow(FXObject*, FXSelector, void*);
	void layout() override;
public:
	// message ids
	enum {
		ID_HELLO = FXMainWindow::ID_LAST,
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
	FXMAPFUNC(SEL_COMMAND,           PhotoWin::ID_HELLO,  PhotoWin::onCmdHello),
	FXMAPFUNC(SEL_COMMAND, PhotoWin::ID_LEFT, PhotoWin::onCmdArrow),
    FXMAPFUNC(SEL_COMMAND, PhotoWin::ID_RIGHT, PhotoWin::onCmdArrow),
};

FXIMPLEMENT(PhotoWin, FXMainWindow, PhotoWinMap, ARRAYNUMBER(PhotoWinMap))

PhotoWin::PhotoWin(FXApp *a) : FXMainWindow(a, "Photo Viewer", NULL, NULL,
		DECOR_ALL, 0,0,640,480) {

    FXHorizontalFrame* toolbar = new FXHorizontalFrame(this,
    LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);

    openIcon = new FXPNGIcon(a, open_png);
    leftIcon = new FXPNGIcon(a, left_png);
    rightIcon = new FXPNGIcon(a, right_png);

	btn = new FXButton(toolbar, "", openIcon, this, ID_HELLO, //open
            BUTTON_TOOLBAR);

    new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE | LAYOUT_FILL_Y); //seperator

    new FXButton(toolbar, "", leftIcon, this, ID_LEFT, BUTTON_TOOLBAR); //left arrow

    new FXButton(toolbar, "", rightIcon, this, ID_RIGHT, BUTTON_TOOLBAR); //right arrow

    new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE | LAYOUT_FILL_Y); //seperator

    FXVerticalFrame* frame = new FXVerticalFrame(this,
        FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    imageView = new FXImageView(frame, nullptr, nullptr, 0,
        LAYOUT_FILL_X | LAYOUT_FILL_Y);

    imageView->setBackColor(getApp()->getBaseColor());

}

PhotoWin::~PhotoWin() {
    delete openIcon;
    delete leftIcon;
    delete rightIcon;
    delete original;
    delete imageView->getImage();
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

    delete original;
    original = img;
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

    delete original;
    original = img;
    fitImage();

    setTitle(FXPath::name(path) + " - Photo Viewer");

    return 1;
}

void PhotoWin::fitImage() {
    if (!original) return;

    int w = original->getWidth();
    int h = original->getHeight();

    double scale = std::min({1.0,
        double(std::max(1, imageView->getWidth() - 4)) / w,
        double(std::max(1, imageView->getHeight() - 4)) / h});

    FXImage* img = new FXImage(getApp(), nullptr, IMAGE_OWNED, w, h);
    std::copy(original->getData(), original->getData() + w * h, img->getData());

    if (scale < 1.0)
        img->scale(std::max(1, int(w * scale)),
                   std::max(1, int(h * scale)), 1);

    img->create();

    FXImage* old = imageView->getImage();
    imageView->setImage(img);
    delete old;
}

void PhotoWin::layout() {
    FXMainWindow::layout();

    int w = imageView->getWidth();
    int h = imageView->getHeight();

    if (w == lastW && h == lastH) return;

    lastW = w;
    lastH = h;
    fitImage();
}

int main(int argc, char *argv[]) {
	FXApp application("Hello", "FOXTest");

	application.init(argc, argv);

	new PhotoWin(&application);

	application.create();

	return application.run();
}
