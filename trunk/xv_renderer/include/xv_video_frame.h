#ifndef _XV_VIDEO_FRAME_H
#define _XV_VIDEO_FRAME_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

class X11_window;

class Xv_video_frame
{
public:
	friend class X11_window;
	Xv_video_frame(int _width, int _height, int fmt, XvPortID _port, unsigned char* _yuv_data);
	~Xv_video_frame();

public:
	int image_width() const { return width; };
	int image_height() const { return height; };

	static unsigned int video_fmt(const char* fmt);

private:
	int width;
	int height;
	int format;
	XvPortID port;
	XvImage* image;
    unsigned char *yuv_data;
};

#endif

