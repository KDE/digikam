/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-19
 * Description : Service to sync digikam and nepomuk storages
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamnepomukservice.h"
#include "digikamnepomukservice.moc"

// Qt includes

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QThread>
#include <QTimer>

// KDE includes

#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kpluginfactory.h>
#include <kurl.h>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Service>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Statement>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/XMLSchema>

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
#include "imageinfo.h"
#include "imagelister.h"

namespace Digikam
{

enum WatchedNepomukProperty
{
    NaoRating,
    NaoDescription,
    NaoTags
};

class NepomukServicePriv
{
public:

    NepomukServicePriv()
    {
        syncToDigikam        = false;
        syncToNepomuk        = false;
        isConnected          = false;
        nepomukChangeTimer   = 0;
        cleanIgnoreListTimer = 0;
        fullSyncJobs         = 0;
        changingDB           = false;
        changingNepomuk      = false;
    }

    bool        syncToDigikam;
    bool        syncToNepomuk;
    bool        isConnected;
    QTimer     *nepomukChangeTimer;
    QTimer     *cleanIgnoreListTimer;
    int         fullSyncJobs;

    bool        changingDB;
    bool        changingNepomuk;

    TagInfo::List tagList;
    QMap<int, TagInfo> tagMap;

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

    TagInfo tagInfo(int tagId)
    {
        foreach (const TagInfo &info, tagList)
            if (info.id == tagId)
                return info;
        return TagInfo();
    }

    void triggerSyncToDigikam()
    {
        if (!nepomukChangeTimer->isActive())
            nepomukChangeTimer->start();
    }

};

class ChangingDB
{
public:

    ChangingDB(NepomukServicePriv *d)
        : d(d)
    {
        d->changingDB = true;
    }
    ~ChangingDB()
    {
        d->changingDB = false;
    }
    NepomukServicePriv* const d;
};

class ChangingNepomuk
{
public:

    ChangingNepomuk(NepomukServicePriv *d)
        : d(d)
    {
        d->changingNepomuk = true;
    }
    ~ChangingNepomuk()
    {
        d->changingNepomuk = false;
    }
    NepomukServicePriv* const d;
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
    KConfigGroup group = config->group("Nepomuk Settings");
    // default to false here, regardless of default settings in digikam
    enableSyncToDigikam(group.readEntry("Sync Nepomuk to Digikam", false));
    enableSyncToNepomuk(group.readEntry("Sync Digikam to Nepomuk", false));
}

void NepomukService::enableSyncToDigikam(bool syncToDigikam)
{
    // Controls syncing Nepomuk -> Digikam.
    // Called from readConfig or via DBus (from DigikamApp)

    kDebug(50003) << "Sync to digikam enabled: " << syncToDigikam;
    if (d->syncToDigikam == syncToDigikam)
        return;
    d->syncToDigikam = syncToDigikam;

    if (!d->isConnected)
        connectToDatabase();

    if (!d->isConnected)
        return;

    if (d->syncToDigikam)
    {
        connect(mainModel(), SIGNAL(statementAdded(const Soprano::Statement&)),
                 this, SLOT(slotStatementAdded(const Soprano::Statement&)));

        connect(mainModel(), SIGNAL(statementRemoved(const Soprano::Statement&)),
                 this, SLOT(slotStatementRemoved(const Soprano::Statement&)));
    }
    else
    {
        disconnect(mainModel(), SIGNAL(statementAdded(const Soprano::Statement&)),
                    this, SLOT(slotStatementAdded(const Soprano::Statement&)));

        disconnect(mainModel(), SIGNAL(statementRemoved(const Soprano::Statement&)),
                    this, SLOT(slotStatementRemoved(const Soprano::Statement&)));
    }

    if (lastSyncToDigikam().isNull())
        d->triggerSyncToDigikam();
}

void NepomukService::enableSyncToNepomuk(bool syncToNepomuk)
{
     // Controls syncing Digikam -> Nepomuk.
    // Called from readConfig or via DBus (from DigikamApp)

    kDebug(50003) << "Sync to nepomuk enabled:" << syncToNepomuk;
    if (d->syncToNepomuk == syncToNepomuk)
        return;
    d->syncToNepomuk = syncToNepomuk;

    if (!d->isConnected)
        connectToDatabase();

    if (!d->isConnected)
        return;

    if (d->syncToNepomuk)
    {
        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset &)),
                this, SLOT(slotImageChange(const ImageChangeset &)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset &)),
                this, SLOT(slotImageTagChange(const ImageTagChangeset &)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset &)),
                this, SLOT(slotTagChange(const TagChangeset &)));

        // initial pushing to Nepomuk?
        if (!hasSyncToNepomuk())
            QTimer::singleShot(1000, this, SLOT(fullSyncDigikamToNepomuk()));
    }
    else
    {
        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(const ImageChangeset &)),
                   this, SLOT(slotImageChange(const ImageChangeset &)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(const ImageTagChangeset &)),
                   this, SLOT(slotImageTagChange(const ImageTagChangeset &)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(const TagChangeset &)),
                   this, SLOT(slotTagChange(const TagChangeset &)));
    }
}

void NepomukService::databaseChanged()
{
    // Called via DBus
    connectToDatabase();
}

void NepomukService::connectToDatabase()
{
    // (Re-)connects to the database

    d->isConnected = false;
    d->tagList.clear();
    d->tagMap.clear();

    DatabaseParameters params = databaseParameters();
    if (params.isValid())
    {
        DatabaseAccess::setParameters(params, DatabaseAccess::MainApplication);

        d->isConnected = DatabaseAccess::checkReadyForUse();
        if (!d->isConnected)
        {
            QString errorMsg = DatabaseAccess().lastError();
            kDebug(50003) << "Failed to initialize database" << params.databaseName;
            return;
        }
    }
}

// ------------------- Sync Digikam -> Nepomuk  ------------------------

void NepomukService::slotImageChange(const ImageChangeset& changeset)
{
    // Receives signals (via DatabaseWatch via DBus) about changes in application

    if (d->changingDB)
        return;

    DatabaseFields::Set changes = changeset.changes();
    int settings = 0;
    if (changes & DatabaseFields::Rating)
        settings |= SyncRating | SyncHasNoRating;
    if (changes & DatabaseFields::Comment)
        settings |= SyncComment;
    if (settings)
    {
        syncToNepomuk(changeset.ids(), (SyncToNepomukSettings)settings);
    }
}

void NepomukService::slotImageTagChange(const ImageTagChangeset& changeset)
{
    // Receives signals (via DatabaseWatch via DBus) about changes in application

    if (d->changingDB)
        return;

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
    d->tagList.clear();
    d->tagMap.clear();
}

void NepomukService::fullSyncDigikamToNepomuk()
{
    // List album root albums of all available collections recursively
    QList<CollectionLocation> collections = CollectionManager::instance()->allAvailableLocations();
    foreach (const CollectionLocation &location, collections)
    {
        DatabaseUrl url = DatabaseUrl::fromAlbumAndName(QString(), "/", location.albumRootPath(), location.id());
        KIO::Job *job = ImageLister::startListJob(url);
        job->addMetaData("listAlbumsRecursively", "true");

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotFullSyncJobResult(KJob*)));

        connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
                this, SLOT(slotFullSyncJobData(KIO::Job*, const QByteArray&)));

        d->fullSyncJobs++;
    }
}

void NepomukService::slotFullSyncJobResult(KJob *job)
{
    Q_UNUSED(job);
    // when initial full sync is done
    if ( (--d->fullSyncJobs) == 0)
        markAsSyncedToNepomuk();
}

void NepomukService::slotFullSyncJobData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

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
        return -1;

    if (nepomukRating % 2)
        return (nepomukRating + 1) / 2;
    else
        return nepomukRating / 2;
}

static int digikamToNepomukRating(int digikamRating)
{
    // Map [-1, 5] -> [0, 10]: 1->2, 2->4, 3->6, 4->8, 5->10 so that there are "full" and not half stars in dolphin
    if (digikamRating == -1)
        return 0;
    return 2*digikamRating;
}

void NepomukService::syncToNepomuk(const QList<qlonglong>& imageIds, SyncToNepomukSettings syncSettings)
{
    QList<ImageInfo> infos;
    foreach (qlonglong imageid, imageIds)
    {
        ImageInfo info(imageid);
        if (!info.isNull())
            infos << info;
    }
    syncToNepomuk(infos, syncSettings);
}

void NepomukService::syncToNepomuk(const QList<ImageInfo>& infos, SyncToNepomukSettings syncSettings)
{
    foreach (const ImageInfo &info, infos)
    {
        ChangingNepomuk changing(d);

        Nepomuk::Resource res(info.fileUrl(), Soprano::Vocabulary::Xesam::File() );

        if (syncSettings&  SyncRating)
        {
            int rating = info.rating();
            if (rating != -1 || syncSettings & SyncHasNoRating)
            {
                //kDebug(50003) << "Setting rating" << info.rating() << res.resourceUri() << res.isValid();
                res.setRating(digikamToNepomukRating(info.rating()));
                d->addIgnoreUri(res.resourceUri(), NaoRating);
            }
        }

        if (syncSettings & SyncComment)
        {
            QString comment = info.comment();
            if (!comment.isEmpty())
            {
                //kDebug(50003) << "Setting comment" << info.comment() << res.resourceUri() << res.isValid();
                res.setDescription(info.comment());
                d->addIgnoreUri(res.resourceUri(), NaoDescription);
            }
        }
    }
}

// For now d->tagList is enough. If we need hierarchy information,
// we need to move the core of AlbumManager to libdigikamdatabase

static Nepomuk::Tag nepomukForDigikamTag(const TagInfo& info)
{
    if (info.id)
    {
        Nepomuk::Tag tag(info.name);
        // from dolphin's panels/information/newtagdialog.cpp
        tag.setLabel(info.name);
        tag.addIdentifier(info.name);

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
        return tag;
    }
    return Nepomuk::Tag();
}

void NepomukService::syncTagsToNepomuk(const QList<qlonglong>& imageIds, const QList<int>& tagIds, bool addOrRemove)
{
    checkTagList();
    foreach (int tagId, tagIds)
    {
        ChangingNepomuk changing(d);
        Nepomuk::Tag tag = nepomukForDigikamTag(d->tagInfo(tagId));
        if (tag.isValid())
        {
            foreach (qlonglong imageId, imageIds)
            {
                ImageInfo info(imageId);
                if (!info.isNull())
                {
                    Nepomuk::Resource res(info.fileUrl(), Soprano::Vocabulary::Xesam::File());

                    if (addOrRemove)
                        res.addTag(tag);
                    else
                        res.removeProperty(Soprano::Vocabulary::NAO::hasTag(), tag);

                    d->addIgnoreUri(res.resourceUri(), NaoTags);
                }
            }
        }
    }
}

void NepomukService::pushTagsToNepomuk(const QList<ImageInfo>& imageInfos)
{
    checkTagList();
    foreach (const ImageInfo &info, imageInfos)
    {
        ChangingNepomuk changing(d);
        if (!info.isNull())
        {
            foreach (int tagId, info.tagIds())
            {
                Nepomuk::Tag tag = nepomukForDigikamTag(d->tagInfo(tagId));
                if (tag.isValid())
                {
                    Nepomuk::Resource res(info.fileUrl(), Soprano::Vocabulary::Xesam::File());

                    res.addTag(tag);

                    d->addIgnoreUri(res.resourceUri(), NaoTags);
                }
            }
        }
    }
}

// ------------------- Sync Nepomuk -> Digikam ------------------------

void NepomukService::slotStatementAdded(const Soprano::Statement& statement)
{

    if (d->changingNepomuk) // no effect currently
        return;

    const Soprano::Node &subject = statement.subject();
    const Soprano::Node &predicate = statement.predicate();
    if (predicate == Soprano::Vocabulary::NAO::numericRating())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoRating))
            return;
        //kDebug(50003) << "Changed rating" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
    else if (predicate == Soprano::Vocabulary::NAO::description())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoDescription))
            return;
        //kDebug(50003) << "Changed comment" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
    else if (predicate == Soprano::Vocabulary::NAO::hasTag())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoTags))
            return;
        //kDebug(50003) << "Added tag" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        d->triggerSyncToDigikam();
    }
}

void NepomukService::slotStatementRemoved(const Soprano::Statement& statement)
{
    Q_UNUSED(statement);

    if (d->changingNepomuk) // no effect currently
        return;

    const Soprano::Node &subject = statement.subject();
    const Soprano::Node &predicate = statement.predicate();

    if (predicate == Soprano::Vocabulary::NAO::hasTag())
    {
        if (d->checkIgnoreUris(subject.uri(), NaoTags))
            return;
        kDebug(50003) << "Removed tag" << subject.toN3() << statement.object().toN3() << d->changingNepomuk;
        Nepomuk::Resource res(subject.uri());
        removeTagInDigikam(res.property(Soprano::Vocabulary::Xesam::url()).toString(), statement.object().uri());
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
                    "PREFIX xesam: <%3> "
                    "SELECT DISTINCT ?path ?value "
                    " WHERE { GRAPH ?g { ?r %4 ?value . } . "
                    " ?r xesam:url ?path ."
                    " ?g nao:created ?t . "
                    " FILTER ( ?t > \"%5\"^^xls:dateTime ) . } ")
                    .arg(Soprano::Vocabulary::NAO::naoNamespace().toString())
                    .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString())
                    .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
                    .arg(predicate)
                    .arg(Soprano::LiteralValue(dateTime).toString());
    /*
    More elegant, for Soprano 2.2.65
                            " FILTER ( ?t > %2 ) . } ")
                            .arg(Soprano::Node::literalToN3(lastSyncDate));

                            .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::Xesam::url())
    */
}

void NepomukService::syncNepomukToDigikam()
{
    // wait until digikam -> nepomuk initial sync, if any, has finished
    if (d->fullSyncJobs)
        d->triggerSyncToDigikam();

    QDateTime lastSyncDate = lastSyncToDigikam();
    if (!lastSyncDate.isValid())
        lastSyncDate = QDateTime::fromTime_t(0);

    QString query;
    KUrl::List fileUrls;
    KUrl fileUrl;
    Soprano::QueryResultIterator it;
    QString pathBinding("path"), valueBinding("value");

    query = nepomukChangeQuery("nao:numericRating", lastSyncDate);
    it = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<int> ratings;
    int rating;
    while( it.next() )
    {
        fileUrl = KUrl(it.binding(pathBinding).uri());
        rating = it.binding(valueBinding).literal().toInt();
        if (!fileUrl.isEmpty() && rating >= 0 && rating <= 10)
        {
            fileUrls << fileUrl;
            ratings << rating;
        }
    }
    syncRatingToDigikam(fileUrls, ratings);

    fileUrls.clear();
    query = nepomukChangeQuery("nao:description", lastSyncDate);
    it = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<QString> comments;
    QString comment;
    while( it.next() )
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
    it = mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
    QList<QUrl> tags;
    QUrl tag;
    while( it.next() )
    {
        fileUrl = KUrl(it.binding(pathBinding).uri());
        tag = it.binding(valueBinding).uri();
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


void NepomukService::syncRatingToDigikam(const KUrl::List& fileUrls, const QList<int>& ratings)
{
    if (fileUrls.isEmpty())
        return;

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
        return;

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
        return;

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
                tagIdsForInfos << tagId;
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
    kDebug() << fileUrl << tag;
    if (fileUrl.isEmpty())
        return;

    ImageInfo info(fileUrl);
    if (info.isNull())
        return;

    QList<int> tags = info.tagIds();
    if (tags.isEmpty())
        return;

    QString tagName = tagnameForNepomukTag(tag);
    QList<int> candidates = candidateDigikamTagsForTagName(tagName);
    kDebug() << tagName << candidates << tags;
    if (candidates.isEmpty())
        return;

    foreach (int candidate, candidates)
    {
        if (tags.contains(candidate))
            info.removeTag(candidate);
    }
}

QList<int> NepomukService::candidateDigikamTagsForTagName(const QString& tagname)
{
    QList<int> candidates;

    if (tagname.isEmpty())
        return candidates;

    checkTagList();

    foreach (const TagInfo &info, d->tagList)
    {
        if (info.name == tagname)
            candidates << info.id;
    }
    return candidates;
}

int NepomukService::bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname)
{
    if (tagname.isEmpty())
        return 0;

    checkTagMap();

    QList<int> candidates = candidateDigikamTagsForTagName(tagname);

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
                return 0;

            int id = tagId;
            int score = 0;
            do {
                const TagInfo &info = d->tagMap.value(id);
                id = info.pid;
                score++;
            } while (id);

            if (!currentMinimumScore || score < currentMinimumScore)
                currentCandidate = tagId;
        }
        return currentCandidate;
    }
}

QString NepomukService::tagnameForNepomukTag(const QUrl& tagUri)
{
    if (!tagUri.isEmpty())
    {
        Nepomuk::Tag tag(tagUri);

        if (tag.isValid())
        {
            return tag.genericLabel();
        }
        else
            kWarning(50003) << "invalid tag" << tagUri;
    }
    return QString();
}

// ------------------- Utilities ------------------------

QDateTime NepomukService::lastSyncToDigikam()
{
    QString timeString = DatabaseAccess().db()->getSetting("SyncNepomukToDigikam-1-Time");
    if (!timeString.isNull())
        return QDateTime::fromString(timeString, Qt::ISODate);
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

void NepomukService::checkTagList()
{
    if (d->tagList.isEmpty())
    {
        d->tagList = DatabaseAccess().db()->scanTags();
    }
}

void NepomukService::checkTagMap()
{
    if (d->tagMap.isEmpty())
    {
        checkTagList();
        foreach (const TagInfo &info, d->tagList)
            d->tagMap[info.id] = info;
    }
}

DatabaseParameters NepomukService::databaseParameters()
{
    // Check running digikam instance first
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply = interface->registeredServiceNames();
    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        QLatin1String digikamService("org.kde.digikam-");
        foreach (const QString &service, serviceNames)
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
                        kDebug(50003) << "Got database params from running instance:" << url;
                        return DatabaseParameters(url);
                    }
                }
            }
        }
    }

    // no running instance, read settings file
    KSharedConfig::Ptr config  = digikamConfig();
    KConfigGroup group = config->group("Album Settings");
    if (group.exists())
    {
        QString dbPath = group.readEntry("Database File Path", QString());
        kDebug(50003) << "Using database path from config file:" << dbPath;
        if (!dbPath.isEmpty())
            return DatabaseParameters::parametersForSQLiteDefaultFile(dbPath);
    }
    return DatabaseParameters();
}

KSharedConfig::Ptr NepomukService::digikamConfig()
{
    return KSharedConfig::openConfig(
                KComponentData("digikam", QByteArray(), KComponentData::SkipMainComponentRegistration)
                                    );
}

} // namespace Digikam

NEPOMUK_EXPORT_SERVICE(Digikam::NepomukService, "digikamnepomukservice")
