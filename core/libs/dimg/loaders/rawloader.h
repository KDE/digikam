/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : A digital camera RAW files loader for DImg
 *               framework using an external dcraw instance.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "drawdecoder.h"
#include "dimgloader.h"
#include "drawdecoding.h"
#include "digikam_export.h"
#include "filters/rawprocessingfilter.h"

namespace Digikam
{

class DImg;
class RawProcessingFilter;

class DIGIKAM_EXPORT RAWLoader : public DRawDecoder, public DImgLoader
{
    Q_OBJECT

public:

    explicit RAWLoader(DImg* const image, const DRawDecoding& rawDecodingSettings = DRawDecoding());

    bool load(const QString& filePath, DImgLoaderObserver* const observer = 0);
    bool save(const QString& /*filePath*/, DImgLoaderObserver* const /*observer=0*/);

    bool hasAlpha()   const;
    bool isReadOnly() const;
    bool sixteenBit() const;

    void postProcess(DImgLoaderObserver* const observer);

    FilterAction filterAction() const;

private:

    bool loadedFromRawData(const QByteArray& data, int width, int height, int rgbmax,
                           DImgLoaderObserver* const observer);

    bool checkToCancelWaitingData();
    void setWaitingDataProgress(double value);

private:

    DImgLoaderObserver*  m_observer;
    RawProcessingFilter* m_filter;
};

}  // namespace Digikam

#endif /* RAWLOADER_H */
