#include <pcm_widget.h>

Pcm_widget::Pcm_widget(QWidget* parent)
    :QWidget(parent)
    , channels(0)
    , track_widget(0)
{
    connect(this, SIGNAL(channel_change(int)), this, SLOT(set_channels_handler(int)));
    create_child_widgets();
}

Pcm_widget::~Pcm_widget()
{
    delete_child_widgets();
}

void Pcm_widget::set_channels(int _channels)
{
    if (channels != _channels)
    {
        delete_child_widgets();
        channels = _channels;
        create_child_widgets();
    }
}

void Pcm_widget::resizeEvent(QResizeEvent* event)
{
    delete_child_widgets();
    create_child_widgets();
}

void Pcm_widget::delete_child_widgets()
{
    for (int i = 0; i < channels; i++)
    {
        if (0 != track_widget[i])
        {
            track_widget[i]->hide();
            delete track_widget[i];
            track_widget[i] = 0;
        }
    }
    delete [] track_widget;
    track_widget = 0;
}

void Pcm_widget::create_child_widgets()
{
    if (channels > 0)
    {
        track_widget = new Track_widget*[channels];
        for (int i = 0; i < channels; i++)
        {
            track_widget[i] = new Track_widget(this);
            track_widget[i]->move(0, i*(height()/channels));
            track_widget[i]->resize(width(), height()/channels);
            track_widget[i]->show();
        }
    }
}

void Pcm_widget::set_track_data(int channel, int32_t* data, int size, int max_value)
{
    if (channel < channels)
    {
        track_widget[channel]->set_track_data(data, size, max_value);
    }
}

void Pcm_widget::set_channels_handler(int _channels)
{
    qDebug() << channels << ", " << width() << ", " << height() << ", " << _channels;
}
