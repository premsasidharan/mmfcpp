#include <player_window.h>

#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>

Player_window::Player_window()
    :QMainWindow(0)
    , timer(this)
    , player(this)
{
    setupUi(this);
    initialize();
}

Player_window::~Player_window()
{
}

void Player_window::initialize()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer_elapsed()));
    connect(file_open, SIGNAL(triggered()), this, SLOT(on_file_open()));
    connect(play_pause_btn, SIGNAL(clicked()), this, SLOT(on_play_pause()));
    timer.setInterval(250);
}

void Player_window::on_file_open()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Wave File (*.wav)"));
    if (player.set_file_path(path.toAscii()))
    {
        playback_slider->setRange(0, player.duration());
        playback_slider->setValue(0);
    }
}

void Player_window::on_play_pause()
{
    if (play_pause_btn->text() == tr("Play"))
    {
        timer.start();
        player.start(0);
        play_pause_btn->setText(tr("Pause"));
    }
    else
    {
        int time = 0;
        timer.stop();
        player.stop(time);
        play_pause_btn->setText(tr("Play"));
        on_timer_elapsed();
    }
}

void Player_window::on_timer_elapsed()
{
    int current_pos = player.current_position();
    playback_slider->setValue(current_pos);
    qDebug() << "Here " << current_pos << ", " << playback_slider->maximum();
    if (current_pos >= playback_slider->maximum())
    {
        timer.stop();
    }
}
