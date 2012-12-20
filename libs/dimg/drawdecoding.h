/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard libkdcraw parameters plus
 *               few customized for post processing.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DRAW_DECODING_H
#define DRAW_DECODING_H

// Qt includes

#include <QList>
#include <QMetaType>
#include <QPolygon>
#include <QDomElement>

// LibKDcraw includes

#include <libkdcraw/rawdecodingsettings.h>
#include <libkdcraw/version.h>

// Local includes

#include "digikam_export.h"
#include "bcgcontainer.h"
#include "wbcontainer.h"
#include "curvescontainer.h"

using namespace KDcrawIface;

namespace Digikam
{

class DIGIKAM_EXPORT DRawDecoding
{

public:

    /** Standard constructor with default settings
     */
    DRawDecoding();

    /** Copy constructor. Creates a copy of a RawDecodingSettings object.
     */
    explicit DRawDecoding(const RawDecodingSettings& prm);

    /** Standard destructor
     */
    virtual ~DRawDecoding();

    /** Method to use a settings to optimize time loading, for example to compute image histogram
     */
    void optimizeTimeLoading();

    /** Method to reset to default values all Raw processing settings.
     */
    void resetPostProcessingSettings();

    /** Method to check is a post-processing setting have been changed
     */
    bool postProcessingSettingsIsDirty() const;

    /** Equality operator.
     */
    bool operator==(const DRawDecoding& other) const;

    void writeToFilterAction(FilterAction& action, const QString& prefix = QString()) const;

public:

    static DRawDecoding fromFilterAction(const FilterAction& action, const QString& prefix = QString());

    /** Used by BQM to read/store Queue Raw decoding settings from/to configuration file
     */
    static void decodingSettingsToXml(const RawDecodingSettings& prm, QDomElement& elm);
    static void decodingSettingsFromXml(const QDomElement& elm, RawDecodingSettings& prm);

public:

    /** All Raw decoding settings provided by libkdcraw.
     */
    RawDecodingSettings rawPrm;

    /// Post Processing settings ----------------------------------------------------

    /** BCG correction values.
    */
    BCGContainer        bcg;

    /** White Balance correction values.
    */
    WBContainer         wb;

    /** Curve adjustments.
    */
    CurvesContainer     curvesAdjust;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DRawDecoding)

#endif /* DRAW_DECODING_H */
