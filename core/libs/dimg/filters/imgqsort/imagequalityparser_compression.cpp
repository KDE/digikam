/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - compression detector.
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

namespace Digikam
{

int ImageQualityParser::compressionDetector() const
{
    // FIXME: set threshold value to an acceptable standard to get the number of blocking artifacts
    const int THRESHOLD  = 30;
    const int block_size = 8;
    int countblocks      = 0;
    int number_of_blocks = 0;
    float sum            = 0.0;

    QList<float> average_bottom;
    QList<float> average_middle;
    QList<float> average_top;
    DColor col;

    // Go through 8 blocks at a time horizontally
    // iterating through columns.

    for (uint i = 0 ; d->running && (i < d->img8.height()) ; ++i)
    {
        // Calculating intensity of top column.

        for (uint j = 0 ; j < d->img8.width() ; j += 8)
        {
            sum = 0.0;

            for (int k = j ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i, j);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_top.push_back(sum / 8.0);
        }

        // Calculating intensity of middle column.

        for (uint j = 0 ; j < d->img8.width() ; j += 8)
        {
            sum = 0.0;

            for (uint k = j ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i + 1, j);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_middle.push_back(sum / 8.0);
        }

        // Calculating intensity of bottom column.

        countblocks = 0;

        for (uint j = 0 ; j < d->img8.width() ; j += 8)
        {
            sum = 0.0;

            for (uint k = j ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i + 2, j);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_bottom.push_back(sum / 8.0);
            ++countblocks;
        }

        // Check if the average intensity of 8 blocks in the top, middle and bottom rows are equal.
        // If so increment number_of_blocks.

        for (int j = 0 ; j < countblocks ; ++j)
        {
            if ((average_middle[j] == (average_top[j] + average_bottom[j]) / 2.0) &&
                average_middle[j] > THRESHOLD)
            {
                ++number_of_blocks;
            }
        }
    }

    average_bottom.clear();
    average_middle.clear();
    average_top.clear();

    // Iterating through rows.

    for (uint j = 0 ; d->running && (j < d->img8.width()) ; ++j)
    {
        // Calculating intensity of top row.

        for (uint i = 0 ; i < d->img8.height() ; i += 8)
        {
            sum = 0.0;

            for (int k = i ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i, j);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_top.push_back(sum / 8.0);
        }

        // Calculating intensity of middle row.

        for (uint i = 0 ; i < d->img8.height() ; i += 8)
        {
            sum = 0.0;

            for (uint k = i ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i, j + 1);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_middle.push_back(sum / 8.0);
        }

        // Calculating intensity of bottom row.

        countblocks = 0;

        for (uint i = 0 ; i < d->img8.height() ; i += 8)
        {
            sum = 0.0;

            for (uint k = i ; k < block_size ; ++k)
            {
                col  = d->img8.getPixelColor(i, j + 2);
                sum += (col.red() + col.green() + col.blue()) / 3.0;
            }

            average_bottom.push_back(sum / 8.0);
            ++countblocks;
        }

        // Check if the average intensity of 8 blocks in the top, middle and bottom rows are equal.
        // If so increment number_of_blocks.

        for (int i = 0 ; i < countblocks ; ++i)
        {
            if ((average_middle[i] == (average_top[i] + average_bottom[i]) / 2.0) &&
                average_middle[i] > THRESHOLD)
            {
                ++number_of_blocks;
            }
        }
    }

    return number_of_blocks;
}

} // namespace Digikam
