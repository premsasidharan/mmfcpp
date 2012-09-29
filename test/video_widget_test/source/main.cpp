#include <QApplication>

#include <video_widget.h>

#include <main_window.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Main_window window(argc, argv);
	window.resize(640, 480);
	window.show();
	

	/*int width1 = atoi(argv[2]);
	int height1 = atoi(argv[3]);
	int width2 = atoi(argv[5]);
	int height2 = atoi(argv[6]);
	int data_size1 = (width1*height1*3)>>1;
	int data_size2 = (width2*height2*3)>>1;
	uint8_t* yuv_data1 = new uint8_t[data_size1];
	uint8_t* yuv_data2 = new uint8_t[data_size2];
	Read_yuv_file file1(argv[1], width1, height1, Media::I420);
	Read_yuv_file file2(argv[4], width2, height2, Media::I420);
	
	Video_widget window;
	file1.open();
	file2.open();
	//file1.seek(50, SEEK_SET);
	//file2.seek(3, SEEK_SET);
	file1.read(yuv_data1, data_size1);
	file2.read(yuv_data2, data_size2);
	window.set_video_params(Media::I420, width1, height1, Media::I420, width2, height2);
	window.set_stereo_mode(rand()%8);
	window.show_frame(yuv_data1, yuv_data2);
	window.show();*/
	
	return app.exec();

	/*file1.close();
	file2.close();
	delete [] yuv_data1;
	yuv_data1 = 0;
	delete [] yuv_data2;
	yuv_data2 = 0;*/
}


