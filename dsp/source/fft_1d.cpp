#include <complex.h>
#include <fft_1d.h>

int compute_fft_1d(complex* data, int length)
{
    int stage_count = 0;
    asm volatile("bsr %0, %1" : "=r" (stage_count) : "r" (length));
    if (0 == length || length != (1 << stage_count))
    {
        printf("\nlength: %d not a power of 2\n", length);
        return 0;
    }

    double angle;
    complex wstart, wdata;
    int steps, stage_size = 1;
    for (int i = 0; i < stage_count; i++)
    {
        steps = (stage_size<<1);
        angle = (-1*M_PI)/((double)stage_size);
        wstart = complex::polar(1.0, angle);
        wdata = complex::cartesian(1.0, 0.0);
        for (int j = 0; j < stage_size; j++)
        {
            wdata *= wstart;
            for (int top = j; top < length; top += steps)
            {
                int bottom = top+stage_size;
                complex temp = data[bottom]*wdata;
                data[bottom] = data[top]-temp;
                data[top] += temp;
            }
        }
        stage_size = steps;
    }
    return 1;
}
