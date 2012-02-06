#ifndef _TONE_DLG_H
#define _TONE_DLG_H

#include <ui_tone_dlg.h>

#include <QTimer>
#include <QDialog>
#include <tone_thread.h>

class Tone_dlg:public QDialog, public Ui::Tone_dlg_ui
{
    Q_OBJECT
public:
    Tone_dlg(QWidget* parent = 0);
    ~Tone_dlg();

protected slots:
    void on_file();
    void on_stop();
    void on_start();

    void on_timer_elapsed();
    void on_thread_started();
    void on_thread_finished();

    void on_bps(int sel);
    void on_channel(int chan_count);
    void on_frequency(const QString& freq);
    void on_duration(const QString& duration);
    void on_sample_rate(const QString& srate);


protected:
    void update_path();
    void enable_controls(bool state);

private:
    int duration;
    int frequency;
    int sample_rate;
    int channel_count;

    QString file_path;

    QTimer timer;
    Tone_thread thread;
    QIntValidator freq_valid;
    QIntValidator duration_valid;
    QIntValidator sample_rate_valid;
};

#endif

