/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-19
 * Description : Service to sync digikam and nepomuk storages
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamnepomukservice.moc"

// Qt includes

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QThread>
#include <QTimer>

// KDE includes

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kio/job.h>
#include <kpluginfactory.h>
#include <kurl.h>
#include <nepomuk/resource.h>
#include <nepomuk/resourcemanager.h>
#include <nepomuk/nepomukservice.h>
#include <nepomuk/tag.h>
#include <nepomuk/variant.h>
#include <soprano/model.h>
#include <soprano/queryresultiterator.h>
#include <soprano/statement.h>
#include <soprano/nao.h>
#include <soprano/xsd.h>

// Local includes

// QList<int> metatype defined in Nepomuk/Variant
#define DATABASECHANGESETS_H_NO_QLIST_METATYPE_DECLARATION
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databaseparameters.h"
#include "databasetransaction.h"
#include "databasewatch.h"
#include "imagecomments.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "tagscache.h"

#include "nfo.h"
#include "nie.h"

namespace Digikam
{

enum WatchedNepomukProperty
{
    NaoRating,
    NaoDescription,
    NaoTags
};

class NepomukService::NepomukServicePriv
{
public:

    NepomukServicePriv() :
        syncToDigikam(false),
        syncToNepomuk(false),
        isConnected(false),
        changingDB(false),
        changingNepomuk(false),
        fullSyncJobs(0),
        nepomukChangeTimer(0),
        cleanIgnoreListTimer(0)
    {
    }

    bool                                     syncToDigikam;
    bool                                     syncToNepomuk;
    bool                                     isConnected;
    bool                                     changingDB;
    bool                                     changingNepomuk;
    int                                      fullSyncJobs;
    QTimer*                                  nepomukChangeTimer;
    QTimer*                                  cleanIgnoreListTimer;
    QMultiHash<QUrl, WatchedNepomukProperty> ignoreUris;

    bool checkIgnoreUris(const QUrl& url, WatchedNepomukProperty property)
    {
        QHash<QUrl, WatchedNepomukProperty>::iterator it;
        it = ignoreUris.find(url, property);

        if (it != ignoreUris.end())
        {
            ignoreUris.erase(it);
            return true;
        }

        return false;
    }

    void addIgnoreUri(const QUrl& url, WatchedNepomukProperty property)
    {
        //if (!ignoreUris.contains(url, property))
        ignoreUris.insert(url, property);

        // always restart timer
        cleanIgnoreListTimer->start();
    }

    void triggerSyncToDigikam()
    {
        if (!nepomukChangeTimer->isActive())
        {
            nepomukChangeTimer->start();
        }
    }
};

class ChangingDB
{
public:

    ChangingDB(NepomukService::NepomukServicePriv* d)
        : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

    NepomukService::NepomukServicePriv* const d;
};

class ChangingNepomuk
{
public:

    ChangingNepomuk(NepomukService::NepomukServicePriv* d)
        : d(d)
    {
        d->changingNepomuk = true;
    }

    ~ChangingNepomuk()
    {
        d->changingNepomuk = false;
    }

    NepomukService::NepomukServicePriv* const d;
};


NepomukService::NepomukService(QObject* parent, const QVariantList&)
    : Nepomuk::Service(parent),
      d(new NepomukServicePriv)
{
    Nepomuk::ResourceManager::instance()->init();

    d->nepomukChangeTimer = new QTimer(this);
    d->nepomukChangeTimer->setSingleShot(true);
    d->nepomukChangeTimer->setInterval(5000);

    connect(d->nepomukChangeTimer, SIGNAL(timeout()),
            this, SLOT(syncNepomukToDigikam()));

    d->cleanIgnoreListTimer = new QTimer(this);
    d->cleanIgnoreListTimer->setSingleShot(true);
    d->cleanIgnoreListTimer->setInterval(5000);

    connect(d->cleanIgnoreListTimer, SIGNAL(timeout()),
            this, SLOT(cleanIgnoreList()));

    readConfig();
}

NepomukService::~NepomukService()
{
}

void NepomukService::readConfig()
{
    // Reads from digikam config what this service should do

    KSharedConfig::Ptr config = digikamConfig();
    KConfigGroup group        = config->group("Nepomuk Settings");
    // default to false here, regardless of default settings in digikam
    enableSyncToDigikam(group.readEntry("Sync Nepomuk to Digikam", false));
    enableSyncToNepomuk(group.readEntry("Sync Digikam to Nepomuk", false));
}

void NepomukService::enableSyncToDigikam(bool syncToDigikam)
{
    // Controls syncing Nepomuk -> Digikam.
    // Called from readConfig or via DBus (from DigikamApp)

    kDebug() << "Sync to digikam enabled: " << syncToDigikam;

    if (d->syncToDigikam == syncToDigikam)
    {
        return;
    }

    d->syncToDigikam = syncToDigikam;

    if (!d->isConnected)
    {
        connectToDatabase(databaseParameters());
    }

    if (!d->isConnected)
    {
        return;
    }

    if (d->syncToDigikam)
    {
        connect(mainModel(), SIGNAL(statementAdded(const Soprano::Statement&)),
                this, SLOT(slotStatementAdded(const Soprano::Statement&)));

        connect(mainModel(), SIGNAL(statementRemoved(const Soprano::Statement&)),
                this, SLOT(slotStatementRemoved(const Soprano::Statement&)));

        /*connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
                this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)));*/

        if (lastSyncToDigikam().isNull())
        {
            d->triggerSyncToDigikam();
        }
    }
    else
    {
        disconnect(mainModel(), SIGNAL(statementAdded(const Soprano::Statement&)),
                   this, SLOT(slotStatementAdded(const Soprano::Statement&)));

        disconnect(mainModel(), SIGNAL(statementRemoved(const Soprano::Statement&)),
                   this, SLOT(slotStatementRemoved(const Soprano::Statement&)));

        /*disconnect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
                   this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)));*/
    }
}

void NepomukService::enableSyncToNepomuk(bool syncToNepomuk)
{
    // Controls syncing Digikam -> Nepomuk.
    // Called from readConfig or via DBus (from DigikamApp)

    kDebug() << "Sync to nepomuk enabled:" << syncToNepomuk;

    if (d->syncToNepomuk == syncToNepomuk)
    {
        return;
    }

    d->syncToNepomuk = syncToNepomuk;

    if (!d->isConnected)
    {
        connectToDatabase(databaseParameters());
    }

    if (!d->isConnected)
    {
        return;
    }

    if (d->syncToNepomuk)
    {
        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
                this, SLOT(slotImageChange(const ImageChangeset&)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset&)),
                this, SLOT(slotImageTagChange(const ImageTagChangeset&)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset&)),
                this, SLOT(slotTagChange(const TagChangeset&)));

        // initial pushing to Nepomuk?
        if (!hasSyncToNepomuk())
        {
            QTimer::singleShot(1000, this, SLOT(fullSyncDigikamToNepomuk()));
        }
    }
    else
    {
        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset&)),
                   this, SLOT(slotImageChange(const ImageChangeset&)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset&)),
                   this, SLOT(slotImageTagChange(const ImageTagChangeset&)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset&)),
                   this, SLOT(slotTagChange(const TagChangeset&)));
    }
}

void NepomukService::triggerResync()
{
    clearSyncedToDigikam();
    clearSyncedToNepomuk();

    if (d->syncToNepomuk)
    {
        fullSyncDigikamToNepomuk();
    }

    if (d->syncToDigikam)
    {
        d->triggerSyncToDigikam();
    }
}

void NepomukService::setDatabase(const QString& paramsUrl)
{
    // Called via DBus
    if (!d->syncToDigikam && !d->syncToNepomuk)
    {
        return;
    }

    KUrl url(paramsUrl);
    kDebug() << "Got database params pushed from running instance:" << url;
    connectToDatabase(DatabaseParameters(url));
}

void NepomukService::connectToDatabase(const DatabaseParameters& params)
{
    // (Re-)connects to the database
    if (params == DatabaseAccess::parameters() || !params.isValid())
    {
        return;
    }

    d->isConnected = false;

    if (params.isValid())
    {
        DatabaseAccess::setParameters(params, DatabaseAccess::MainApplication);

        d->isConnected = DatabaseAccess::checkReadyForUse();

        if (!d->isConnected)
        {
            QString errorMsg = DatabaseAccess().lastError();
            kDebug() << "Failed to initialize database" << params.databaseName;
            return;
        }
    }
}

// ------------------- Sync Digikam -> Nepomuk  ------------------------

void NepomukService::slotImageChange(const ImageChangeset& changeset)
{
    // Receives signals (via DatabaseWatch via DBus) about changes in application

    if (d->changingDB)
    {
        return;
    }

    DatabaseFields::Set changes = changeset.changes();
    int settings                = 0;

    if (changes & DatabaseFields::Rating)
    {
        settings |= SyncRating | SyncHasNoRating;
    }

    if (changes & DatabaseFields::Comment)
    {
        settings |= SyncComment;
    }

    if (settings)
    {
        syncToNepomuk(changeset.ids(), (SyncToNepomukSettings)settings);
    }
}

void NepomukService::slotImageTagChange(const ImageTagChangeset& changeset)
{
    // Receives signals (via DatabaseWatch via DBus) about changes in application

    if (d->changingDB)
    {
        return;
    }

    kDebug() << changeset.ids() << changeset.tags() << (changeset.operation()==ImageTagChangeset::Added);

    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
            syncTagsToNepomuk(changeset.ids(), changeset.tags(), true);
            break;
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:
            syncTagsToNepomuk(changeset.ids(), changeset.tags(), false);
            break;
        default:
            break;
    }
}

void NepomukService::slotTagChange(const TagChangeset&)
{
}

void NepomukService::fullSyncDigikamToNepomuk()
{
    // List album root albums of all available collections recursively
    QList<CollectionLocation> collections = CollectionManager::instance()->allAvailableLocations();
    foreach (const CollectionLocation& location, collections)
    {
        DatabaseUrl url = DatabaseUrl::fromAlbumAndName(QString(), "/", location.albumRootPath(), location.id());
        KIO::Job* job = ImageLister::startListJob(url);
        job->addMetaData("listAlbumsRecursively", "true");

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotFullSyncJobResult(KJob*)));

        connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
                this, SLOT(slotFullSyncJobData(KIO::Job*, const QByteArray&)));

        d->fullSyncJobs++;
    }
}

void NepomukService::slotFullSyncJobResult(KJob* job)
{
    Q_UNUSED(job);

    // when initial full sync is done
    if ( (--d->fullSyncJobs) == 0)
    {
        markAsSyncedToNepomuk();
    }
}

void NepomukService::slotFullSyncJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    QList<ImageInfo> infos;

    QByteArray tmp(data);
    QDataStream ds(&tmp, QIODevice::ReadOnly);

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        infos << ImageInfo(record);
    }

    syncToNepomuk(infos, SyncToNepomukSettings(SyncRating | SyncComment)); // do _not_ sync no rating
    pushTagsToNepomuk(infos);
}

static int nepomukToDigikamRating(int nepomukRating)
{
    // Map [0,10] -> [-1,5]
    if (nepomukRating == 0)
    {
        return -1;
    }

    if (nepomukRating % 2)
    {
        return (nepomukRating + 1) / 2;
    }
    else
    {
        return nepomukRating / 2;
    }
}

static int digikamToNepomukRating(int digikamRating)
{
    // Map [-1, 5] -> [0, 10]: 1->2, 2->4, 3->6, 4->8, 5->10 so that there are "full" and not half stars in dolphin
    if (digikamRating == -1)
    {
        return 0;
    }

    return 2*digikamRating;
}

void NepomukService::syncToNepomuk(const QList<qlonglong>& imageIds, SyncToNepomukSettings syncSettings)
{
    QList<ImageInfo> infos;
    foreach (const qlonglong& imageid, imageIds)
    {
        ImageInfo info(imageid);

        if (!info.isNull())
        {
            infos << info;
        }
    }
    syncToNepomuk(infos, syncSettings);
}

void NepomukService::syncToNepomuk(const QList<ImageInfo>& infos, SyncToNepomukSettings syncSettings)
{
    foreach (const ImageInfo& info, infos)
    {
        ChangingNepomuk changing(d);

        Nepomuk::Resource res(info.fileUrl());

        if (syncSettings & SyncRating)
        {
            int rating = info.rating();

            if (rating != -1 || (syncSettings & SyncHasNoRating))
            {
                //kDebug() << "Setting rating" << info.rating() << res.resourceUri() << res.isValid();
                res.setRating(digikamToNepomukRating(info.rating()));
                d->addIgnoreUri(res.resourceUri(), NaoRating);
            }
        }

        if (syncSettings & SyncComment)
        {
            QString comment = info.comment();

            if (!comment.isEmpty())
            {
                //kDebug() << "Setting comment" << info.comment() << res.resourceUri() << res.isValid();
                res.setDescription(info.comment());
                d->addIgnoreUri(res.resourceUri(), NaoDescription);
            }
        }
    }
}

static Nepomuk::Tag nepomukForDigikamTag(int tagId)
{
    if (tagId)
    {
        QString tagName = TagsCache::instance()->tagName(tagId);
        Nepomuk::Tag tag(tagName);

        if (!tag.exists())
        {
            // from dolphin's panels/information/newtagdialog.cpp
            tag.setLabel(tagName);
            tag.addIdentifier(tagName);

            TagInfo info = DatabaseAccess().db()->getTagInfo(tagId);

            if (info.icon.isNull())
            {
                // Don't think we can use actual large files for tag icon
                /*
                // album image icon
                QString albumRootPath = CollectionManager::instance()->albumRootPath(info.iconAlbumRootId);
                album->m_icon         = albumRootPath + info.iconRelativePath;
                */
            }
            else
            {
                tag.addSymbol(info.icon);
            }
        }

        return tag;
    }

    return Nepomuk::Tag();
}

void NepomukService::syncTagsToNepomuk(const QList<qlonglong>& imageIds, const QList<int>& tagIds, bool addOrRemove)
{
    foreach (int tagId, tagIds)
    {
        ChangingNepomuk changing(d);
        Nepomuk::Tag tag = nepomukForDigikamTag(tagId);
        kDebug() << tag.resourceUri();

        if (tag.isValid())
        {
            foreach (const qlonglong& imageId, imageIds)
            {
                ImageInfo info(imageId);

                if (!info.isNull())
                {
                    Nepomuk::Resource res(info.fileUrl());
                    kDebug()<< res.resourceUri() << addOrRemove << res.properties();

                    if (addOrRemove)
                    {
                        res.addTag(tag);
                    }
                    else
                    {
                        res.removeProperty(Soprano::Vocabulary::NAO::hasTag(), tag.resourceUri());
                    }

                    d->addIgnoreUri(res.resourceUri(), NaoTags);
                    kDebug()<< "after change:" << res.properties();
                }
            }
        }
    }
}

void NepomukService::pushTagsToNepomuk(const QList<ImageInfo>& imageInfos)
{
    foreach (const ImageInfo& info, imageInfos)
    {
        ChangingNepomuk changing(d);

        if (!info.isNull())
        {
            foreach (int tagId, info.tagIds())
            {
                Nepomuk::Tag tag = nepomukForDigikamTag(tagId);

                if (tag.isValid())
                {
                    Nepomuk::Resource res(info.fileUrl());

                    res.addTag(tag);

                    d->addIgnoreUri(res.resourceUri(), NaoTags);
                }
            }
        }
    }
}

// ------------------- Sync Nepomuk -> Digikam ------------------------

/*
void NepomukService::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    kDebug() << changeset.operation() << changeset.ids();
    if (changeset.operation() == CollectionImageChangeset::Added)
    {
        syncAddedImagesToDigikam(changeset.ids());
    }
}
*/

void NepomukService::slotStatementAdded(const Soprano::Statement& statement)
{
    if (d->changingNepomuk) // no effect currently
    {
        return;
    }

    const Soprano::Node& subject = statement.subject();

    const Soprano::Node& predicate = statement.predicate();

    if (predicate == Soprano::Vocabulary::NAO::numericRating())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoRating))
        {
            return;
        }

        //kDebug() << "Changed rating" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
    else if (predicate == Soprano::Vocabulary::NAO::description())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoDescription))
        {
            return;
        }

        //kDebug() << "Changed comment" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
    else if (predicate == Soprano::Vocabulary::NAO::hasTag())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoTags))
        {
            return;
        }

        //kDebug() << "Added tag" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
}

void NepomukService::slotStatementRemoved(const Soprano::Statement& statement)
{
    Q_UNUSED(statement);

    if (d->changingNepomuk) // no effect currently
    {
        return;
    }

    const Soprano::Node& subject   = statement.subject();

    const Soprano::Node& predicate = statement.predicate();

    if (predicate == Soprano::Vocabulary::NAO::hasTag())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoTags))
        {
            return;
        }

        kDebug() << "Removed tag" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        Nepomuk::Resource res(subject.uri());
        removeTagInDigikam(res.property(Nepomuk::Vocabulary::NIE::url()).toString(), statement.object().uri());
    }
}

void NepomukService::cleanIgnoreList()
{
    d->ignoreUris.clear();
}

static QString nepomukChangeQuery(const QString& predicate, const QDateTime& dateTime)
{
    return  QString("PREFIX nao: <%1> "
                    "PREFIX xls: <%2> "
                    "PREFIX nie: <%3> "
                    "SELECT DISTINCT ?path ?value "
                    " WHERE { GRAPH ?g { ?r %4 ?value . } . "
                    " ?r nie:url ?path ."
                    " ?g nao:created ?t . "
                    " FILTER ( ?t > \"%5\"^^xls:dateTime ) . } ")
            .arg(Soprano::Vocabulary::NAO::naoNamespace().toString())
            .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString())
            .arg(Nepomuk::Vocabulary::NIE::nieNamespace().toString())
            .arg(predicate)
            .arg(Soprano::LiteralValue(dateTime).toString());
    /*
    More elegant, for Soprano 2.2.65
                            " FILTER ( ?t > %2 ) . } ")
                            .arg(Soprano::Node::literalToN3(lastSyncDate));

                            .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NIE::url())
    */
}

void NepomukService::syncNepomukToDigikam()
{
    // wait until digikam -> nepomuk initial sync, if any, has finished
    if (d->fullSyncJobs)
    {
        d->triggerSyncToDigikam();
    }

    QDateTime lastSyncDate = lastSyncToDigikam();

    if (!lastSyncDate.isValid())
    {
        lastSyncDate = QDateTime::fromTime_t(0);
    }

    QString query;
    KUrl::List fileUrls;
    KUrl fileUrl;
    Soprano::QueryResultIterator it;
    QString pathBinding("path"), valueBinding("value");

    query = nepomukChangeQuery("nao:numericRating", lastSyncDate);
    it    = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<int> ratings;
    int rating;

    while ( it.next() )
    {
        fileUrl = KUrl(it.binding(pathBinding).uri());
        rating  = it.binding(valueBinding).literal().toInt();

        if (!fileUrl.isEmpty() && rating >= 0 && rating <= 10)
        {
            fileUrls << fileUrl;
            ratings << rating;
        }
    }

    syncRatingToDigikam(fileUrls, ratings);

    fileUrls.clear();
    query = nepomukChangeQuery("nao:description", lastSyncDate);
    it    = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<QString> comments;
    QString comment;

    while ( it.next() )
    {
        fileUrl = KUrl(it.binding(pathBinding).uri());
        comment = it.binding(valueBinding).literal().toString();

        if (!fileUrl.isEmpty())
        {
            fileUrls << fileUrl;
            comments << comment;
        }
    }

    syncCommentToDigikam(fileUrls, comments);

    fileUrls.clear();
    query = nepomukChangeQuery("nao:hasTag", lastSyncDate);
    it    = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<QUrl> tags;
    QUrl tag;

    while ( it.next() )
    {
        fileUrl = KUrl(it.binding(pathBinding).uri());
        tag     = it.binding(valueBinding).uri();

        if (!fileUrl.isEmpty())
        {
            fileUrls << fileUrl;
            tags << tag;
        }
    }

    syncTagsToDigikam(fileUrls, tags);

    // we mark this regardless of having changed anything
    markAsSyncedToDigikam();
}

/*
TODO: Integrate to ImageScanner
void NepomukService::syncAddedImagesToDigikam(const QList<qlonglong> &ids)
{
    foreach (qlonglong id, ids)
    {
        ImageInfo info(id);
        if (info.isNull())
            continue;

        ChangingDB changing(d);

        Nepomuk::Resource res(info.fileUrl(), Nepomuk::Vocabulary::NFO::FileDataObject());
        Nepomuk::Variant rating = res.property(Soprano::Vocabulary::NAO::numericRating());
        if (rating.isValid())
            info.setRating(nepomukToDigikamRating(rating.toInt()));

        Nepomuk::Variant comment = res.property(Soprano::Vocabulary::NAO::description());
        if (comment.isValid())
        {
            DatabaseAccess access;
            ImageComments comments = info.imageComments(access);
            comments.addComment(comment.toString());
        }

        QList<Nepomuk::Tag> tags = res.tags();
        foreach (const Nepomuk::Tag& tag, tags)
        {
            int id = bestDigikamTagForTagName(info, tag.genericLabel());
            if (id)
                info.setTag(id);
        }
    }
}
*/

void NepomukService::syncRatingToDigikam(const KUrl::List& fileUrls, const QList<int>& ratings)
{
    if (fileUrls.isEmpty())
    {
        return;
    }

    QList<ImageInfo> infos;
    QList<int> ratingsForInfos;
    const int size = fileUrls.size();

    for (int i=0; i<size; i++)
    {
        // If the path is not in digikam collections, info will be null.
        // It does the same check first that we would be doing here
        ImageInfo info(fileUrls[i]);

        if (!info.isNull())
        {
            infos << info;
            ratingsForInfos << nepomukToDigikamRating(ratings[i]);
        }
    }

    if (!infos.isEmpty())
    {
        ChangingDB changing(d);

        DatabaseAccess access;
        DatabaseTransaction transaction(&access);
        const int infosSize = infos.size();

        for (int i=0; i<infosSize; i++)
        {
            infos[i].setRating(ratingsForInfos[i]);
        }
    }
}

void NepomukService::syncCommentToDigikam(const KUrl::List& fileUrls, const QStringList& comments)
{
    if (fileUrls.isEmpty())
    {
        return;
    }

    QList<ImageInfo> infos;
    QList<QString> commentsForInfos;
    const int size = fileUrls.size();

    for (int i=0; i<size; i++)
    {
        // If the path is not in digikam collections, info will be null.
        // It does the same check first that we would be doing here
        ImageInfo info(fileUrls[i]);

        if (!info.isNull())
        {
            infos << info;
            commentsForInfos << comments[i];
        }
    }

    if (!infos.isEmpty())
    {
        ChangingDB changing(d);

        DatabaseAccess access;
        DatabaseTransaction transaction(&access);
        const int infosSize = infos.size();

        for (int i=0; i<infosSize; i++)
        {
            DatabaseAccess access;
            ImageComments comments = infos[i].imageComments(access);
            comments.addComment(commentsForInfos[i]);
        }
    }
}

void NepomukService::syncTagsToDigikam(const KUrl::List& fileUrls, const QList<QUrl>& tags)
{
    if (fileUrls.isEmpty())
    {
        return;
    }

    QList<ImageInfo> infos;
    QList<int> tagIdsForInfos;
    const int size = fileUrls.size();

    for (int i=0; i<size; i++)
    {
        // If the path is not in digikam collections, info will be null.
        // It does the same check first that we would be doing here
        ImageInfo info(fileUrls[i]);

        if (!info.isNull())
        {
            infos << info;
            QString tagName = tagnameForNepomukTag(tags[i]);
            int tagId = bestDigikamTagForTagName(info, tagName);

            if (tagId)
            {
                tagIdsForInfos << tagId;
            }
        }
    }

    if (!infos.isEmpty())
    {
        DatabaseAccess access;
        DatabaseTransaction transaction(&access);
        const int infosSize = infos.size();

        for (int i=0; i<infosSize; i++)
        {
            infos[i].setTag(tagIdsForInfos[i]);
        }
    }
}

void NepomukService::removeTagInDigikam(const KUrl& fileUrl, const QUrl& tag)
{
    if (fileUrl.isEmpty())
    {
        return;
    }

    ImageInfo info(fileUrl);

    if (info.isNull())
    {
        return;
    }

    QList<int> tags = info.tagIds();

    if (tags.isEmpty())
    {
        return;
    }

    QString tagName = tagnameForNepomukTag(tag);
    QList<int> candidates = TagsCache::instance()->tagsForName(tagName);

    if (candidates.isEmpty())
    {
        return;
    }

    foreach (int candidate, candidates)
    {
        if (tags.contains(candidate))
        {
            info.removeTag(candidate);
        }
    }
}

int NepomukService::bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname)
{
    if (tagname.isEmpty())
    {
        return 0;
    }

    QList<int> candidates = TagsCache::instance()->tagsForName(tagname);

    if (candidates.isEmpty())
    {
        // add top-level tag
        return DatabaseAccess().db()->addTag(0, tagname, QString(), 0);
    }
    else if (candidates.size() == 1)
    {
        return candidates.first();
    }
    else
    {
        int currentCandidate = 0;
        int currentMinimumScore = 0;
        QList<int> assignedTags = info.tagIds();
        foreach (int tagId, candidates)
        {
            // already assigned one of the candidates?
            if (assignedTags.contains(tagId))
            {
                return 0;
            }

            int id = tagId;
            int score = 0;

            do
            {
                id = TagsCache::instance()->parentTag(id);
                score++;
            }
            while (id);

            if (!currentMinimumScore || score < currentMinimumScore)
            {
                currentCandidate = tagId;
            }
        }
        return currentCandidate;
    }
}

QString NepomukService::tagnameForNepomukTag(const QUrl& tagUri) const
{
    if (!tagUri.isEmpty())
    {
        Nepomuk::Tag tag(tagUri);

        if (tag.isValid())
        {
            return tag.genericLabel();
        }
        else
        {
            kWarning() << "invalid tag" << tagUri;
        }
    }

    return QString();
}

// ------------------- Utilities ------------------------

QDateTime NepomukService::lastSyncToDigikam() const
{
    QString timeString = DatabaseAccess().db()->getSetting("SyncNepomukToDigikam-1-Time");

    if (!timeString.isNull())
    {
        return QDateTime::fromString(timeString, Qt::ISODate);
    }

    return QDateTime();
}

bool NepomukService::hasSyncToNepomuk()
{
    return DatabaseAccess().db()->getSetting("InitialSyncDigikamToNepomuk-1") == "yes";
}

void NepomukService::markAsSyncedToDigikam()
{
    DatabaseAccess().db()->setSetting("SyncNepomukToDigikam-1-Time", QDateTime::currentDateTime().toString(Qt::ISODate));
}

void NepomukService::markAsSyncedToNepomuk()
{
    DatabaseAccess().db()->setSetting("InitialSyncDigikamToNepomuk-1", "yes");
}

void NepomukService::clearSyncedToDigikam()
{
    DatabaseAccess().db()->setSetting("SyncNepomukToDigikam-1-Time", QString());
}

void NepomukService::clearSyncedToNepomuk()
{
    DatabaseAccess().db()->setSetting("InitialSyncDigikamToNepomuk-1", QString());
}

DatabaseParameters NepomukService::databaseParameters() const
{
    // Check running digikam instance first
    QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply       = interface->registeredServiceNames();

    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        QLatin1String digikamService("org.kde.digikam-");
        foreach (const QString& service, serviceNames)
        {
            if (service.startsWith(digikamService))
            {
                QDBusInterface interface(service, "/Digikam", "org.kde.digikam");

                if (interface.isValid())
                {
                    QDBusReply<QString> paramReply = interface.call("currentDatabaseParameters");

                    if (paramReply.isValid())
                    {
                        KUrl url(paramReply.value());
                        kDebug() << "Got database params from running instance:" << url;
                        return DatabaseParameters(url);
                    }
                }
            }
        }
    }

    // no running instance, read settings file
    KSharedConfig::Ptr config = digikamConfig();
    DatabaseParameters params = DatabaseParameters::parametersFromConfig(config);

    if (!params.databaseName.isEmpty())
    {
        kDebug() << "Using database path from config file:" << params;
        return params;
    }

    return DatabaseParameters();
}

KSharedConfig::Ptr NepomukService::digikamConfig() const
{
    return KSharedConfig::openConfig(KComponentData("digikam", QByteArray(),
                                                    KComponentData::SkipMainComponentRegistration));
}

} // namespace Digikam

NEPOMUK_EXPORT_SERVICE(Digikam::NepomukService, "digikamnepomukservice")
