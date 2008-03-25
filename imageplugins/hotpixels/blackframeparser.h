/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : black frames parser
 * 
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * 
 * Part of the algorithm for finding the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
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

#ifndef BLACKFRAMEPARSER_H
#define BLACKFRAMEPARSER_H

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Qt includes.

#include <qimage.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qrect.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "dimg.h"
#include "loadsavethread.h"
#include "hotpixel.h"

using namespace Digikam;

namespace DigikamHotPixelsImagesPlugin
{

class BlackFrameParser: public QObject
{
    Q_OBJECT

public:

    BlackFrameParser(QObject *parent);
    ~BlackFrameParser();

    void parseHotPixels(const QString &file);
    void parseBlackFrame(const KURL &url);
    void parseBlackFrame(QImage& img);
    QImage image(){return m_Image;}

signals:

    void parsed(QValueList<HotPixel>);
    void signalLoadingProgress(float);
    void signalLoadingComplete();

private slots:

    void slotLoadingProgress(const LoadingDescription&, float);
    void slotLoadImageFromUrlComplete(const LoadingDescription&, const DImg&);
    
private:

    void blackFrameParsing();
    void consolidatePixels(QValueList<HotPixel>& list);
    void validateAndConsolidate(HotPixel *a, HotPixel *b);

private:

    QString         m_OutputString;
    QString         m_localFile;

    QImage          m_Image;

    LoadSaveThread *m_imageLoaderThread;
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif // BLACKFRAMEPARSER_H
