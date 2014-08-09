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

// Qt
#include <QObject>
#include <QStringList>
#include <QPointer>

// KDE
#include <kurl.h>

// Local
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
    QString comment;
    int rating;
};
/**
 * A real metadata backend using Baloo to store and retrieve metadata.
 */
class DIGIKAM_DATABASE_EXPORT BalooWrap : public QObject
{
    Q_OBJECT
public:
    BalooWrap(QObject* parent = 0);
    ~BalooWrap();


    static QPointer<BalooWrap> internalPtr;
    static BalooWrap* instance();
    static bool isCreated() { return !(internalPtr.isNull()); }

    QStringList getTags(KUrl& url);

    QString getComment(KUrl& url);

    int getRating(KUrl& url);

    void setTags(KUrl& url, QStringList* tags);

    void setComment(KUrl& url, QString* comment);

    void setRating(KUrl& url, int rating);

    void setAllData(KUrl& url, QStringList *tags, QString *comment, int rating);

    BalooInfo getSemanticInfo(const KUrl&);

    int bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname) const;

    void addInfoToDigikam(BalooInfo& info, const KUrl &url);

//    virtual TagSet allTags() const;

//    virtual void refreshAllTags();

//    virtual void storeSemanticInfo(const KUrl&, const SemanticInfo&);

//    virtual void retrieveSemanticInfo(const KUrl&);

//    virtual QString labelForTag(const SemanticInfoTag&) const;

//    virtual SemanticInfoTag tagForLabel(const QString&);

public Q_SLOTS:
    void slotFetchFinished(KJob* job);

private:
    class Private;
    Private* const d;
};

} // namespace

#endif /* BALOOWRAP_H */
