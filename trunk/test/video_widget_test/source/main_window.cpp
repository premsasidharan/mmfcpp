#include <main_window.h>

Main_window::Main_window(int argc, char**argv)
	:timer(this)
	, data1(0)
	, data2(0)
	, video(this)
	, file1(argv[1], atoi(argv[2]), atoi(argv[3]), Media::I420)
	, file2(argv[4], atoi(argv[5]), atoi(argv[6]), Media::I420)
	, stereo_mode(7)
	, disp_mode(Video_widget::RGB)
{
	file1.open();
	file2.open();
	data1 = new uint8_t[file1.frame_size()];
	data2 = new uint8_t[file2.frame_size()];
	video.set_video_params(Media::I420, atoi(argv[2]), atoi(argv[3]), Media::I420, atoi(argv[5]), atoi(argv[6]));

	video.set_display_mode(disp_mode);
	video.set_stereo_mode(stereo_mode);	

	connect(&timer, SIGNAL(timeout()), this, SLOT(timed_out()));

	timer.setInterval(40);
	timer.start();
}

Main_window::~Main_window()
{
	delete [] data1; data1 = 0;
	delete [] data2; data2 = 0;
}

void Main_window::resizeEvent(QResizeEvent*)
{
	video.resize(width(), height());
}

void Main_window::timed_out()
{
	file1.read(data1, file1.frame_size());
	file2.read(data2, file2.frame_size());
	
	video.show_frame(data1, data2);
}

void Main_window::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
		case Qt::Key_Up:
			++stereo_mode;
			break;
		case Qt::Key_Down:
			--stereo_mode;
			break;
		case Qt::Key_Left:
			disp_mode = (Video_widget::Mode)((int)disp_mode+1);
			break;
		case Qt::Key_Right:
			disp_mode = (Video_widget::Mode)((int)disp_mode-1);
			break;
		default:
			return;
	}

	if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
	{ 
		if (stereo_mode >= 9)
		{
			stereo_mode = 0;
		}	
		else if (stereo_mode < 0)
		{
			stereo_mode = 9;
		}
		video.set_stereo_mode(stereo_mode);
	}

	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
	{ 
		if (disp_mode > Video_widget::GRID_NRGB)
		{
			disp_mode = Video_widget::Y;
		}	
		else if (disp_mode < Video_widget::Y)
		{
			disp_mode = Video_widget::GRID_NRGB;
		}
		video.set_display_mode(disp_mode);
	}
	qDebug() << "Stereo Mode :" << stereo_mode << ", Disp. Mode: " << disp_mode;
}

