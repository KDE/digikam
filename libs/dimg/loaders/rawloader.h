/* ============================================================
 * Author : Gilles Caulier <caulier dot gilles at kdemail dot net> 
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using dcraw program.
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

#include <qmutex.h>
#include <qobject.h>
#include <qwaitcondition.h>

// Local includes.

#include "dimgloader.h"
#include "rawdecodingsettings.h"
#include "digikam_export.h"

class KProcess;
class QCustomEvent;
class QTimer;

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT RAWLoader : public QObject, public DImgLoader
{

    Q_OBJECT

public:

    RAWLoader(DImg* image, RawDecodingSettings rawDecodingSettings=RawDecodingSettings());

    bool load(const QString& filePath, DImgLoaderObserver *observer);
    bool save(const QString& filePath, DImgLoaderObserver *observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return true; };

private:

    bool                m_sixteenBit;
    bool                m_hasAlpha;

    RawDecodingSettings m_rawDecodingSettings;

    DImgLoaderObserver *m_observer;
    QString             m_filePath;

    KProcess           *m_process;
    bool                m_running;
    bool                m_normalExit;
    QTimer             *m_queryTimer;

    uchar              *m_data;
    int                 m_dataPos;
    int                 m_width;
    int                 m_height;
    int                 m_rgbmax;

    QMutex              m_mutex;
    QWaitCondition      m_condVar;

private:

    // Methods to load RAW image using external dcraw instance.

    bool loadFromDcraw(const QString& filePath, DImgLoaderObserver *observer);

    virtual void customEvent(QCustomEvent *);
    void startProcess();

private slots:

    void slotProcessExited(KProcess *);
    void slotReceivedStdout(KProcess *, char *buffer, int buflen);
    void slotReceivedStderr(KProcess *, char *buffer, int buflen);
    void slotContinueQuery();

};

}  // NameSpace Digikam
    
#endif /* RAWLOADER_H */
