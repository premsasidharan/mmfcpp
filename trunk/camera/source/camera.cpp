#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#include <camera.h>

struct Buffer
{
    void* start;
    size_t length;
};

Camera::Camera()
    :width(0)
    , height(0)
    , device(-1)
    , buffer(0)
    , num_buffers(0)
    , device_name(0)
{
}

Camera::~Camera()
{
    close();
}

bool Camera::open(const char* _name)
{
    close();
    bool flag;
    int length = strlen(_name);
    device_name = new char[length+1];
    strcpy(device_name, _name);

    device = ::open(device_name, O_RDWR|O_NONBLOCK, 0);
    flag = (-1 != device);
    flag = flag && initialize();

    return flag;
}

void Camera::close()
{
    stop();
    unmap_device();

    if (0 != device_name)
    {
        delete [] device_name;
        device_name = 0;
    }

    if (device >= 0)
    {
        ::close(device);
    }

    device = -1;
}

void Camera::stop()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(VIDIOC_STREAMOFF, &type);
}

bool Camera::start()
{
    bool flag = (-1 != device);
    if (flag)
    {
        struct v4l2_buffer buf;
        enum v4l2_buf_type type;
        for (int i = 0; i < num_buffers; ++i)
        {
            memset(&buf, 0, sizeof(buf));

            buf.index = i;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            flag = flag && ioctl(VIDIOC_QBUF, &buf);
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        flag = flag && ioctl(VIDIOC_STREAMON, &type);
    }
    return flag;
}

bool Camera::initialize()
{
    bool flag = (-1 != device);
    if (flag)
    {
        unsigned int min;
        struct v4l2_crop crop;
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_fmtdesc fmt_desc;
        struct v4l2_frmsizeenum frame_sizes;

        flag = ioctl(VIDIOC_QUERYCAP, &cap);

        printf("\nCapabilities: 0x%x", cap.capabilities);
        flag = flag && (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && (cap.capabilities & V4L2_CAP_STREAMING);

        /* Select video input, video standard and tune here. */
        memset (&cropcap, 0, sizeof(cropcap));
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        flag = flag && ioctl(VIDIOC_CROPCAP, &cropcap);
        if (flag)
        {
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect; /* reset to default */
            printf("\n%d %d %d %d", cropcap.defrect.left, cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height);

            //ioctl (VIDIOC_S_CROP, &crop);

            bool result = true;
            for (int i = 0; ; i++)
            {
                memset(&fmt_desc, 0, sizeof(fmt_desc));
                fmt_desc.index = i;
                fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                result = result && ioctl(VIDIOC_ENUM_FMT, &fmt_desc);
                if (true == result)
                {
                    char* ch = (char *)&fmt_desc.pixelformat;
                    printf("\n%d flags: 0x%x, %s, %c%c%c%c", i, fmt_desc.flags, fmt_desc.description, ch[0], ch[1], ch[2], ch[3]);
                    for (int j = 0; ; j++)
                    {
                        memset(&frame_sizes, 0, sizeof(frame_sizes));
                        frame_sizes.index = j;
                        frame_sizes.pixel_format = fmt_desc.pixelformat;
                        bool ret = ioctl(VIDIOC_ENUM_FRAMESIZES, &frame_sizes);
                        if (true == ret)
                        {
                            switch (frame_sizes.type)
                            {
                                case V4L2_FRMSIZE_TYPE_DISCRETE:
                                    printf("\n%d Discrete(widthxheight): %dx%d", j, frame_sizes.discrete.width, frame_sizes.discrete.height);
                                    break;
                                case V4L2_FRMSIZE_TYPE_STEPWISE:
                                case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                                    printf("\n%d Step-wise (min_widthxmin_height, step_widthxstep_height, max_widthxmax_height: %dx%d", j,
                                        frame_sizes.stepwise.min_width, frame_sizes.stepwise.min_height,
                                        frame_sizes.stepwise.step_width, frame_sizes.stepwise.step_height,
                                        frame_sizes.stepwise.max_width, frame_sizes.stepwise.max_height);
                                    break;
                                default:
                                    printf("\nNone");
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    return flag;
}

bool Camera::set_mode(unsigned int f, int w, int h)
{
    bool flag = (-1 != device);
    if (flag)
    {
        width = w; height = h;

        struct v4l2_format fmt; 
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = f;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        flag = ioctl(VIDIOC_S_FMT, &fmt);
        flag = flag && mmap_device();
    }
    return flag;
}

int Camera::read(unsigned char* yuv, int size)
{
    int ret = 0;
    if (-1 != device)
    {
        bool flag;
        struct v4l2_buffer v4l2_buf;
        memset(&v4l2_buf, 0, sizeof(v4l2_buf));
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        flag = ioctl(VIDIOC_DQBUF, &v4l2_buf);

        if (flag && v4l2_buf.index < (unsigned int)num_buffers)
        {
            int data_size = buffer[v4l2_buf.index].length;
            unsigned char* data = (unsigned char*) buffer[v4l2_buf.index].start;
            if (0 != data && data_size <= size)
            {
                memcpy(yuv, data, data_size);
                ret = data_size;
            }
        }
        ioctl(VIDIOC_QBUF, &v4l2_buf);
    }
    return ret;
}

bool Camera::wait()
{
    fd_set fds;
    int ret = 0;
    struct timeval tv;
    int flag = (-1 != device);

    while (flag)
    {
        FD_ZERO(&fds);
        FD_SET(device, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        ret = select(device+1, &fds, 0, 0, &tv);
        if (-1 == ret && EINTR == errno)
        {
            continue;
        }

        if (0 == ret)
        {
            printf("\nTimeout expired");
        }

        flag = 0;
    }

    return (ret > 0);
}

bool Camera::mmap_device()
{
    bool flag = (-1 != device);
    if (flag)
    {
        struct v4l2_requestbuffers req;
        memset(&req, 0, sizeof(req));

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        flag = ioctl(VIDIOC_REQBUFS, &req);

        if (flag && req.count >= 2)
        {
            buffer = (struct Buffer*) calloc(req.count, sizeof(Buffer));
            if (0 != buffer)
            {
                int i;
                struct v4l2_buffer buf;
                num_buffers = req.count;
                for (i = 0; i < num_buffers && flag; ++i)
                {
                    memset(&buf, 0, sizeof(buf));
                    buf.index = i;
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;

                    flag = flag && ioctl(VIDIOC_QUERYBUF, &buf);

                    buffer[i].length = buf.length;
                    buffer[i].start = mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, device, buf.m.offset);

                    flag = flag && (!(MAP_FAILED == buffer[i].start));
                }

                if (false == flag)
                {
                    unmap_device();
                }
            }
        }
    }
    return flag;
}

void Camera::unmap_device()
{
    if (0 != buffer)
    {
        for (int i = 0; i < num_buffers; ++i)
        {
            munmap(buffer[i].start, buffer[i].length);
        }

        free(buffer);
        buffer = 0;
        num_buffers = 0;
    }
}

bool Camera::ioctl(int request, void* arg)
{
    return (device == -1)?false:((0 == ::ioctl(device, request, arg))?true:false);
}

