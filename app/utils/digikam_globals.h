/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_GLOBALS_H
#define DIGIKAM_GLOBALS_H

// Qt includes

#include <QStringList>
#include <QIODevice>
#include <QProcessEnvironment>

// Local includes

#include "digikam_export.h"

class QWidget;
class QObject;
class QShortcut;
class QKeySequence;

/** Macros for image filters.
 */
#define CLAMP0255(a)   qBound(0,a,255)
#define CLAMP065535(a) qBound(0,a,65535)
#define CLAMP(x,l,u)   qBound(l,x,u)
#define MAX3(a, b, c)  (qMax(qMax(a,b),c))
#define MIN3(a, b, c)  (qMin(qMin(a,b),c))

/** Degrees to radian conversion coeff (PI/180). To optimize computation.
 */
#define DEG2RAD 0.017453292519943

namespace Digikam
{

/** Field value limits for all digiKam-specific fields (not EXIF/IPTC fields)
 */
static const int RatingMin          = 0;
static const int RatingMax          = 5;
static const int NoRating           = -1;

// --------------------------------------------------------

/** segments for histograms and curves
 */
static const int NUM_SEGMENTS_16BIT = 65536;
static const int NUM_SEGMENTS_8BIT  = 256;
static const int MAX_SEGMENT_16BIT  = NUM_SEGMENTS_16BIT - 1;
static const int MAX_SEGMENT_8BIT   = NUM_SEGMENTS_8BIT  - 1;

// --------------------------------------------------------

/** Delay in milliseconds to automatically expands album tree-view with D&D
 *  See bug #286263 for details.
 */
static const int AUTOEXPANDDELAY    = 800;

// --------------------------------------------------------

enum ColorLabel
{
    NoColorLabel        = 0,
    RedLabel,
    OrangeLabel,
    YellowLabel,
    GreenLabel,
    BlueLabel,
    MagentaLabel,
    GrayLabel,
    BlackLabel,
    WhiteLabel,
    FirstColorLabel     = NoColorLabel,
    LastColorLabel      = WhiteLabel,
    NumberOfColorLabels = LastColorLabel + 1
};

// --------------------------------------------------------

enum PickLabel
{
    NoPickLabel        = 0,
    RejectedLabel,
    PendingLabel,
    AcceptedLabel,
    FirstPickLabel     = NoPickLabel,
    LastPickLabel      = AcceptedLabel,
    NumberOfPickLabels = LastPickLabel + 1
};

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

// --------------------------------------------------------

/** Convenience method for creating keyboard shortcuts.
 */
DIGIKAM_EXPORT QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot);

/** Return list of supported image formats by Qt for reading or writing operations if suitable
 *  container used by QFileDialog.
 *  For simple container of type mime, use 'allTypes' string.
 *  Supported modes are QIODevice::ReadOnly, QIODevice::WriteOnly, and QIODevice::ReadWrite.
 */
DIGIKAM_EXPORT QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode, QString& allTypes);

/** Show a dialog with all RAW camera supported by digiKam, through libraw.
 */
DIGIKAM_EXPORT void showRawCameraList();

/** If digiKam run into AppImage, return a cleaned environment for QProcess to execute a
 *  program outside the bundle without broken run-time dependencies.
 *  Use case : system based Hugin CLI tools called by Panorama wizard.
 *  If digiKam do not run as AppImage bundle, this method return a QProcessEnvironment instance
 *  based on system environment.
 */
DIGIKAM_EXPORT QProcessEnvironment adjustedEnvironmentForAppImage();

} // namespace Digikam

#endif // DIGIKAM_GLOBALS_H
