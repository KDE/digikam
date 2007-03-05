/* ============================================================
 * Author : Gilles Caulier <caulier dot gilles at gmail dot com> 
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using an external dcraw instance.
 * 
 * Copyright 2005-2007 by Gilles Caulier and Marcel Wiesweg
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

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rawdecodingsettings.h>

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT RAWLoader : public KDcrawIface::KDcraw, public DImgLoader
{
    Q_OBJECT

public:

    RAWLoader(DImg* image, KDcrawIface::RawDecodingSettings rawDecodingSettings=KDcrawIface::RawDecodingSettings());

    bool load(const QString& filePath, DImgLoaderObserver *observer=0);

    //RAW files are always Read only.
    bool save(const QString& /*filePath*/, DImgLoaderObserver */*observer=0*/) { return false; };

    bool hasAlpha()   const { return false;                                  };
    bool isReadOnly() const { return true;                                   };
    bool sixteenBit() const { return m_rawDecodingSettings.sixteenBitsImage; };

private:

    // Methods to load RAW image using external dcraw instance.

    bool loadedFromDcraw(QByteArray data, int width, int height, int rgbmax,
                         DImgLoaderObserver *observer);

    bool checkToCancelWaitingData();
    bool checkToCancelRecievingData();

    void setWaitingDataProgress(double value);
    void setRecievingDataProgress(double value);

private:

    DImgLoaderObserver *m_observer;
};

}  // NameSpace Digikam
    
#endif /* RAWLOADER_H */
