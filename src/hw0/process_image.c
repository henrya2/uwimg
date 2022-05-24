#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    if (x < 0)
    {
        x = 0;
    }
    else if (x >= im.w)
    {
        x = im.w - 1;
    }

    if (y < 0)
    {
        y = 0;
    }
    else if (y >= im.h)
    {
        y = im.h - 1;
    }

    if (c < 0)
    {
        c = 0;
    }
    else if (c >= im.c)
    {
        c = im.c - 1;
    }

    return im.data[x + y * im.w + c * im.w * im.h];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if (x < 0 || x >= im.w)
        return;
    if (y < 0 || y >= im.h)
        return;
    if (c < 0 || c >= im.c)
        return;

    im.data[x + y * im.w + c * im.w * im.h] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    // TODO Fill this in
    memcpy(copy.data, im.data, sizeof(float) * im.w * im.h * im.c);
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    // TODO Fill this in
    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            // Y' = 0.299 R' + 0.587 G' + .114 B'
            float r = get_pixel(im, x, y, 0);
            float g = get_pixel(im, x, y, 1);
            float b = get_pixel(im, x, y, 2);
            float gray_value = 0.299f * r + 0.587f * g + 0.114f * b;
            set_pixel(gray, x, y, 0, gray_value);
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    // TODO Fill this in
    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            float value = get_pixel(im, x, y, c);
            set_pixel(im, x, y, c, value + v);
        }
    }
}

void clamp_image(image im)
{
    // TODO Fill this in
    for (int n = 0; n < im.c; ++n)
    {
        for (int y = 0; y < im.h; ++y)
        {
            for (int x = 0; x < im.w; ++x)
            {
                float value = get_pixel(im, x, y, n);
                if (value < 0)
                {
                    value = 0;
                }
                if (value > 1.0f)
                {
                    value = 1.0f;
                }
                set_pixel(im, x, y, n, value);
            }
        }
    }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    // TODO Fill this 
    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            float r = get_pixel(im, x, y, 0);
            float g = get_pixel(im, x, y, 1);
            float b = get_pixel(im, x, y, 2);
            float V_val = three_way_max(r, g, b);
            float m_val = three_way_min(r, g, b);
            float C_val = V_val - m_val;
            float S_val = (V_val == 0) ? (0) : (C_val / V_val);
            float Ha_val = 0;
            if (C_val == 0)
            {
                Ha_val = 0;
            }
            else if (V_val == r)
            {
                Ha_val = (g - b) / C_val;
            }
            else if (V_val == g)
            {
                Ha_val = (b - r) / C_val + 2;
            }
            else if (V_val == b)
            {
                Ha_val = (r - g) / C_val + 4;
            }

            float H_val = Ha_val / 6;
            if (H_val < 0)
            {
                H_val += 1.0f;
            }

            set_pixel(im, x, y, 0, H_val);
            set_pixel(im, x, y, 1, S_val);
            set_pixel(im, x, y, 2, V_val);
        }
    }
}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
    for (int y = 0; y < im.h; ++y)
    {
        for (int x = 0; x < im.w; ++x)
        {
            float red, green, blue;

            float hue = get_pixel(im, x, y, 0);
            float saturation = get_pixel(im, x, y, 1);
            float value = get_pixel(im, x, y, 2);

            float chroma = saturation * value;
            float minValue = value - chroma;
            float Hprime = hue * 6;
            float tmp_x = chroma * (1 - fabs(fmod(Hprime, 2) - 1));

            if (Hprime < 1)
            {
                red = chroma;
                green = tmp_x;
                blue = 0;
            }
            else if (Hprime < 2)
            {
                red = tmp_x;
                green = chroma;
                blue = 0;
            }
            else if (Hprime < 3)
            {
                red = 0;
                green = chroma;
                blue = tmp_x;
            }
            else if (Hprime < 4)
            {
                red = 0;
                green = tmp_x;
                blue = chroma;
            }
            else if (Hprime < 5)
            {
                red = tmp_x;
                green = 0;
                blue = chroma;
            }
            else
            { // Hprime < 6
                red = chroma;
                green = 0;
                blue = tmp_x;
            }

            set_pixel(im, x, y, 0, red + minValue);
            set_pixel(im, x, y, 1, green + minValue);
            set_pixel(im, x, y, 2, blue + minValue);
        }
    }
}
