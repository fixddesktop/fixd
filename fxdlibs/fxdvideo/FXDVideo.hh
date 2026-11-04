#include <fx.h>
#include <mpv/client.h>

class FXDVideo : public FXWindow {
	FXDECLARE(FXDVideo)
protected:
	FXDVideo();
private:
	mpv_handle* mpv;
	FXGUISignal* sig;
	static void onMpvWakeup(void *ctx);
	void onMpvEvent(mpv_event*);
public:
	void loadVideo(const FXString& str);
	long onPaint(FXObject*, FXSelector, void*);
	long onGuiSignalMpv(FXObject*, FXSelector, void*);

public:
	enum {
		ID_MPV = FXMainWindow::ID_LAST,
		ID_LAST
	};
public:
	FXDVideo(FXComposite*, FXObject*, FXSelector, 
		FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0);

	virtual void create();
	virtual ~FXDVideo();
};
