 
#include <wave_hdr.h>
#include <tone_thread.h>

#include <QFile>
#include <QDebug>

#include <complex.h>

Tone_thread::Tone_thread(QObject* parent)
    :QThread(parent)
    , bps(0)
    , channels(0)
    , duration(0)
    , frequency(0)
    , sample_rate(0)
    , stop_flag(false)
    , progress(0)
{

}

Tone_thread::~Tone_thread()
{

}

void Tone_thread::set_parameters(const QString& path, int srate, int _bps, int _channels, int _f, int t)
{
    bps = _bps;
    channels = _channels;
    duration = t;
    frequency = _f;
    sample_rate = srate;

    file_path = path;
}

void Tone_thread::run()
{
    stop_flag = false;
    QFile file(this);

    file.setFileName(file_path);
    if (file.open(QIODevice::WriteOnly))
    {
        Wave_hdr header;
        memcpy(header.riff_id, "RIFF", 4);
        memcpy(header.wave.format, "WAVE", 4);
        memcpy(header.wave.fmt_id, "fmt ", 4);
        memcpy(header.wave.data_id, "data", 4);

        header.wave.data_size = duration*sample_rate*channels*bps;
        header.wave.size = sizeof(Wave_params);
        header.wave.params.audio_format = 1;
        header.wave.params.num_channels = channels;
        header.wave.params.sample_rate = sample_rate;
        header.wave.params.byte_rate = sample_rate*channels*bps;
        header.wave.params.block_align = channels*bps;
        header.wave.params.bits_per_sample = 8*bps;
        header.size = header.wave.data_size+sizeof(Wave_data);

        file.write((char*)&header, sizeof(Wave_hdr));

        double max_value;
        switch (bps)
        {
            case 1:
                max_value = (double)0X007E;
                break;
            case 2:
                max_value = (double)0x7FFF;
                break;
            case 3:
                max_value = (double)0x7FFFFF;
                break;
            default:
                max_value = (double)0x7FFFFFFF;
        }
        max_value *= 0.25;

        progress = 0;
        int samples = duration*sample_rate;
        signed char pcm_buff[bps*channels];
        complex seed = complex::polar(1.0, (2*M_PI*(double)frequency)/((double)sample_rate));
        complex current = seed;

        while (samples > 0 && (false == stop_flag))
        {
            int k = 0;
            int data;
            data = ((bps == 1)?0x80:0)+(int)(max_value*current.img_part());
            for (int i = 0; i < channels; i++)
            {
                for (int j = 0; j < bps; j++)
                {
                    pcm_buff[k] = ((data>>(8*j))&0xFF);
                    ++k;
                }
            }
            current *= seed;
            file.write((char*)pcm_buff, bps*channels);
            --samples;
            progress.fetchAndAddAcquire(1);
        }
        file.close();
    }
}

void Tone_thread::stop_thread()
{
    stop_flag = true;
}
