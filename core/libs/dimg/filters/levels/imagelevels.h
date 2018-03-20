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

#ifndef IMAGELEVELS_H
#define IMAGELEVELS_H

// Qt includes

#include <QUrl>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageHistogram;

class DIGIKAM_EXPORT ImageLevels
{

public:

    explicit ImageLevels(bool sixteenBit);
    ~ImageLevels();

    bool   isDirty();
    bool   isSixteenBits();
    void   reset();

    // Methods for to manipulate the levels data.

    void   levelsChannelReset(int channel);
    void   levelsAuto(ImageHistogram* const hist);
    void   levelsChannelAuto(ImageHistogram* const hist, int channel);
    int    levelsInputFromColor(int channel, const DColor& color);
    void   levelsBlackToneAdjustByColors(int channel, const DColor& color);
    void   levelsGrayToneAdjustByColors(int channel, const DColor& color);
    void   levelsWhiteToneAdjustByColors(int channel, const DColor& color);
    void   levelsCalculateTransfers();
    float  levelsLutFunc(int nchannels, int channel, float value);
    void   levelsLutSetup(int nchannels);
    void   levelsLutProcess(uchar* const srcPR, uchar* const destPR, int w, int h);

    // Methods for to set manually the levels values.

    void   setLevelGammaValue(int channel, double val);
    void   setLevelLowInputValue(int channel, int val);
    void   setLevelHighInputValue(int channel, int val);
    void   setLevelLowOutputValue(int channel, int val);
    void   setLevelHighOutputValue(int channel, int val);

    double getLevelGammaValue(int channel);
    int    getLevelLowInputValue(int channel);
    int    getLevelHighInputValue(int channel);
    int    getLevelLowOutputValue(int channel);
    int    getLevelHighOutputValue(int channel);

    // Methods for to save/load the levels values to/from a Gimp levels text file.

    bool   saveLevelsToGimpLevelsFile(const QUrl& fileUrl);
    bool   loadLevelsFromGimpLevelsFile(const QUrl& fileUrl);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGELEVELS_H */
