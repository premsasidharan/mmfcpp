#include <offline_widget.h>

#include <gl_thread.h>

Offline_widget::Offline_widget(int width, int height, const QString& path, QGLWidget* shared, QWidget *parent)
    :QGLWidget(parent, shared)
    , flag(true)
    , dev_path(path)
    , video_width(width)
    , video_height(height)
    , thread(this)
{
    resize(0, 0);
}

Offline_widget::~Offline_widget()
{
    flag = false;
    glDeleteBuffers(10, buffer);
}

void Offline_widget::paintGL()
{
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f); 
    glVertex2f(1.0f, 1.0f); 
    glVertex2f(-1.0f, 1.0f); 
    glEnd();
}

void Offline_widget::initializeGL()
{
    mutex.lock();
    glGenBuffers(10, buffer);
    for (int i = 0; i < 10; i++)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, (video_width*video_height*2), 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        free.enqueue(buffer[i]);
    }
    mutex.unlock();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    thread.start();
}

void Offline_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
}

void Offline_widget::run()
{
    int data_size = video_width*video_height*2;
    flag = camera.open(dev_path.toAscii().data());
    flag = flag && camera.set_mode(V4L2_PIX_FMT_YUYV, video_width, video_height);
    flag = flag && camera.start();
    if (false == flag)
    {
        qDebug() << "Unable to use device: " << dev_path;
    }

    makeCurrent();
    while (flag)
    {
        if (free.isEmpty())
        {
            wc_mutex.lock();
            wc.wait(&wc_mutex);
            wc_mutex.unlock();
        }
        else
        {
            mutex.lock();
            GLuint buff_id = free.dequeue();
            mutex.unlock();
            
            bool status = camera.wait();
            flag = flag && status; 
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
            unsigned char* y_data = (unsigned char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            flag = flag && (data_size == camera.read(y_data, data_size));
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            
            mutex.lock();
            render.enqueue(buff_id);
            mutex.unlock();

            emit update_widget();
        }
    }
    doneCurrent();

    camera.stop();
    camera.close();

    wc.wakeOne();

    qDebug() << "Exiting thread";
}

GLuint Offline_widget::renderBuffer()
{
    GLuint buff_id;
    mutex.lock();
    buff_id = render.dequeue();
    mutex.unlock();
    return buff_id;
}

bool Offline_widget::isRenderBufferEmpty()
{
    return render.isEmpty();
}

void Offline_widget::addToFreeQueue(GLuint id)
{
    mutex.lock();
    free.enqueue(id);
    mutex.unlock();
    wc.wakeOne();
}

void Offline_widget::stop_thread()
{
    wc.wakeOne();
    flag = false;
    if (thread.isRunning())
    {
        wc_mutex.lock();
        wc.wait(&wc_mutex);
        wc_mutex.unlock();
    }
}

