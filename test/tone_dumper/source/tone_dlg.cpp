#include <tone_dlg.h>

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

Tone_dlg::Tone_dlg(QWidget* parent)
    :QDialog(parent)
    , duration(60)
    , frequency(1000)
    , sample_rate(44100)
    , channel_count(2)
    , timer(this)
    , thread(this)
    , freq_valid(this)
    , duration_valid(this)
    , sample_rate_valid(this)
{
    setupUi(this);

    bps_combo_box->setCurrentIndex(1);

    chan_edit->setRange(1, 10);
    chan_edit->setValue(channel_count);

    sample_rate_valid.setRange(1, 0x7FFFFFFF);
    sample_rate_edit->setValidator(&sample_rate_valid);
    sample_rate_edit->setText(QString("%1").arg(sample_rate));

    duration_valid.setRange(1, 60*60); //60mins
    duration_edit->setValidator(&duration_valid);
    duration_edit->setText(QString("%1").arg(duration));

    freq_valid.setRange(1, sample_rate/2);
    freq_edit->setValidator(&freq_valid);
    freq_edit->setText(QString("%1").arg(frequency));

    file_path =  QDir::homePath()+QDir::separator()+
                 QString("test_%1Hz_%2_%3ch_%4Hz_%5sec.wav")
                  .arg(sample_rate)
                  .arg(bps_combo_box->itemText(bps_combo_box->currentIndex()))
                  .arg(channel_count)
                  .arg(frequency)
                  .arg(duration);
    file_edit->setText(file_path);

    connect(file_btn, SIGNAL(clicked()), this, SLOT(on_file()));
    connect(start_btn, SIGNAL(clicked()), this, SLOT(on_start()));
    connect(stop_btn, SIGNAL(clicked()), this, SLOT(on_stop()));

    connect(chan_edit, SIGNAL(valueChanged(int)), this, SLOT(on_channel(int)));
    connect(freq_edit, SIGNAL(textChanged(const QString&)), this, SLOT(on_frequency(const QString&)));
    connect(duration_edit, SIGNAL(textChanged(const QString&)), this, SLOT(on_duration(const QString&)));
    connect(sample_rate_edit, SIGNAL(textChanged(const QString&)), this, SLOT(on_sample_rate(const QString&)));

    connect(bps_combo_box, SIGNAL(activated(int)), this, SLOT(on_bps(int)));
    connect(&thread, SIGNAL(started()), this, SLOT(on_thread_started()));
    connect(&thread, SIGNAL(finished()), this, SLOT(on_thread_finished()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer_elapsed()));

    file_progress->reset();
    file_edit->setReadOnly(true);

    stop_btn->setEnabled(false);
    timer.setInterval(500);
}

Tone_dlg::~Tone_dlg()
{
}

void Tone_dlg::on_file()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"),
                             file_path, tr("Wave File (*.wav)"));
    if (!path.isEmpty())
    {
        file_path = path;
        file_edit->setText(file_path);
    }
}

void Tone_dlg::on_start()
{
    thread.set_parameters(file_path, sample_rate,
        1+bps_combo_box->currentIndex(), channel_count, frequency, duration);
    thread.start();
}

void Tone_dlg::on_stop()
{
    thread.stop_thread();
}

void Tone_dlg::on_bps(int sel)
{
    duration_edit->setValidator(0);
    duration_valid.setRange(1, (sel+1)*1000);
    duration_edit->setValidator(&duration_valid);
    update_path();
}

void Tone_dlg::on_channel(int chan_count)
{
    channel_count = chan_count;
    update_path();
}

void Tone_dlg::on_frequency(const QString& freq)
{
    frequency = freq.toInt();
    update_path();
}

void Tone_dlg::on_duration(const QString& dur)
{
    duration = dur.toInt();
    update_path();
}

void Tone_dlg::on_sample_rate(const QString& srate)
{
    sample_rate = srate.toInt();
    update_path();
    freq_edit->setValidator(0);
    freq_valid.setRange(1, sample_rate/2);
    freq_edit->setValidator(&freq_valid);
}

void Tone_dlg::update_path()
{
    int n = file_path.length()-file_path.lastIndexOf(QDir::separator());
    file_path.chop(n);
    qDebug() << file_path;
    file_path = file_path+QDir::separator()+
                 QString("test_%1Hz_%2_%3ch_%4Hz_%5sec.wav")
                  .arg(sample_rate)
                  .arg(bps_combo_box->itemText(bps_combo_box->currentIndex()))
                  .arg(channel_count)
                  .arg(frequency)
                  .arg(duration);
    file_edit->setText(file_path);
}

void Tone_dlg::on_thread_started()
{
    enable_controls(false);
    file_progress->setRange(0, duration*sample_rate);
    timer.start();
}

void Tone_dlg::on_thread_finished()
{
    enable_controls(true);
    on_timer_elapsed();
    timer.stop();
}

void Tone_dlg::on_timer_elapsed()
{
    file_progress->setValue(thread.get_progress());
}

void Tone_dlg::enable_controls(bool state)
{
    bps_combo_box->setEnabled(state);
    chan_edit->setEnabled(state);
    sample_rate_edit->setEnabled(state);
    duration_edit->setEnabled(state);
    freq_edit->setEnabled(state);
    start_btn->setEnabled(state);
    file_btn->setEnabled(state);
    stop_btn->setEnabled(!state);
}
