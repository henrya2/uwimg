#include <math.h>
#include "image.h"

#define MY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MY_MAX(a, b) ((a) > (b) ? (a) : (b))

float frac(float v)
{
    return v - floor(v);
}

int floor_int(float v)
{
    return (int)floor(v);
}

int round_int(float v)
{
    return (int)round(v);
}

float nn_interpolate(image im, float x, float y, int c)
{
    int round_x = MY_MIN(round_int(x), im.w - 1);
    int round_y = MY_MIN(round_int(y), im.h - 1);
    return get_pixel(im, round_x, round_y, c);
}

image nn_resize(image im, int w, int h)
{
    int c = im.c;
    image new_image = make_image(w, h, c);

    for (int n_c = 0; n_c < c; ++n_c)
    {
        for (int y = 0; y < h; ++y)
        {
            float src_y = ((y + 0.5f) * im.h) / (float)h - 0.5f;
            for (int x = 0; x < w; ++x)
            {
                float src_x = ((x + 0.5f) * im.w) / (float)w - 0.5f;
                float value = nn_interpolate(im, src_x, src_y, n_c);
                set_pixel(new_image, x, y, n_c, value);
            }
        }
    }

    return new_image;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    int x0 = floor_int(x);
    int y0 = floor_int(y);
    int x1 = MY_MIN(x0 + 1, im.w - 1);
    int y1 = MY_MIN(y0 + 1, im.h - 1);

    float frac_x1 = frac(x);
    float frac_y1 = frac(y);
    float frac_x0 = 1.0f - frac_x1;
    float frac_y0 = 1.0f - frac_y1;

    float ch00 = get_pixel(im, x0, y0, c);
    float ch01 = get_pixel(im, x0, y1, c);
    float ch10 = get_pixel(im, x1, y0, c);
    float ch11 = get_pixel(im, x1, y1, c);

    // lerp(lerp(ch00, ch01, frac_y1), lerp(ch10, ch11, frac_y1), frac_x1);
    return 
    ch00 * (frac_x0 * frac_y0) + 
    ch01 * (frac_x0 * frac_y1) +
    ch10 * (frac_x1 * frac_y0) + 
    ch11 * (frac_x1 * frac_y1);
}

image bilinear_resize(image im, int w, int h)
{
    int c = im.c;
    image new_image = make_image(w, h, c);

    float src_over_dest_width = im.w / (float)(w);
    float src_over_dest_height = im.h / (float)(h);

    for (int n_c = 0; n_c < c; ++n_c)
    {
        for (int y = 0; y < h; ++y)
        {
            float src_y = (y + 0.5f) * src_over_dest_height - 0.5f;
            for (int x = 0; x < w; ++x)
            {
                float src_x = (x + 0.5f) * src_over_dest_width - 0.5f;
                float value = bilinear_interpolate(im, src_x, src_y, n_c);
                set_pixel(new_image, x, y, n_c, value);
            }
        }
    }

    return new_image;
}

