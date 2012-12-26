#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <linux/videodev2.h>

struct Buffer;

class Camera
{
public:
    Camera();
    ~Camera();

public:
    bool open(const char* name);
    void close();

    bool set_mode(unsigned int f, int w, int h);

    bool start();
    void stop();

    bool wait();
    int read(unsigned char* data, int size);

protected:
    bool initialize();
    bool mmap_device();
    void unmap_device();
    bool ioctl(int request, void* arg);

protected:
    int width;
    int height;
    int device;
    Buffer* buffer;
    int num_buffers;
    char* device_name;
};

#endif

