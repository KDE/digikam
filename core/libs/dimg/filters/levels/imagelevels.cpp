/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-29
 * Description : image levels manipulation methods.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

/*  Map RGB to intensity  */

#define LEVELS_RGB_INTENSITY_RED    0.30
#define LEVELS_RGB_INTENSITY_GREEN  0.59
#define LEVELS_RGB_INTENSITY_BLUE   0.11
#define LEVELS_RGB_INTENSITY(r,g,b) ((r) * LEVELS_RGB_INTENSITY_RED   + \
                                     (g) * LEVELS_RGB_INTENSITY_GREEN + \
                                     (b) * LEVELS_RGB_INTENSITY_BLUE)

#include "imagelevels.h"

// Qt includes

#include <QFile>

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Local includes

#include "digikam_debug.h"
#include "imagehistogram.h"
#include "digikam_globals.h"

namespace Digikam
{

class ImageLevels::Private
{

public:

    enum PixelType
    {
        RedPixel = 0,
        GreenPixel,
        BluePixel,
        AlphaPixel
    };

    struct _Levels
    {
        double gamma[5];

        int    low_input[5];
        int    high_input[5];

        int    low_output[5];
        int    high_output[5];
    };

    struct _Lut
    {
        unsigned short** luts;
        int              nchannels;
    };

public:

    Private()
    {
        levels     = 0;
        lut        = 0;
        dirty      = false;
        sixteenBit = false;
    }

    // Levels data.
    struct _Levels* levels;

    // Lut data.
    struct _Lut*    lut;

    bool            sixteenBit;
    bool            dirty;
};

ImageLevels::ImageLevels(bool sixteenBit)
    : d(new Private)
{
    d->lut            = new Private::_Lut;
    d->levels         = new Private::_Levels;
    d->sixteenBit     = sixteenBit;

    memset(d->levels, 0, sizeof(struct Private::_Levels));
    d->lut->luts      = NULL;
    d->lut->nchannels = 0;

    reset();
}

ImageLevels::~ImageLevels()
{
    if (d->lut)
    {
        if (d->lut->luts)
        {
            for (int i = 0 ; i < d->lut->nchannels ; ++i)
            {
                delete [] d->lut->luts[i];
            }

            delete [] d->lut->luts;
        }

        delete d->lut;
    }

    if (d->levels)
    {
        delete d->levels;
    }

    delete d;
}

bool ImageLevels::isDirty()
{
    return d->dirty;
}

bool ImageLevels::isSixteenBits()
{
    return d->sixteenBit;
}

void ImageLevels::reset()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
    {
        levelsChannelReset(channel);
    }
}

void ImageLevels::levelsChannelReset(int channel)
{
    if (!d->levels)
    {
        return;
    }

    d->levels->gamma[channel]       = 1.0;
    d->levels->low_input[channel]   = 0;
    d->levels->high_input[channel]  = d->sixteenBit ? 65535 : 255;
    d->levels->low_output[channel]  = 0;
    d->levels->high_output[channel] = d->sixteenBit ? 65535 : 255;
    d->dirty = false;
}

void ImageLevels::levelsAuto(ImageHistogram* const hist)
{
    if (!d->levels || !hist)
    {
        return;
    }

    levelsChannelReset(LuminosityChannel);

    for (int channel = RedChannel ; channel <= BlueChannel ; ++channel)
    {
        levelsChannelAuto(hist, channel);
    }

    d->dirty = true;
}

void ImageLevels::levelsChannelAuto(ImageHistogram* const hist, int channel)
{
    if (!d->levels || !hist)
    {
        return;
    }

    d->levels->gamma[channel]       = 1.0;
    d->levels->low_output[channel]  = 0;
    d->levels->high_output[channel] = d->sixteenBit ? 65535 : 255;
    double count                    = hist->getCount(channel, 0, d->sixteenBit ? 65535 : 255);

    if (count == 0.0)
    {
        d->levels->low_input[channel]  = 0;
        d->levels->high_input[channel] = 0;
    }
    else
    {
        //  Set the low input

        double new_count       = 0.0;
        double percentage      = 0.0;
        double next_percentage = 0.0;
        int i;

        for (i = 0 ; i < (d->sixteenBit ? 65535 : 255) ; ++i)
        {
            new_count       += hist->getValue(channel, i);
            percentage      = new_count / count;
            next_percentage = (new_count + hist->getValue(channel, i + 1)) / count;

            if (fabs(percentage - 0.006) < fabs(next_percentage - 0.006))
            {
                d->levels->low_input[channel] = i + 1;
                break;
            }
        }

        //  Set the high input

        new_count = 0.0;

        for (i = (d->sixteenBit ? 65535 : 255) ; i > 0 ; --i)
        {
            new_count       += hist->getValue(channel, i);
            percentage      = new_count / count;
            next_percentage = (new_count + hist->getValue(channel, i - 1)) / count;

            if (fabs(percentage - 0.006) < fabs(next_percentage - 0.006))
            {
                d->levels->high_input[channel] = i - 1;
                break;
            }
        }
    }

    d->dirty = true;
}

int ImageLevels::levelsInputFromColor(int channel, const DColor& color)
{
    switch (channel)
    {
        case LuminosityChannel:
            return qMax(qMax(color.red(), color.green()), color.blue());

        case RedChannel:
            return color.red();

        case GreenChannel:
            return color.green();

        case BlueChannel:
            return color.blue();
    }

    return 0;  // just to please the compiler.
}

void ImageLevels::levelsBlackToneAdjustByColors(int channel, const DColor& color)
{
    if (!d->levels)
    {
        return;
    }

    d->levels->low_input[channel] = levelsInputFromColor(channel, color);
    d->dirty = true;
}

void ImageLevels::levelsWhiteToneAdjustByColors(int channel, const DColor& color)
{
    if (!d->levels)
    {
        return;
    }

    d->levels->high_input[channel] = levelsInputFromColor(channel, color);
    d->dirty = true;
}

void ImageLevels::levelsGrayToneAdjustByColors(int channel, const DColor& color)
{
    if (!d->levels)
    {
        return;
    }

    int            input;
    int            range;
    double         inten;
    double         out_light;
    unsigned short lightness;

    // Calculate lightness value.

    lightness = (unsigned short)LEVELS_RGB_INTENSITY(color.red(), color.green(), color.blue());
    input     = levelsInputFromColor(channel, color);
    range     = d->levels->high_input[channel] - d->levels->low_input[channel];

    if (range <= 0)
    {
        return;
    }

    input -= d->levels->low_input[channel];

    if (input < 0)
    {
        return;
    }

    // Normalize input and lightness.

    inten     = (double) input     / (double) range;
    out_light = (double) lightness / (double) range;

    if (out_light <= 0)
    {
        return;
    }

    // Map selected color to corresponding lightness.

    d->levels->gamma[channel] = log(inten) / log(out_light);
    d->dirty = true;
}

void ImageLevels::levelsCalculateTransfers()
{
    double inten;
    int    i, j;

    if (!d->levels)
    {
        return;
    }

    // Recalculate the levels arrays.

    for (j = 0 ; j < 5 ; ++j)
    {
        for (i = 0; i <= (d->sixteenBit ? 65535 : 255); ++i)
        {
            //  determine input intensity.

            if (d->levels->high_input[j] != d->levels->low_input[j])
            {
                inten = ((double)(i - d->levels->low_input[j]) /
                         (double)(d->levels->high_input[j] - d->levels->low_input[j]));
            }
            else
            {
                inten = (double)(i - d->levels->low_input[j]);
            }

            inten = CLAMP(inten, 0.0, 1.0);

            if (d->levels->gamma[j] != 0.0)
            {
                inten = pow(inten, (1.0 / d->levels->gamma[j]));
            }
        }
    }
}

float ImageLevels::levelsLutFunc(int n_channels, int channel, float value)
{
    double inten;
    int    j;

    if (!d->levels)
    {
        return 0.0;
    }

    if (n_channels == 1)
    {
        j = 0;
    }
    else
    {
        j = channel + 1;
    }

    inten = value;

    // For color  images this runs through the loop with j = channel +1
    // the first time and j = 0 the second time.
    //
    // For bw images this runs through the loop with j = 0 the first and
    // only time.

    for (; j >= 0 ; j -= (channel + 1))
    {
        // Don't apply the overall curve to the alpha channel.

        if (j == 0 && (n_channels == 2 || n_channels == 4) && channel == n_channels - 1)
        {
            return inten;
        }

        //  Determine input intensity.

        if (d->levels->high_input[j] != d->levels->low_input[j])
            inten = ((double)((float)(d->sixteenBit ? 65535 : 255) * inten - d->levels->low_input[j]) /
                     (double)(d->levels->high_input[j] - d->levels->low_input[j]));
        else
        {
            inten = (double)((float)(d->sixteenBit ? 65535 : 255) * inten - d->levels->low_input[j]);
        }

        if (d->levels->gamma[j] != 0.0)
        {
            if (inten >= 0.0)
            {
                inten =  pow(inten, (1.0 / d->levels->gamma[j]));
            }
            else
            {
                inten = -pow(-inten, (1.0 / d->levels->gamma[j]));
            }
        }

        //  determine the output intensity.

        if (d->levels->high_output[j] >= d->levels->low_output[j])
            inten = (double)(inten * (d->levels->high_output[j] -
                                      d->levels->low_output[j]) + d->levels->low_output[j]);

        else if (d->levels->high_output[j] < d->levels->low_output[j])
            inten = (double)(d->levels->low_output[j] - inten *
                             (d->levels->low_output[j] - d->levels->high_output[j]));

        inten /= (float)(d->sixteenBit ? 65535 : 255);
    }

    return inten;
}

void ImageLevels::levelsLutSetup(int nchannels)
{
    int    i;
    uint   v;
    double val;

    if (d->lut->luts)
    {
        for (i = 0 ; i < d->lut->nchannels ; ++i)
        {
            delete [] d->lut->luts[i];
        }

        delete [] d->lut->luts;
    }

    d->lut->nchannels = nchannels;
    d->lut->luts      = new unsigned short*[d->lut->nchannels];

    for (i = 0 ; i < d->lut->nchannels ; ++i)
    {
        d->lut->luts[i] = new unsigned short[(d->sixteenBit ? 65535 : 255) + 1];

        for (v = 0 ; v <= (uint)(d->sixteenBit ? 65535 : 255) ; ++v)
        {
            // to add gamma correction use func(v ^ g) ^ 1/g instead.

            val = (float)(d->sixteenBit ? 65535 : 255) *
                  levelsLutFunc(d->lut->nchannels, i, v / (float)(d->sixteenBit ? 65535 : 255)) + 0.5;

            d->lut->luts[i][v] = (unsigned short)CLAMP(val, 0.0, (d->sixteenBit ? 65535.0 : 255.0));
        }
    }
}

void ImageLevels::levelsLutProcess(uchar* const srcPR, uchar* const destPR, int w, int h)
{
    unsigned short* lut0 = NULL, *lut1 = NULL, *lut2 = NULL, *lut3 = NULL;

    int   i;

    if (d->lut->nchannels > 0)
    {
        lut0 = d->lut->luts[0];
    }

    if (d->lut->nchannels > 1)
    {
        lut1 = d->lut->luts[1];
    }

    if (d->lut->nchannels > 2)
    {
        lut2 = d->lut->luts[2];
    }

    if (d->lut->nchannels > 3)
    {
        lut3 = d->lut->luts[3];
    }

    if (!d->sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar* ptr = srcPR;
        uchar* dst = destPR;

        for (i = 0 ; i < w * h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (d->lut->nchannels > 0)
            {
                red = lut0[red];
            }

            if (d->lut->nchannels > 1)
            {
                green = lut1[green];
            }

            if (d->lut->nchannels > 2)
            {
                blue = lut2[blue];
            }

            if (d->lut->nchannels > 3)
            {
                alpha = lut3[alpha];
            }

            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(srcPR);
        unsigned short* dst = reinterpret_cast<unsigned short*>(destPR);

        for (i = 0 ; i < w * h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (d->lut->nchannels > 0)
            {
                red = lut0[red];
            }

            if (d->lut->nchannels > 1)
            {
                green = lut1[green];
            }

            if (d->lut->nchannels > 2)
            {
                blue = lut2[blue];
            }

            if (d->lut->nchannels > 3)
            {
                alpha = lut3[alpha];
            }

            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
}

void ImageLevels::setLevelGammaValue(int channel, double val)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        d->levels->gamma[channel] = val;
        d->dirty = true;
    }
}

void ImageLevels::setLevelLowInputValue(int channel, int val)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        d->levels->low_input[channel] = val;
        d->dirty = true;
    }
}

void ImageLevels::setLevelHighInputValue(int channel, int val)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        d->levels->high_input[channel] = val;
        d->dirty = true;
    }
}

void ImageLevels::setLevelLowOutputValue(int channel, int val)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        d->levels->low_output[channel] = val;
        d->dirty = true;
    }
}

void ImageLevels::setLevelHighOutputValue(int channel, int val)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        d->levels->high_output[channel] = val;
        d->dirty = true;
    }
}

double ImageLevels::getLevelGammaValue(int channel)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        return (d->levels->gamma[channel]);
    }

    return 0.0;
}

int ImageLevels::getLevelLowInputValue(int channel)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        return (d->levels->low_input[channel]);
    }

    return 0;
}

int ImageLevels::getLevelHighInputValue(int channel)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        return (d->levels->high_input[channel]);
    }

    return 0;
}

int ImageLevels::getLevelLowOutputValue(int channel)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        return (d->levels->low_output[channel]);
    }

    return 0;
}

int ImageLevels::getLevelHighOutputValue(int channel)
{
    if (d->levels && channel >= 0 && channel < 5)
    {
        return (d->levels->high_output[channel]);
    }

    return 0;
}

bool ImageLevels::loadLevelsFromGimpLevelsFile(const QUrl& fileUrl)
{
    // TODO : support QUrl !

    FILE*   file = 0;
    int     low_input[5];
    int     high_input[5];
    int     low_output[5];
    int     high_output[5];
    double  gamma[5];
    int     i, fields;
    char    buf[50];
    char*   nptr = 0;

    file = fopen(QFile::encodeName(fileUrl.toLocalFile()).constData(), "r");

    if (!file)
    {
        return false;
    }

    if (! fgets(buf, sizeof(buf), file))
    {
        fclose(file);
        return false;
    }

    if (strcmp(buf, "# GIMP Levels File\n") != 0)
    {
        fclose(file);
        return false;
    }

    for (i = 0 ; i < 5 ; ++i)
    {
        // FIXME: scanf without field width limits can crash with huge input data
        fields = fscanf(file, "%d %d %d %d ",
                        &low_input[i],
                        &high_input[i],
                        &low_output[i],
                        &high_output[i]);

        if (fields != 4)
        {
            qCWarning(DIGIKAM_DIMG_LOG) <<  "Invalid Gimp levels file!";
            fclose(file);
            return false;
        }

        if (!fgets(buf, 50, file))
        {
            qCWarning(DIGIKAM_DIMG_LOG) <<  "Invalid Gimp levels file!";
            fclose(file);
            return false;
        }

        gamma[i] = strtod(buf, &nptr);

        if (buf == nptr || errno == ERANGE)
        {
            qCWarning(DIGIKAM_DIMG_LOG) <<  "Invalid Gimp levels file!";
            fclose(file);
            return false;
        }
    }

    for (i = 0 ; i < 5 ; ++i)
    {
        setLevelGammaValue(i, gamma[i]);
        setLevelLowInputValue(i,   d->sixteenBit ? low_input[i]   * 255 : low_input[i]);
        setLevelHighInputValue(i,  d->sixteenBit ? high_input[i]  * 255 : high_input[i]);
        setLevelLowOutputValue(i,  d->sixteenBit ? low_output[i]  * 255 : low_output[i]);
        setLevelHighOutputValue(i, d->sixteenBit ? high_output[i] * 255 : high_output[i]);
    }

    fclose(file);
    return true;
}

bool ImageLevels::saveLevelsToGimpLevelsFile(const QUrl& fileUrl)
{
    // TODO : support QUrl !

    FILE* file = 0;
    int   i;

    file = fopen(QFile::encodeName(fileUrl.toLocalFile()).constData(), "w");

    if (!file)
    {
        return false;
    }

    fprintf(file, "# GIMP Levels File\n");

    for (i = 0 ; i < 5 ; ++i)
    {
        char buf[256];
        sprintf(buf, "%f", getLevelGammaValue(i));

        fprintf(file, "%d %d %d %d %s\n",
                d->sixteenBit ? getLevelLowInputValue(i)  / 255 : getLevelLowInputValue(i),
                d->sixteenBit ? getLevelHighInputValue(i) / 255 : getLevelHighInputValue(i),
                d->sixteenBit ? getLevelLowOutputValue(i) / 255 : getLevelLowOutputValue(i),
                d->sixteenBit ? getLevelHighInputValue(i) / 255 : getLevelHighInputValue(i),
                buf);
    }

    fflush(file);
    fclose(file);

    return true;
}

}  // namespace Digikam
