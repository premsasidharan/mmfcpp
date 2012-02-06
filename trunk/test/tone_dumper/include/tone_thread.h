#ifndef _TONE_THREAD_H
#define _TONE_THREAD_H

#include <QThread>
#include <QAtomicInt>

class Tone_thread:public QThread
{
    Q_OBJECT
public:
    Tone_thread(QObject* parent = 0);
    ~Tone_thread();

public:
    void stop_thread();
    void set_parameters(const QString& path, int srate, int _bps, int _channels, int _f, int t);
    int get_progress() { return progress; };

protected:
    void run();

private:
    int bps;
    int channels;
    int duration;
    int frequency;
    int sample_rate;

    bool stop_flag;

    QString file_path;

    QAtomicInt progress;
};

#endif

