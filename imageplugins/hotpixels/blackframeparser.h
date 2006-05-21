/* ============================================================
 * File  : blackframeparser.h
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : 
 * 
 * Copyright 2005 by Unai Garro
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
 * ============================================================ 
 * Part of the algorithm for finding the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
 * ============================================================*/


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

#include <kio/scheduler.h>  //KIO::get
#include <kio/jobclasses.h> //KIO::TransferJob
#include <kurl.h>

// Local includes.

#include "hotpixel.h"

namespace DigikamHotPixelsImagesPlugin
{

class BlackFrameParser: public QObject
{
    Q_OBJECT
    
public:
    
    BlackFrameParser();
    ~BlackFrameParser();
        
    void parseHotPixels(QString file);
    void parseBlackFrame(KURL url);
    void parseBlackFrame(QImage& img);
    QImage image(){return mImage;}
    
private:

    QString mOutputString;
    void blackFrameParsing(bool useData=false);
    void consolidatePixels (QValueList<HotPixel>& list);
    void validateAndConsolidate(HotPixel *a, HotPixel *b);
        
private:
    
    QByteArray mData;
    QImage     mImage;
        
private slots:
        
    void blackFrameDataArrived(KIO::Job*, const QByteArray& data);
    void slotResult(KIO::Job*);

signals:
    void parsed(QValueList<HotPixel>);
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif // BLACKFRAMEPARSER_H
