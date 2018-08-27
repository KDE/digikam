/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MEDIAWIKI_IMAGEINFO_H
#define MEDIAWIKI_IMAGEINFO_H

// Qt includes

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QUrl>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace MediaWiki
{

/**
 * @brief An image info.
 */
class DIGIKAM_EXPORT Imageinfo
{
public:

    /**
     * @brief Constructs an image info.
     */
    Imageinfo();

    /**
     * @brief Constructs an image info from an other image info.
     * @param other an other image info
     */
    Imageinfo(const Imageinfo& other);

    /**
     * @brief Destructs an image info.
     */
    ~Imageinfo();

    /**
     * @brief Assingning an image info from an other image info.
     * @param other an other image info
     */
    Imageinfo& operator=(Imageinfo other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const Imageinfo& other) const;

    /**
     * @brief Get the time and date of the revision.
     * @return the time and date of the revision
     */
    QDateTime timestamp() const;

    /**
     * @brief Set the time and date of the revision.
     * @param timestamp the time and date of the revision
     */
    void setTimestamp(const QDateTime& timestamp);

    /**
     * @brief Get the user who made the revision.
     * @return the user who made the revision
     */
    QString user() const;

    /**
     * @brief Set the user who made the revision.
     * @param user the user who made the revision
     */
    void setUser(const QString& user);

    /**
     * @brief Get the edit comment.
     * @return the edit comment
     */
    QString comment() const;

    /**
     * @brief Set the edit comment.
     * @param comment the edit comment
     */
    void setComment(const QString& comment);

    /**
     * @brief Get the URL of the image.
     * @return the URL of the image
     */
    QUrl url() const;

    /**
     * @brief Set the URL of the image.
     * @param url the URL of the image
     */
    void setUrl(const QUrl& url);

    /**
     * @brief Get the description URL of the image.
     * @return the description URL of the image
     */
    QUrl descriptionUrl() const;

    /**
     * @brief Set the description URL of the image.
     * @param descriptionUrl the description URL of the image
     */
    void setDescriptionUrl(const QUrl& descriptionUrl);

    /**
     * @brief Get the thumb URL of the image.
     * @return the thumb URL of the image
     */
    QUrl thumbUrl() const;

    /**
     * @brief Get the thumb URL of the image.
     * @param thumbUrl the thumb URL of the image
     */
    void setThumbUrl(const QUrl& thumbUrl);

    /**
     * @brief Get the thumb width of the image.
     * @return the thumb width of the image
     */
    qint64 thumbWidth() const;

    /**
     * @brief Set the thumb width of the image.
     * @param thumbWidth the thumb width of the image
     */
    void setThumbWidth(qint64 thumbWidth);

    /**
     * @brief Get the thumb height of the image.
     * @return the thumb height of the image
     */
    qint64 thumbHeight() const;

    /**
     * @brief Set the thumb height of the image.
     * @param thumbHeight the thumb height of the image
     */
    void setThumbHeight(qint64 thumbHeight);

    /**
     * @brief Get the image's size in bytes.
     * @return the image's size in bytes
     */
    qint64 size() const;

    /**
     * @brief Set the image's size in bytes.
     * @param size the image's size in bytes
     */
    void setSize(qint64 size);

    /**
     * @brief Get the image's width.
     * @return the image's width
     */
    qint64 width() const;

    /**
     * @brief Set the image's width.
     * @param width the image's width
     */
    void setWidth(qint64 width);

    /**
     * @brief Get the image's height.
     * @return the image's height
     */
    qint64 height() const;

    /**
     * @brief Set the image's height.
     * @param height the image's height
     */
    void setHeight(qint64 height);

    /**
     * @brief Get the image's SHA-1 hash.
     * @return the image's SHA-1 hash
     */
    QString sha1() const;

    /**
     * @brief Set the image's SHA-1 hash.
     * @param sha1 the image's SHA-1 hash
     */
    void setSha1(const QString& sha1);

    /**
     * @brief Get the image's MIME type.
     * @return the image's MIME type
     */
    QString mime() const;

    /**
     * @brief Set the image's MIME type.
     * @param mime the image's MIME type
     */
    void setMime(const QString& mime);

    /**
     * @brief Get image metadata.
     * @return image metadata
     */
    const QHash<QString, QVariant>& metadata() const;

    /**
     * @brief Get image metadata.
     * @return image metadata
     */
    QHash<QString, QVariant>& metadata();

    /**
     * @brief Set image metadata.
     * @param metadata image metadata
     */
     void setMetadata(const QHash<QString, QVariant>& metadata);

private:

    class Private;
    Private* const d;
};

} // namespace MediaWiki

#endif // MEDIAWIKI_IMAGEINFO_H
