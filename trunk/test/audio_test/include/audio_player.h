#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_

#include <wave_file_src.h>
#include <audio_renderer.h>

class Audio_player:public Observer
{
public:
    Audio_player();
    ~Audio_player();
    
public:
    int wait();
    int stop(int& time);
    int start(int time);
    
    int set_file_path(const char* path);
    
    int event_handler(Media::events event, Abstract_media_object* obj);
    
private:
    Wave_file_src src;
    Audio_renderer sink;
    Condition_variable cv;
};

#endif
 
