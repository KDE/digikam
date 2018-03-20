/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef HISTORY_IMAGE_ID_H
#define HISTORY_IMAGE_ID_H

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
        InvalidType    = 0,
        /**
         * The original file (typically created by a camera)
         */
        Original       = 1 << 0,

        /**
         * A file created during the editing the history,
         * between the original file and the current file.
         */
        Intermediate   = 1 << 1,

        /**
         * When a file is created from multiple files, there can be no direct
         * original (panorama) but multiple sources, or one direct original
         * and some other, additional source files.
         * To record source files outside of the direct history, this type is used.
         */
        Source         = 1 << 2,

        /**
         * The "current" file. This is a special entry: It refers to the file from
         * which this history was read. It need not be written to the file,
         * because it describes the file itself. There is typically
         * exactly one current entry if the history is associated with an image;
         * there can be no current entry.
         */
        Current        = 1 << 3
    };

    /**
     * Note: In this class, the Type is used as a simple enum,
     * but it is also prepared for usage as flags.
     */
    Q_DECLARE_FLAGS(Types, Type)

public:

    /// Creates an invalid HistoryImageId
    HistoryImageId();

    /// Creates an id with the given UUID and type
    explicit HistoryImageId(const QString& uuid, Type type = Current);

    /// A valid id needs at least a valid type and a UUID or a filename
    bool isValid() const;

    Type type() const;

    bool isOriginalFile() const
    {
        return type() == Original;
    }

    bool isSourceFile() const
    {
        return type() == Source;
    }

    bool isIntermediateFile() const
    {
        return type() == Intermediate;
    }

    bool isCurrentFile() const
    {
        return type() == Current;
    }

    bool operator==(const HistoryImageId& other) const;

    void setType(HistoryImageId::Type type);
    void setUuid(const QString& uuid);
    void setFileName(const QString& fileName);
    void setCreationDate(const QDateTime& creationDate);
    void setPathOnDisk(const QString& filePath);
    void setPath(const QString& path);
    void setUniqueHash(const QString& uniqueHash, qlonglong fileSize);

    bool hasFileOnDisk() const;

    ///If a file on disk is referenced: Returns the path, without filename, with a trailing slash
    QString path() const;

    /// If a file on disk is referenced: Returns the full file path (folder + filename)
    QString filePath() const;

    bool hasFileName() const;

    /// If a file on disk is referenced: Returns the file name (without folder)
    QString fileName() const;

    bool      hasUuid()                 const;
    QString   uuid()                    const;
    bool      hasCreationDate()         const;
    QDateTime creationDate()            const;
    bool      hasUniqueHashIdentifier() const;
    QString   uniqueHash()              const;
    qlonglong fileSize()                const;
    QString   originalUuid()            const;

public:

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
    /// The path of the referred file (NOTE: without file name!, including trailing slash)
    QString   m_filePath;
    /// The uniqueHash of the referred file
    QString   m_uniqueHash;
    /// The file size of the referred file
    qlonglong m_fileSize;
    /**
     * A unique identifier designating the _original image_ from which the referred
     * image was created. Typically, this is a RAW or JPEG created by the camera in
     * the moment of taking the photograph.
     */
    QString   m_originalUUID;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::HistoryImageId)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::HistoryImageId::Types)

#endif // HISTORY_IMAGE_ID_H
