#ifndef _PCM_WIDGET_H_
#define _PCM_WIDGET_H_

#include <QtGui>
#include <track_widget.h>

class Pcm_widget:public QWidget
{
    Q_OBJECT
public:
    Pcm_widget(QWidget* parent = 0);
    ~Pcm_widget();

public:
    void set_channels(int _channels);
    int get_channels() const { return channels; };
    void set_track_data(int channel, int32_t* data, int size, int max_value);

protected:
     void create_child_widgets();
     void delete_child_widgets();
     void resizeEvent(QResizeEvent* event);

signals:
     void channel_change(int channels);

protected slots:
     void set_channels_handler(int _channels);

private:
    int channels;
    Track_widget** track_widget;
};

#endif
