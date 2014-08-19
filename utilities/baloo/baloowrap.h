/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-08-05
 * Description : Find Duplicates tree-view search album.
 *
 * Copyright (C) 2014 by Veaeceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef BALOOWRAP_H
#define BALOOWRAP_H

// Qt includes

#include <QObject>
#include <QStringList>
#include <QPointer>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

class KJob;
class KUrl;

namespace Digikam
{

class ImageInfo;

class BalooInfo
{
public:

    BalooInfo()
    {
        rating = -1;
    }

    QStringList tags;
    QString     comment;
    int         rating;
};

/**
 * @brief The BalooWrap class is a singleton class which offer
 *        functionatity for reading and writing image
 *        comment, tags and rating from Baloo to digiKam
 *        and from digiKam to Baloo
 *
 *        The singleton functionality is required because
 *        it also watches for changes in Baloo and notify
 *        digiKam, so it could trigger a scan
 */
class DIGIKAM_DATABASE_EXPORT BalooWrap : public QObject
{
    Q_OBJECT

public:

    BalooWrap(QObject* const parent = 0);
    ~BalooWrap();

    /**
     * @brief internalPtr - singleton inplementation
     */
    static QPointer<BalooWrap> internalPtr;
    static BalooWrap* instance();
    static bool isCreated();

    void setTags(const KUrl& url, QStringList* const tags);

    void setComment(const KUrl& url, QString* const comment);

    void setRating(const KUrl& url, int rating);

    /**
     * @brief setAllData - generic method to set all data from digiKam to Baloo
     * @param url        - image filepath
     * @param tags       - tags to set to image, pass NULL to ignore
     * @param comment    - comment set to image, pass NULL to ignore
     * @param rating     - rating to set to image, set to -1 to ignore
     */
    void setAllData(const KUrl& url, QStringList* const tags, QString* const comment, int rating);

    /**
     * @brief getSemanticInfo - Used by ImageScanner to retrieve all information
     *                          tags, comment, rating
     * @param url  - image url
     * @return     - container class for tags, comment, rating
     */
    BalooInfo getSemanticInfo(const KUrl& url);

    /**
     * @brief bestDigikamTagForTagName - used in previous Nepomuk implementation,
     *                                   searches for the best match for tag name
     * @param info
     * @param tagname                  - tagname to be searched
     * @return                         - tagId of the new or existing tag
     */
    int bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname) const;

    /**
     * @brief addInfoToDigikam  - alternative way to add info in digiKam, using
     *                            Database acess, not used now.
     * @param info
     * @param url
     */
    void addInfoToDigikam(const BalooInfo& info, const KUrl& fileUrl);

    void setSyncToBaloo(bool value);

    bool getSyncToBaloo() const;

    void setSyncToDigikam(bool value);

    bool getSyncToDigikam() const;

public Q_SLOTS:

    /**
     * @brief slotFetchFinished - used for asyncronous information retrieval
     *                            does not work now
     * @param job               - KJob for retrieving Baloo info
     */
    void slotFetchFinished(KJob* job);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* BALOOWRAP_H */
