#include <offline_widget.h>

#include <gl_thread.h>

Offline_widget::Offline_widget(int width, int height, const QString& path, QGLWidget* shared, QWidget *parent)
    :QGLWidget(parent, shared)
    , file(path)
    , video_width(width)
    , video_height(height)
    , thread(this)
{
    resize(0, 0);
}

Offline_widget::~Offline_widget()
{
    glDeleteBuffers(10, buffer);
}

void Offline_widget::paintGL()
{
    glBegin(GL_QUADS);
    glVertex2f(-1.0, -1.0);
    glVertex2f(1.0, -1.0); 
    glVertex2f(1.0, 1.0); 
    glVertex2f(-1.0, 1.0); 
    glEnd();
}

void Offline_widget::initializeGL()
{
    mutex.lock();
    glGenBuffers(10, buffer);
    for (int i = 0; i < 10; i++)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, (video_width*video_height*3)>>1, 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        free.enqueue(buffer[i]);
    }
    mutex.unlock();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    thread.start();
}

void Offline_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
}

void Offline_widget::run()
{
    bool flag = file.open(QIODevice::ReadOnly);
    if (false == flag)
    {
        qDebug() << "Unable to Open File: " << file.fileName();
        return;
    }

    makeCurrent();
    while (!file.atEnd())
    {
        if (free.isEmpty())
        {
            //qDebug() << "Free Queue Empty 1:" << QTime::currentTime().toString("ss:zzz");
            wc_mutex.lock();
            wc.wait(&wc_mutex);
            wc_mutex.unlock();
            //qDebug() << "Free Queue Empty 2:" << QTime::currentTime().toString("ss:zzz");
        }
        else
        {
            mutex.lock();
            GLuint buff_id = free.dequeue();
            mutex.unlock();
            
            //makeCurrent();
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
            char* y_data = (char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
            file.read(y_data, (video_width*video_height*3)>>1);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            //doneCurrent();
            
            mutex.lock();
            render.enqueue(buff_id);
            mutex.unlock();
        }
    }
    doneCurrent();

    file.close();
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

