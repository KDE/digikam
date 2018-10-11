/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "imagequalityparser_p.h"

ImageQualityParser::ImageQualityParser(const DImg& image,
                                       const ImageQualityContainer& settings,
                                       PickLabel* const label)
    : d(new Private)
{
    d->imq     = settings;
    d->image   = image;
    d->neimage = image;
    d->label   = label;
}

ImageQualityParser::~ImageQualityParser()
{
    delete d;
}

void ImageQualityParser::readImage() const
{
    DColor col;
    int j = 0;

    d->img8 = d->image;
    d->img8.convertToEightBit();                        // Convert to 8 bits color depth.

    // grayscale image creation for noise detector

    d->src_gray = Mat(d->img8.numPixels(), 1, CV_8UC1); // Create a matrix containing the pixel values of grayscaled image

    for (uint x = 0 ; d->running && (x < d->img8.width()) ; ++x)
    {
        for (uint y = 0 ; d->running && (y < d->img8.height()) ; ++y)
        {
            col                         = d->img8.getPixelColor(x, y);
            d->src_gray.at<uchar>(x, y) = (col.red() + col.green() + col.blue()) / 3;
        }
    }

    // For Noise detection

    if (d->imq.detectNoise)
    {
        for (int c = 0 ; d->running && (c < 3) ; ++c)
        {
            d->fimg[c] = new float[d->neimage.numPixels()];
        }

        j = 0;

        for (uint y = 0 ; d->running && (y < d->neimage.height()) ; ++y)
        {
            for (uint x = 0 ; d->running && (x < d->neimage.width()) ; ++x)
            {
                col           = d->neimage.getPixelColor(x, y);
                d->fimg[0][j] = col.red();
                d->fimg[1][j] = col.green();
                d->fimg[2][j] = col.blue();
                ++j;
            }
        }
    }
}

void ImageQualityParser::startAnalyse()
{
    // For Noise Estimation
    // Use the Top/Left corner of 256x256 pixels to analyze noise contents from image.
    // This will speed-up computation time with OpenCV.

    readImage();

    double blur             = 0.0;
    short  blur2            = 0;
    double noise            = 0.0;
    int    compressionLevel = 0;
    double finalQuality     = 0.0;
    double exposureLevel    = 0.0;

    // If blur option is selected in settings, run the blur detection algorithms
    if (d->running && d->imq.detectBlur)
    {
        // Returns blur value between 0 and 1.
        // If NaN is returned just assign NoPickLabel
        blur  = blurDetector();
        qCDebug(DIGIKAM_DIMG_LOG) << "Amount of Blur present in image is:" << blur;

        // Returns blur value between 1 and 32767.
        // If 1 is returned just assign NoPickLabel
        blur2 = blurDetector2();
        qCDebug(DIGIKAM_DIMG_LOG) << "Amount of Blur present in image [using LoG Filter] is:" << blur2;
    }

    if (d->running && d->imq.detectNoise)
    {
        // Some images give very low noise value. Assign NoPickLabel in that case.
        // Returns noise value between 0 and 1.
        noise = noiseDetector();
        qCDebug(DIGIKAM_DIMG_LOG) << "Amount of Noise present in image is:" << noise;
    }

    if (d->running && d->imq.detectCompression)
    {
        // Returns number of blocks in the image.
        compressionLevel = compressionDetector();
        qCDebug(DIGIKAM_DIMG_LOG) << "Amount of compression artifacts present in image is:" << compressionLevel;
    }

    if (d->running && d->imq.detectOverexposure)
    {
        // Returns percents of over-exposure in the image
        exposureLevel = exposureAmount();
        qCDebug(DIGIKAM_DIMG_LOG) << "Over-exposure percents in image is: " << exposureLevel;
    }

#ifdef TRACE

    QFile filems("imgqsortresult.txt");

    if (filems.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream oms(&filems);
        oms << "File:" << d->image.originalFilePath() << endl;

        if (d->imq.detectBlur)
        {
            oms << "Blur Present:" << blur << endl;
            oms << "Blur Present(using LoG filter):"<< blur2 << endl;
        }

        if (d->imq.detectNoise)
        {
            oms << "Noise Present:" << noise << endl;
        }

        if (d->imq.detectCompression)
        {
            oms << "Compression Present:" << compressionLevel << endl;
        }

        if (d->imq.detectOverexposure)
        {
            oms << "Over-exposure Percents:" << exposureLevel << endl;
        }
    }

#endif // TRACE

    // Calculating finalquality

    // All the results to have a range of 1 to 100.
    if (d->running)
    {
        double finalBlur         = (blur * 100.0)  + ((blur2 / 32767) * 100.0);
        double finalNoise        = noise * 100.0;
        double finalCompression  = (compressionLevel / 1024.0) * 100.0; // we are processing 1024 pixels size image
        double finalOverExposure = 100.0 - (exposureLevel * 100.0);

        finalQuality            = finalBlur         * d->imq.blurWeight        +
                                  finalNoise        * d->imq.noiseWeight       +
                                  finalCompression  * d->imq.compressionWeight +
                                  finalOverExposure * 100.0;

        // FIXME: the over-eposure detection is not handle here!

        finalQuality           = finalQuality / 100.0;

        qCDebug(DIGIKAM_DIMG_LOG) << "Final Quality estimated: " << finalQuality;

        // Assigning PickLabels

        if (finalQuality == 0.0)
        {
            // Algorithms have not been run. So return noPickLabel
            *d->label = NoPickLabel;
        }
        else if ((int)finalQuality < d->imq.rejectedThreshold)
        {
            *d->label = RejectedLabel;
        }
        else if (((int)finalQuality > d->imq.rejectedThreshold) &&
                 ((int)finalQuality < d->imq.acceptedThreshold))
        {
            *d->label = PendingLabel;
        }
        else
        {
            *d->label = AcceptedLabel;
        }
    }
    else
    {
        *d->label = NoPickLabel;
    }
}

void ImageQualityParser::cancelAnalyse()
{
    d->running = false;
}

} // namespace Digikam
