/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : A digital camera RAW files loader for DImg
 *               framework using an external dcraw instance.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef RAWLOADER_H
#define RAWLOADER_H

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "dimgloader.h"
#include "drawdecoding.h"
#include "digikam_export.h"
#include "iccprofile.h"
#include "rawprocessingfilter.h"

namespace Digikam
{

class DImg;
class RawProcessingFilter;

class DIGIKAM_EXPORT RAWLoader : public KDcrawIface::KDcraw, public DImgLoader
{
    Q_OBJECT

public:

    explicit RAWLoader(DImg* image, DRawDecoding rawDecodingSettings=DRawDecoding());

    bool load(const QString& filePath, DImgLoaderObserver* observer=0);
    void postProcess(DImgLoaderObserver* observer);

    // NOTE: RAW files are always Read only.
    bool save(const QString& /*filePath*/, DImgLoaderObserver* /*observer=0*/)
    {
        return false;
    };

    bool hasAlpha()   const
    {
        return false;
    };
    bool isReadOnly() const
    {
        return true;
    };
    bool sixteenBit() const
    {
        return m_rawDecodingSettings.sixteenBitsImage;
    };

    FilterAction filterAction() const;

private:

    // Methods to load RAW image using external dcraw instance.

    bool loadedFromDcraw(QByteArray data, int width, int height, int rgbmax,
                         DImgLoaderObserver* observer);

    bool checkToCancelWaitingData();
    void setWaitingDataProgress(double value);

private:

    DImgLoaderObserver*  m_observer;
    RawProcessingFilter* m_filter;
};

}  // namespace Digikam

#endif /* RAWLOADER_H */
