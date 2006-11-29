/* ============================================================
 * Authors : Gilles Caulier <caulier dot gilles at kdemail dot net> 
 *           Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date    : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using an external dcraw instance.
 * 
 * Copyright 2005-2006 by Gilles Caulier and Marcel Wiesweg
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

// Qt includes

#include <qobject.h>

// Local includes.

#include "dimgloader.h"
#include "rawdecodingsettings.h"
#include "digikam_export.h"

class KProcess;

namespace Digikam
{

class DImg;
class RAWLoaderPriv;

class DIGIKAM_EXPORT RAWLoader : public QObject, public DImgLoader
{

    Q_OBJECT

public:

    RAWLoader(DImg* image, RawDecodingSettings rawDecodingSettings=RawDecodingSettings());
    ~RAWLoader();

    bool load(const QString& filePath, DImgLoaderObserver *observer=0);
    bool save(const QString& filePath, DImgLoaderObserver *observer=0);

    virtual bool hasAlpha()   const { return false; };
    virtual bool isReadOnly() const { return true;  };
    virtual bool sixteenBit() const;

private:

    // Methods to load RAW image using external dcraw instance.

    bool loadFromDcraw(const QString& filePath, DImgLoaderObserver *observer=0);

    virtual void customEvent(QCustomEvent *);
    void startProcess();

private slots:

    void slotProcessExited(KProcess *);
    void slotReceivedStdout(KProcess *, char *buffer, int buflen);
    void slotReceivedStderr(KProcess *, char *buffer, int buflen);
    void slotContinueQuery();

private:

    RAWLoaderPriv *d;

};

}  // NameSpace Digikam
    
#endif /* RAWLOADER_H */
