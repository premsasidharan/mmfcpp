#include <player.h>

Player::Player(const QString path)
	:QObject()
	, window(0, 0, 320, 240)
	, av_src("av")
	, video_sink("xv", &window)
	, audio_sink("alsa", "default")
    , audio_decoder("audio_decoder")
    , video_decoder("video_decoder")
{
	if (1 == av_src.set_file_path(path.toAscii().constData()))
	{
		connect(&window, SIGNAL(renderer_close()), this, SLOT(on_renderer_close()));

        window.show();
		::connect(av_src, video_decoder);
		::connect(av_src, audio_decoder);
		::connect(audio_decoder, audio_sink);
		::connect(video_decoder, video_sink);	
	}
}

Player::~Player()
{
	::disconnect(audio_decoder, audio_sink);
	::disconnect(video_decoder, video_sink);
	::disconnect(av_src, audio_decoder);
	::disconnect(av_src, video_decoder);
}

int Player::start()
{
	return ::start(av_src, 0);
}

int Player::stop()
{
	int time;
	return ::stop(av_src, time);
}

void Player::on_renderer_close()
{
	int time;
	::stop(av_src, time);
}

