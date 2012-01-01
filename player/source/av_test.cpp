#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

#include <xv_renderer.h>
#include <av_file_src.h>
#include <audio_renderer.h>
#include <avcodec_audio_decoder.h>
#include <avcodec_video_decoder.h>

int main(int argc, char** argv)
{
    MEDIA_TRACE();
    
    X11_window window(0, 0, 320, 240);	
    
	int time = 0;
	Av_file_src src("av");
	Xv_renderer video_sink("xv", &window);
	Audio_renderer audio_sink("alsa", "default");
    Avcodec_audio_decoder audio_decoder("audio_decoder");
    Avcodec_video_decoder video_decoder("video_decoder");

	if (argc < 2)
	{
		printf("\n\tInsufficien arguments\n");
		exit(0);
	}

	if (1 == src.set_file_path(argv[1]))
	{
        window.show();
		connect(src, video_decoder);
		connect(src, audio_decoder);
		connect(audio_decoder, audio_sink);
		connect(video_decoder, video_sink);
		start(src, 0);
		X11_app::exec();
		stop(src, time);
		disconnect(audio_decoder, audio_sink);
		disconnect(video_decoder, video_sink);
		disconnect(src, audio_decoder);
		disconnect(src, video_decoder);
	}
	else
	{
		printf("\nInvalid File Path");
	}
	return 0;
}

