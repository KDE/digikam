/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef HISTORYIMAGEID_H
#define HISTORYIMAGEID_H

#include <QString>
#include <QDateTime>

#include "digikam_export.h"

namespace Digikam 
{

class DIGIKAM_EXPORT HistoryImageId
{
public:

    HistoryImageId();
    HistoryImageId(const QString& originalUUID, const QString& fileUUID,
                   const QString& fileName, const QDateTime& creationDate);

    bool matches(const HistoryImageId& other) const;
    bool isEmpty() const;
    bool isOriginalFile() const;

    /**
     * A unique identifier designating the _original image_ from which the referred
     * image was created. Typically, this is a RAW or JPEG created by the camera in
     * the moment of taking the photograph.
     */
    QString m_originalUUID;

    /// The creationDate of the original image
    QDateTime m_creationDate;
    /// The filename of the referred file
    QString m_fileName;

    /**
     * A unique identifier for the referred file. This id shall be changed each time
     * the image is edited.
     */
    QString m_fileUUID;
};
}
#endif // HISTORYIMAGEID_H
