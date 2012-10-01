#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QTimer>
#include <QWidget>
#include <QMouseEvent>

#include <video_widget.h>
#include <read_yuv_file.h>

class Main_window:public QWidget
{
	Q_OBJECT
public:
	Main_window(int argc, char**argv);
	~Main_window();

protected:
	void resizeEvent(QResizeEvent*);
	void keyPressEvent(QKeyEvent* event);

protected slots:
	void timed_out();

private:
	QTimer timer;
	uint8_t* data1;
	uint8_t* data2;
	Video_widget video;
	Read_yuv_file file1;
	Read_yuv_file file2;

	int format[2];
	int vwidth[2];
	int vheight[2];

	int stereo_mode;
	Video_widget::Mode disp_mode;
};

#endif


