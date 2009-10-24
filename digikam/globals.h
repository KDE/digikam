/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global variables and flags for the digiKam project
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef DIGIKAMGLOBALS_H
#define DIGIKAMGLOBALS_H

namespace Digikam
{

// Field value limits for all Digikam-specific fields (not EXIF/IPTC fields)

static const int RatingMin = 0;
static const int RatingMax = 5;
static const int NoRating  = -1;

// --------------------------------------------------------

// segments for histograms and curves
static const int NUM_SEGMENTS_16BIT = 65536;
static const int NUM_SEGMENTS_8BIT = 256;
static const int MAX_SEGMENT_16BIT = NUM_SEGMENTS_16BIT - 1;
static const int MAX_SEGMENT_8BIT = NUM_SEGMENTS_8BIT - 1;

// --------------------------------------------------------

enum HistogramBoxType
{
    RGB = 0,
    RGBA,
    LRGB,
    LRGBA,
    LRGBC,
    LRGBAC
};

enum HistogramScale
{
    LinScaleHistogram = 0,      // Linear scale
    LogScaleHistogram           // Logarithmic scale
};

enum HistogramRenderingType
{
    FullImageHistogram = 0,     // Full image histogram rendering.
    ImageSelectionHistogram     // Image selection histogram rendering.
};

// --------------------------------------------------------

enum ChannelType
{
    LuminosityChannel = 0,
    RedChannel,
    GreenChannel,
    BlueChannel,
    AlphaChannel,
    ColorChannels
};

enum ColorChannelsType
{
    ColorChannelsRed = 0,      // Red color to foreground in All Colors Channel mode.
    ColorChannelsGreen,        // Green color to foreground in All Colors Channel mode.
    ColorChannelsBlue          // Blue color to foreground in All Colors Channel mode.
};

} // namespace Digikam

#endif /* DIGIKAMGLOBALS_H */
