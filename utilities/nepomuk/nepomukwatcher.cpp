/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-18
 * Description : Nepomuk Watcher class that keep tracks of changes in Nepomuk
 *               and apply them into digiKam database
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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
#include "nepomukwatcher.h"
#include "digikamnepomukservice.h"
#include <kdebug.h>
#include <kurl.h>

#include <Nepomuk2/ResourceWatcher>
#include <Nepomuk2/Variant>
#include <QVariant>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk2/Vocabulary/NFO>
#include <Nepomuk2/Vocabulary/NIE>

using namespace Nepomuk2;
using namespace Soprano::Vocabulary;

namespace Digikam
{

class NepomukWatcher::NepomukWatcherPriv
{
public:
    NepomukWatcherPriv()
    {
        resWatch = 0;
    };
    ResourceWatcher* resWatch;
    ResourceWatcher* tagWatch;
    DkNepomukService* parent;
};

NepomukWatcher::NepomukWatcher(DkNepomukService* parent)
    : QObject(parent), d(new NepomukWatcherPriv())
{
    d->parent   = parent;
    d->resWatch = new ResourceWatcher();
    d->resWatch->addProperty(NAO::hasTag());
    d->resWatch->addProperty(NAO::numericRating());
    d->resWatch->addProperty(NAO::description());

    d->resWatch->addType(Nepomuk2::Vocabulary::NFO::Image());

    d->tagWatch = new ResourceWatcher();
    d->tagWatch->addType(NAO::Tag());

    connect(d->resWatch, SIGNAL(propertyAdded(Nepomuk2::Resource,
                                              Nepomuk2::Types::Property,
                                              QVariant)),
        this, SLOT(slotPropertyAdded(Nepomuk2::Resource,
                                     Nepomuk2::Types::Property,
                                     QVariant)));

    connect(d->resWatch, SIGNAL(propertyRemoved(Nepomuk2::Resource,
                                                Nepomuk2::Types::Property,
                                                QVariant)),
        this, SLOT(slotPropertyRemoved(Nepomuk2::Resource,
                                       Nepomuk2::Types::Property,
                                       QVariant)));

    connect(d->tagWatch, SIGNAL(resourceCreated(Nepomuk2::Resource, QList<QUrl>)),
            this, SLOT(slotResAdded(Nepomuk2::Resource, QList<QUrl>)));

    connect(d->tagWatch, SIGNAL(resourceRemoved(QUrl, QList<QUrl>)),
            this, SLOT(slotResRemoved(QUrl, QList<QUrl>)));

    kDebug() << "Starting Resource Watcher ...";
    d->resWatch->start();
    d->tagWatch->start();
}

NepomukWatcher::~NepomukWatcher()
{
    d->resWatch->stop();
    d->tagWatch->stop();
    delete d->resWatch;
    delete d->tagWatch;
}

void NepomukWatcher::slotPropertyAdded(Resource res, Types::Property prop, QVariant var)
{
    //kDebug() << "Property Added !!! ++++++++++++++++++++++++" << res.uri();
    //kDebug() << "Property Added " << prop << " " <<  var << " +++++++++++++++++++++++++";
    //kDebug() << var << "++++++++++++++++++++++++++";

    //if(d->locked)
    //    return;

    //d->locked = true;
    //TODO: use change DkNepomukService's functions to use only one argument
    if(prop == NAO::hasTag())
    {
        KUrl::List resList;
        QList<QUrl> tagList;
        kDebug() << "Will add tags to image";
        KUrl url(res.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
        //kDebug() << var.toUrl() << "++++++++";
        resList << url;
        tagList << var.toUrl();
        d->parent->syncTagsToDigikam(resList, tagList);
    }
    if(prop == NAO::numericRating())
    {
        kDebug() << "Will change rating to image";
        KUrl::List resList;
        QList<int> ratingList;
        KUrl url(res.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
        resList << url;
        ratingList << var.toInt();
        d->parent->syncRatingToDigikam(resList, ratingList);
    }
    if(prop == NAO::description())
    {
        kDebug() << "Will add description";
        KUrl::List resList;
        QList<QString> commentList;
        KUrl url(res.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
        resList << url;
        commentList << var.toString();
        d->parent->syncCommentToDigikam(resList, commentList);
    }

}

void NepomukWatcher::slotPropertyRemoved(Resource res, Types::Property prop, QVariant var)
{
    //kDebug() << "Property Removed !!! ++++++++++++++++++++++++" << res.uri();
    //kDebug() << "Property Removed " << prop << " " << var << "+++++++++++++++++++++++++";
    //kDebug() << var << "++++++++++++++++++++++++++";

    //if(d->locked)
    //    return;

    //d->locked = true;

    kDebug() << (res.type() == NAO::hasTag());
    if(prop == NAO::hasTag())
    {
        QUrl tag = var.toUrl();
        kDebug() << "Will remove tags from image";
        KUrl url(res.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
        d->parent->removeTagInDigikam(url, tag);
    }

}

void NepomukWatcher::slotResAdded(Resource res, QList<QUrl> types)
{
    kDebug() << "Resource created++++++++++++";

    if(types.contains(NAO::hasTag()))
    {
        kDebug() << "Will add tags";
    }
}

void NepomukWatcher::slotResRemoved(QUrl url, QList<QUrl> types)
{
    kDebug() << "Resource removed +++++++++++++++++";

    if(types.contains(NAO::hasTag()))
    {
        kDebug() << "Will remove tags";
    }
}
}