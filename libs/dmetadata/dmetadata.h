/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DMETADATA_H
#define DMETADATA_H

// Qt includes.

#include <qstring.h>

// LibKExiv2 includes.

#include <libkexiv2/kexiv2.h>

// Local includes.

#include "dimg.h"
#include "photoinfocontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetadata : public KExiv2Iface::KExiv2
{

public:

    DMetadata();
    DMetadata(const QString& filePath);
    ~DMetadata();

    /** Re-implemented from libKexiv2 to use dcraw identify method if Exiv2 failed. */
    bool load(const QString& filePath);

    /** Try to extract metadata using dcraw identify method */
    bool loadUsingDcraw(const QString& filePath);
    
    /** Metadata manipulation methods */

    QString getImageComment() const;
    bool    setImageComment(const QString& comment);

    int  getImageRating() const;
    bool setImageRating(int rating);

    bool setImagePhotographerId(const QString& author, const QString& authorTitle);
    bool setImageCredits(const QString& credit, const QString& source, const QString& copyright);

    PhotoInfoContainer getPhotographInformations() const;

    bool getXMLImageProperties(QString& comments, QDateTime& date, 
                               int& rating, QStringList& tagsPath);
    bool setXMLImageProperties(const QString& comments, const QDateTime& date, 
                               int rating, const QStringList& tagsPath);

private:

    bool setProgramId(bool on=true);
    bool setIptcTag(const QString& text, int maxLength, const char* debugLabel, const char* tagKey);
};

}  // NameSpace Digikam

#endif /* DMETADATA_H */
