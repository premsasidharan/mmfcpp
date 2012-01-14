#ifndef _PLAYER_H
#define _PLAYER_H

#include <yuv_window.h>
#include <xv_renderer.h>
#include <av_file_src.h>
#include <audio_renderer.h>
#include <avcodec_audio_decoder.h>
#include <avcodec_video_decoder.h>

#include <QObject>

class Player:public QObject
{
	Q_OBJECT
public:
	Player(const QString path);
	~Player();

public:
	int start();
	int stop();

private:
	Yuv_window window;
	Av_file_src av_src;
	Xv_renderer video_sink;
	Audio_renderer audio_sink;
    Avcodec_audio_decoder audio_decoder;
    Avcodec_video_decoder video_decoder;
};

#endif
 
