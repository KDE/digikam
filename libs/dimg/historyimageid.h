/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Qt includes

#include <QDateTime>
#include <QMetaType>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam 
{

class DIGIKAM_EXPORT HistoryImageId
{
public:

    enum Type
    {
        InvalidType,
        /// The original file (typically created by a camera)
        Original,
        /// A file created in the history from the original file to the current file
        Intermediate,
        /** The "current" file. This is a special entry: It refers to the file from
         *  which this history was read. It need not be written to the file,
         *  because it describes the file itself. There is typically
         *  exactly one current entry if the history is associated with an image;
         *  there can be no current entry.
         */
        Current
    };

    /// Creates an invalid HistoryImageId
    HistoryImageId();

    /// Creates an id with the given UUID and type
    HistoryImageId(const QString& uuid, Type type = Current);

    /// A valid id needs at least a valid type and a UUID or a filename
    bool isValid() const;

    bool isOriginalFile() const { return m_type == Original; }
    bool isIntermediateFile() const { return m_type == Intermediate; }
    bool isCurrentFile() const  { return m_type == Current; }

    bool operator==(const HistoryImageId& other) const;

    void setType(HistoryImageId::Type type);
    void setUuid(const QString& uuid);
    void setFileName(const QString& fileName);
    void setCreationDate(const QDateTime& creationDate);
    void setPathOnDisk(const QString& filePath);
    void setUniqueHash(const QString& uniqueHash, int fileSize);

    /// Type of this History Image Id
    Type      m_type;

    /**
     * A unique identifier for the referred file. This id shall be changed each time
     * the image is edited.
     */
    QString   m_uuid;

    /// The filename of the referred file
    QString   m_fileName;
    /// The creationDate of the original image
    QDateTime m_creationDate;
    /// The path of the referred file (without file name)
    QString   m_filePath;
    /// The uniqueHash of the referred file
    QString   m_uniqueHash;
    /// The file size of the referred file
    int       m_fileSize;
    /**
     * A unique identifier designating the _original image_ from which the referred
     * image was created. Typically, this is a RAW or JPEG created by the camera in
     * the moment of taking the photograph.
     */
    QString   m_originalUUID;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::HistoryImageId)

#endif // HISTORYIMAGEID_H
