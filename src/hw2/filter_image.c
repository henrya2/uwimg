#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{
    float sum = 0;

    for (int n = 0; n < im.c; ++n)
    {
        for (int y = 0; y < im.h; ++y)
        {
            for (int x = 0; x < im.w; ++x)
            {
                sum += get_pixel(im, x, y, n);
            }
        }
    }   

    if (sum <= 0)
        return;

    for (int n = 0; n < im.c; ++n)
    {
        for (int y = 0; y < im.h; ++y)
        {
            for (int x = 0; x < im.w; ++x)
            {
                float value = get_pixel(im, x, y, n);
                set_pixel(im, x, y, n, value / sum);
            }
        }
    }    
}

image make_box_filter(int w)
{
    float v = 1.0f / (w * w);
    image im = make_image(w, w, 1);

    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            set_pixel(im, x, y, 0, v);
        }
    }

    return im;
}

float convolve_v(image im, image filter, int x, int y, int n_c)
{
    int half_w = filter.w / 2;
    int half_h = filter.h / 2;

    float sum = 0;

    for (int y_conv = -half_h; y_conv <= half_h; ++y_conv)
    {
        int filter_y = y_conv + half_h;
        int im_y = y + y_conv;
        for (int x_conv = -half_w; x_conv <= half_w; ++x_conv)
        {
            int filter_x = x_conv + half_w;
            int im_x = x + x_conv;

            int filter_c = MIN(n_c, filter.c - 1);
            sum += get_pixel(im, im_x, im_y, n_c) * get_pixel(filter, filter_x, filter_y, filter_c);
        }
    }

    return sum;
}

image convolve_image(image im, image filter, int preserve)
{
    assert(im.c == filter.c || filter.c == 1);

    image new_im = make_image(im.w, im.h, preserve ? im.c : 1);

    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            if (preserve)
            {
                for (int n = 0; n < im.c; ++n)
                {
                    float conv_v = convolve_v(im, filter, x, y, n);
                    set_pixel(new_im, x, y, n, conv_v);
                }
            }
            else
            {
                float conv_sum = 0;

                for (int n = 0; n < im.c; ++n)
                {
                    conv_sum += convolve_v(im, filter, x, y, n);
                }

                set_pixel(new_im, x, y, 0, conv_sum);
            }
        }
    }

    return new_im;
}

image make_highpass_filter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0,  0.0);
    set_pixel(filter, 1, 0, 0, -1.0);
    set_pixel(filter, 2, 0, 0,  0.0);
    set_pixel(filter, 0, 1, 0, -1.0);
    set_pixel(filter, 1, 1, 0,  4.0);
    set_pixel(filter, 2, 1, 0, -1.0);
    set_pixel(filter, 0, 2, 0,  0.0);
    set_pixel(filter, 1, 2, 0, -1.0);
    set_pixel(filter, 2, 2, 0,  0.0);

    return filter;
}

image make_sharpen_filter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, 0.0);
    set_pixel(filter, 1, 0, 0, -1.0);
    set_pixel(filter, 2, 0, 0, 0.0);
    set_pixel(filter, 0, 1, 0, -1.0);
    set_pixel(filter, 1, 1, 0, 5.0);
    set_pixel(filter, 2, 1, 0, -1.0);
    set_pixel(filter, 0, 2, 0, 0.0);
    set_pixel(filter, 1, 2, 0, -1.0);
    set_pixel(filter, 2, 2, 0, 0.0);

    return filter;
}

image make_emboss_filter()
{
    image filter = make_image(3, 3, 1);
    set_pixel(filter, 0, 0, 0, -2.0);
    set_pixel(filter, 1, 0, 0, -1.0);
    set_pixel(filter, 2, 0, 0, 0.0);
    set_pixel(filter, 0, 1, 0, -1.0);
    set_pixel(filter, 1, 1, 0, 1.0);
    set_pixel(filter, 2, 1, 0, 1.0);
    set_pixel(filter, 0, 2, 0, 0.0);
    set_pixel(filter, 1, 2, 0, 1.0);
    set_pixel(filter, 2, 2, 0, 2.0);

    return filter;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: 
// Sharpen_filter, emboss filter should user preserve, since we need color.
// High pass filter should not use preserve, since we need frequency.

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: 
// All filters need to do clamp. 
// Because all filters may make some channel values below 0 or above 1.

float gaussian_2d(float x, float y, float sigma)
{
    return (1.0f / (TWOPI * sigma * sigma)) * expf(-(x * x + y * y)/ (2.0f * sigma * sigma));
}

image make_gaussian_filter(float sigma)
{
    int filter_w = (int)ceilf(6 * sigma);
    if ((filter_w & 1) == 0)
        filter_w += 1;

    int half_filer_w = filter_w / 2;

    image im = make_image(filter_w, filter_w, 1);

    for (int y = -half_filer_w; y <= half_filer_w; ++y)
    {
        for (int x = -half_filer_w; x <= half_filer_w; ++x)
        {
            float g = gaussian_2d(x, y, sigma);
            int x_i = x + half_filer_w;
            int y_i = y + half_filer_w;
            set_pixel(im, x_i, y_i, 0, g);
        }
    }
    l1_normalize(im);
    
    return im;
}

image add_image(image a, image b)
{
    image new_im = make_image(a.w, a.h, a.c);
    for (int n = 0; n < a.c; ++n)
    {
        for (int y = 0; y < a.h; ++y)
        {
            for (int x = 0; x < a.w; ++x)
            {
                float v = get_pixel(a, x, y, n) + get_pixel(b, x, y, n);
                set_pixel(new_im, x, y, n, v);
            }
        }
    }
    return new_im;
}

image sub_image(image a, image b)
{
    image new_im = make_image(a.w, a.h, a.c);
    for (int n = 0; n < a.c; ++n)
    {
        for (int y = 0; y < a.h; ++y)
        {
            for (int x = 0; x < a.w; ++x)
            {
                float v = get_pixel(a, x, y, n) - get_pixel(b, x, y, n);
                set_pixel(new_im, x, y, n, v);
            }
        }
    }
    return new_im;
}

image make_gx_filter()
{
    image im = make_image(3, 3, 1);
    set_pixel(im, 0, 0, 0, -1.0);
    set_pixel(im, 1, 0, 0,  0.0);
    set_pixel(im, 2, 0, 0,  1.0);
    set_pixel(im, 0, 1, 0, -2.0);
    set_pixel(im, 1, 1, 0,  0.0);
    set_pixel(im, 2, 1, 0,  2.0);
    set_pixel(im, 0, 2, 0, -1.0);
    set_pixel(im, 1, 2, 0,  0.0);
    set_pixel(im, 2, 2, 0,  1.0);
    
    return im;
}

image make_gy_filter()
{
    image im = make_image(3, 3, 1);
    set_pixel(im, 0, 0, 0, -1.0);
    set_pixel(im, 1, 0, 0, -2.0);
    set_pixel(im, 2, 0, 0, -1.0);
    set_pixel(im, 0, 1, 0,  0.0);
    set_pixel(im, 1, 1, 0,  0.0);
    set_pixel(im, 2, 1, 0,  0.0);
    set_pixel(im, 0, 2, 0,  1.0);
    set_pixel(im, 1, 2, 0,  2.0);
    set_pixel(im, 2, 2, 0,  1.0);

    return im;
}

void feature_normalize(image im)
{
    float min_val = get_pixel(im, 0, 0, 0);
    float max_val = get_pixel(im, 0, 0, 0);

    for (int n = 0; n < im.c; ++n)
    {
        for (int y = 0; y < im.h; ++y)
        {
            for (int x = 0; x < im.w; ++x)
            {
                float val = get_pixel(im, x, y, n);
                min_val = MIN(min_val, val);
                max_val = MAX(max_val, val);
            }
        }
    }
    float range_val = max_val - min_val;

    if (range_val == 0)
    {
        memset(im.data, 0, sizeof(float) * im.c * im.w * im.h);
        return;
    }

    for (int n = 0; n < im.c; ++n)
    {
        for (int y = 0; y < im.h; ++y)
        {
            for (int x = 0; x < im.w; ++x)
            {
                float val = get_pixel(im, x, y, n);
                float norm_val = (val - min_val) / range_val;
                set_pixel(im, x, y, n, norm_val);
            }
        }
    }
}

image *sobel_image(image im)
{
    image im_mag = make_image(im.w, im.h, 1);
    image im_dir = make_image(im.w, im.h, 1);
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    image im_gx = convolve_image(im, gx_filter, 0);
    image im_gy = convolve_image(im, gy_filter, 0);
    for (int n = 0; n < im.c; ++n) 
    {
        for (int y = 0; y < im.h; ++y) 
        {
            for (int x = 0; x < im.w; ++x) 
            {
                float gx_val = get_pixel(im_gx, x, y, n);
                float gy_val = get_pixel(im_gy, x, y, n);
                set_pixel(im_mag, x, y, n, sqrtf(gx_val * gx_val + gy_val * gy_val));
                set_pixel(im_dir, x, y, n, atan2f(gy_val, gx_val));
            }
        }
    }

    image *results = calloc(2, sizeof(image));
    results[0] = im_mag;
    results[1] = im_dir;
    return results;
}

image colorize_sobel(image im)
{
    image colorized = make_image(im.w, im.h, im.c);
    image *sobel = sobel_image(im);
    image im_mag = sobel[0];
    image im_dir = sobel[1];
    for (int y = 0; y < im.h; ++y) 
    {
        for (int x = 0; x < im.w; ++x) 
        {
            float mag_val = get_pixel(im_mag, x, y, 0);
            float dir_val = get_pixel(im_dir, x, y, 0);

            set_pixel(colorized, x, y, 0, dir_val);
            set_pixel(colorized, x, y, 1, 1.0);
            set_pixel(colorized, x, y, 2, mag_val);
        }
    }

    hsv_to_rgb(colorized);
    return colorized;
}
