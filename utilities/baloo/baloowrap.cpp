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

#include "baloowrap.h"

// Qt includes

#include <QUrl>
#include <QDebug>

// KDE includes

#include <klocalizedstring.h>


// Baloo includes

#include <Baloo/baloo/file.h>
#include <kfilemetadata/usermetadata.h>

// Local includes

#include "digikam_debug.h"


namespace Digikam
{

class BalooWrap::Private
{
public:

    Private()
    {
        syncFromBalooToDigikam = false;
        syncFromDigikamToBaloo = false;
    }

    bool syncFromDigikamToBaloo;
    bool syncFromBalooToDigikam;
};

QPointer<BalooWrap> BalooWrap::internalPtr = QPointer<BalooWrap>();

BalooWrap::BalooWrap(QObject* const parent)
    : QObject(parent), d(new BalooWrap::Private)
{

}

BalooWrap::~BalooWrap()
{

    delete d;
}

bool BalooWrap::isCreated()
{
    return !(internalPtr.isNull());
}

BalooWrap* BalooWrap::instance()
{
    if (BalooWrap::internalPtr.isNull())
    {
        BalooWrap::internalPtr = new BalooWrap();
    }

    return BalooWrap::internalPtr;
}


void BalooWrap::setTags(const QUrl& url, QStringList* const tags)
{
    setAllData(url,tags, NULL, -1);
}

void BalooWrap::setComment(const QUrl& url, QString* const comment)
{
    setAllData(url, NULL, comment, -1);
}

void BalooWrap::setRating(const QUrl& url, int rating)
{
    setAllData(url, NULL, NULL, rating);
}

void BalooWrap::setAllData(const QUrl& url, QStringList* const tags, QString* const comment, int rating)
{

    if(!d->syncFromDigikamToBaloo)
    {
        return;
    }

    KFileMetaData::UserMetaData md(url.path());

    if(tags != NULL)
    {
        md.setTags(*tags);
    }

    if(comment != NULL)
    {
        md.setUserComment(*comment);
    }

    if(rating != -1)
    {
        // digiKam store rating as value form 0 to 5
        // while baloo store it as value from 0 to 10
        md.setRating(rating*2);
    }

}

BalooInfo BalooWrap::getSemanticInfo(const QUrl& url)
{
    if(!d->syncFromBalooToDigikam)
    {
        return BalooInfo();
    }

    //Baloo::FileFetchJob* const job = new Baloo::FileFetchJob(url.toLocalFile());

    //job->exec();
    KFileMetaData::UserMetaData md(url.path());

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Job started";

    //Baloo::File file = job->file();
    BalooInfo bInfo;

    // Baloo have rating from 0 to 10, while digikam have only from 0 to 5
    bInfo.rating     = md.rating()/2;
    bInfo.comment    = md.userComment();

    Q_FOREACH(QString tag, md.tags().toSet())
    {
        bInfo.tags.append(i18n("BalooTags/") + tag);
    }

    return bInfo;
}


void BalooWrap::setSyncToBaloo(bool value)
{
    d->syncFromDigikamToBaloo = value;
}

bool BalooWrap::getSyncToBaloo() const
{
    return d->syncFromDigikamToBaloo;
}

bool BalooWrap::getSyncToDigikam() const
{
    return d->syncFromBalooToDigikam;
}

void BalooWrap::setSyncToDigikam(bool value)
{
    d->syncFromBalooToDigikam = value;
}

// NOTE: useful code to extend functionality in the future

//int BalooWrap::bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname) const
//{
//    if (tagname.isEmpty())
//    {
//        return 0;
//    }

//    QList<int> candidates = TagsCache::instance()->tagsForName(tagname);

//    if (candidates.isEmpty())
//    {
//        // add top-level tag
//        return DatabaseAccess().db()->addTag(0, tagname, QString(), 0);
//    }
//    else if (candidates.size() == 1)
//    {
//        return candidates.first();
//    }
//    else
//    {
//        int currentCandidate    = 0;
//        int currentMinimumScore = 0;
//        QList<int> assignedTags = info.tagIds();

//        foreach(int tagId, candidates)
//        {
//            // already assigned one of the candidates?
//            if (assignedTags.contains(tagId))
//            {
//                return 0;
//            }

//            int id    = tagId;
//            int score = 0;

//            do
//            {
//                id = TagsCache::instance()->parentTag(id);
//                score++;
//            }
//            while (id);

//            if (!currentMinimumScore || score < currentMinimumScore)
//            {
//                currentCandidate = tagId;
//            }
//        }

//        return currentCandidate;
//    }
//}

//void BalooWrap::addInfoToDigikam(const BalooInfo& bInfo, const QUrl& fileUrl)
//{
//    QStringList tags = bInfo.tags;
//    QList<int> tagIdsForInfo;
//    ImageInfo info   = ImageInfo::fromUrl(fileUrl);

//    // If the path is not in digikam collections, info will be null.
//    // It does the same check first that we would be doing here

//    if(info.isNull())
//    {
//        return;
//    }

//    const int size = tags.size();

//    for (int i = 0; i < size; ++i)
//    {
//        int tagId = bestDigikamTagForTagName(info, tags.at(i));

//        if (tagId)
//        {
//            tagIdsForInfo << tagId;
//        }
//    }

//    if (!tagIdsForInfo.isEmpty())
//    {
//        DatabaseAccess access;
//        DatabaseTransaction transaction(&access);
//        const int infosSize = tagIdsForInfo.size();

//        for (int i = 0; i < infosSize; ++i)
//        {
//            info.setTag(tagIdsForInfo.at(i));
//        }
//    }
//}

//TagSet BalooWrap::allTags() const
//{
//    if (d->mAllTags.empty())
//    {
//        const_cast<BalooWrap*>(this)->refreshAllTags();
//    }
//    return d->mAllTags;
//}

//void BalooWrap::refreshAllTags()
//{
//    Baloo::TagListJob* job = new Baloo::TagListJob();
//    job->exec();

//    d->mAllTags.clear();
//    Q_FOREACH(const QString& tag, job->tags())
//    {
//        d->mAllTags << tag;
//    }
//}

} // namespace Digikam
