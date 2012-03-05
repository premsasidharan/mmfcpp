#include <track_widget.h>

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QPainter>

Track_widget::Track_widget(QWidget* parent)
    :QWidget(parent)
{
}

Track_widget::~Track_widget()
{
}

void Track_widget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    mutex.lock();
    if (0 != track_data.size())
    {
        //painter.fillRect(0, 0, width(), height(), QBrush(QColor(rand()%256, rand()%256, rand()%256)));
        for (int i = 0; i < width(); i++)
        {
            painter.drawLine(i, height()/2, i, track_data[i]);
        }
    }
    else
    {
        //painter.fillRect(0, 0, width(), height(), QBrush(QColor(rand()%256, rand()%256, rand()%256)));
    }
    mutex.unlock();
}

void Track_widget::set_track_data(int32_t* data, int size, int max_value)
{
    mutex.lock();
    track_data.resize(width());
    if (max_value == (1<<7))
    {
        for (int i = 0; i < width(); i++)
        {
            track_data[i] = ((height()/2)-((height()/2)*((max_value-(double)data[i])/(double)max_value)));
        }
    }
    else
    {
        for (int i = 0; i < width(); i++)
        {
            track_data[i] = ((height()/2)-((height()/2)*((double)data[i])/(double)max_value));
        }
    }
    mutex.unlock();
    update();
}
