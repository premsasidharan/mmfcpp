#ifndef _TRACK_WIDGET_H_
#define _TRACK_WIDGET_H_

#include <QMutex>
#include <QWidget>
#include <QVector>

class Track_widget:public QWidget
{
    Q_OBJECT
public:
    Track_widget(QWidget* parent);
    ~Track_widget();

public:
    void set_track_data(int32_t* data, int size, int max_value);

protected:
     void paintEvent(QPaintEvent* event);

private:
     QMutex mutex;
     QVector<int> track_data;
};

#endif
