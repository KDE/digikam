/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-19
 * Description : Service to sync digikam and nepomuk storages
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamnepomukservice.moc"

// Qt includes
#include <QTimer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QThread>
#include <QPointer>

// KDE includes
#include <kpluginfactory.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

// Nepomuk includes
#include "Nepomuk2/Tag"
#include "Nepomuk2/ResourceManager"
#include "Nepomuk2/Service"

#include "dknepomukwrap.h"
#include "nepomukwatcher.h"
#include "nepomukquery.h"

// Local includes
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

namespace Digikam {

enum WatchedNepomukProperty
{
    NaoRating,
    NaoDescription,
    NaoTags
};

class DkNepomukService::NepomukServicePriv
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
    QPointer<NepomukWatcher>                nepomukWatch;
    QPointer<NepomukQuery>                  nepomukQuery;

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

    explicit ChangingDB(DkNepomukService::NepomukServicePriv* d)
        : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

    DkNepomukService::NepomukServicePriv* const d;
};

class ChangingNepomuk
{
public:

    explicit ChangingNepomuk(DkNepomukService::NepomukServicePriv* d)
        : d(d)
    {
        d->changingNepomuk = true;
    }

    ~ChangingNepomuk()
    {
        d->changingNepomuk = false;
    }

    DkNepomukService::NepomukServicePriv* const d;
};

DkNepomukService::DkNepomukService(QObject* parent, const QVariantList&)
               : Nepomuk2::Service(parent, true), d(new NepomukServicePriv())
{
    Nepomuk2::ResourceManager::instance()->init();

    d->cleanIgnoreListTimer = new QTimer(this);
    d->cleanIgnoreListTimer->setSingleShot(true);
    d->cleanIgnoreListTimer->setInterval(5000);


    connect(d->cleanIgnoreListTimer, SIGNAL(timeout()),
            this, SLOT(cleanIgnoreList()));

    readConfig();

    d->nepomukChangeTimer = new QTimer(this);
    d->nepomukChangeTimer->setSingleShot(true);
    d->nepomukChangeTimer->setInterval(5000);

    connect(d->nepomukChangeTimer, SIGNAL(timeout()),
            this, SLOT(syncNepomukToDigikam()));

}

DkNepomukService::~DkNepomukService()
{
    if(!d->nepomukWatch.isNull())
    {
        kDebug() << "Deleting nepomukWatch";
        delete d->nepomukWatch;
    }
    if(!d->nepomukQuery.isNull())
    {
        kDebug() << "Deleting nepomukQuery";
        delete d->nepomukQuery;
    }
}


void DkNepomukService::getNepomukTags()
{
    QList<Nepomuk2::Tag> tags = Nepomuk2::Tag::allTags();

    kDebug() << "Got" << tags.size() << "tags from Nepomuk";

}

void DkNepomukService::readConfig()
{
    // Reads from digikam config what this service should do

    KSharedConfig::Ptr config = digikamConfig();
    KConfigGroup group        = config->group("Nepomuk Settings");
    // default to false here, regardless of default settings in digikam
    enableSyncToDigikam(group.readEntry("Sync Nepomuk to Digikam", false));
    enableSyncToNepomuk(group.readEntry("Sync Digikam to Nepomuk", false));
}

void DkNepomukService::enableSyncToDigikam(bool syncToDigikam)
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

    /**
     * This class will watch for changes and will keep digiKam database in sync
     * with nepomuk, if enabled
     */
    d->nepomukWatch = new NepomukWatcher(this);

    /**
     * NepomukQuery class will handle all Nepomuk queries
     * from DigikamNepomukService
     */
    d->nepomukQuery = new NepomukQuery(this);
}

void DkNepomukService::enableSyncToNepomuk(bool syncToNepomuk)
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
        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
                this, SLOT(slotImageChange(ImageChangeset)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
                this, SLOT(slotImageTagChange(ImageTagChangeset)));

        connect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
                this, SLOT(slotTagChange(TagChangeset)));

        // initial pushing to Nepomuk?
        if (!hasSyncToNepomuk())
        {
            QTimer::singleShot(1000, this, SLOT(fullSyncDigikamToNepomuk()));
        }
    }
    else
    {
        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
                   this, SLOT(slotImageChange(ImageChangeset)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
                   this, SLOT(slotImageTagChange(ImageTagChangeset)));

        disconnect(DatabaseAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
                   this, SLOT(slotTagChange(TagChangeset)));
    }
}

void DkNepomukService::cleanIgnoreList()
{
    d->ignoreUris.clear();
}


void DkNepomukService::triggerResync(bool toDigikam, bool toNepomuk)
{
    if (!d->isConnected)
    {
        return;
    }

    kDebug() << " Triggered Resync";

    clearSyncedToDigikam();
    clearSyncedToNepomuk();

    if (d->syncToNepomuk && toNepomuk)
    {
        fullSyncDigikamToNepomuk();
    }

    if (d->syncToDigikam && toDigikam)
    {
        d->triggerSyncToDigikam();
    }
}

void DkNepomukService::setDatabase(const QString& paramsUrl)
{
    // Called via DBus
    if (!d->syncToDigikam && !d->syncToNepomuk)
    {
        return;
    }

    KUrl url(paramsUrl);
    kDebug() << "Got database params pushed from running instance:" << url;
    connectToDatabase(DatabaseParameters(url));

    connect(TagsCache::instance(), SIGNAL(tagAboutToBeDeleted(QString)),
            this, SLOT(slotTagDeleted(QString)));
}

void DkNepomukService::connectToDatabase(const DatabaseParameters& params)
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

void DkNepomukService::slotImageChange(const ImageChangeset& changeset)
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

void DkNepomukService::slotImageTagChange(const ImageTagChangeset& changeset)
{
    // Receives signals (via DatabaseWatch via DBus) about changes in application

    if (d->changingDB)
    {
        return;
    }

    kDebug() << changeset.ids() << changeset.tags() << (changeset.operation() == ImageTagChangeset::Added);

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

void DkNepomukService::slotTagChange(const TagChangeset& change)
{
    switch(change.operation())
    {
        case TagChangeset::Added:
            //NOTE: Add them to ignore list???
            DkNepomukWrap::digikamToNepomukTag(change.tagId());
            break;
        case TagChangeset::Renamed:
            break;
        default:
            break;
    }
    //DkNepomukWrap::renameNepomuk
}

void DkNepomukService::slotTagDeleted(QString name)
{
    DkNepomukWrap::removeTag(name);
}

void DkNepomukService::fullSyncDigikamToNepomuk()
{
    // List album root albums of all available collections recursively
    QList<CollectionLocation> collections = CollectionManager::instance()->allAvailableLocations();
    foreach(const CollectionLocation& location, collections)
    {
        DatabaseUrl url = DatabaseUrl::fromAlbumAndName(QString(), "/", location.albumRootPath(), location.id());
        KIO::Job* job = ImageLister::startListJob(url);
        job->addMetaData("listAlbumsRecursively", "true");

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotFullSyncJobResult(KJob*)));

        connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
                this, SLOT(slotFullSyncJobData(KIO::Job*,QByteArray)));

        d->fullSyncJobs++;
    }
}

void DkNepomukService::slotFullSyncJobResult(KJob* job)
{
    Q_UNUSED(job);

    // when initial full sync is done
    if ((--d->fullSyncJobs) == 0)
    {
        markAsSyncedToNepomuk();
    }
}

void DkNepomukService::slotFullSyncJobData(KIO::Job*, const QByteArray& data)
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
    // Map [-1, 5] -> [0, 10]: 1->2, 2->4, 3->6, 4->8, 5->10
    // so that there are "full" and not half stars in dolphin
    if (digikamRating == -1)
    {
        return 0;
    }

    return 2 * digikamRating;
}

void DkNepomukService::syncToNepomuk(const QList<qlonglong>& imageIds, SyncToNepomukSettings syncSettings)
{
    QList<ImageInfo> infos;
    foreach(const qlonglong& imageid, imageIds)
    {
        ImageInfo info(imageid);

        if (!info.isNull())
        {
            infos << info;
        }
    }
    syncToNepomuk(infos, syncSettings);
}

void DkNepomukService::syncToNepomuk(const QList<ImageInfo>& infos, SyncToNepomukSettings syncSettings)
{
    foreach(const ImageInfo& info, infos)
    {
        ChangingNepomuk changing(d);

        Nepomuk2::Resource res(info.fileUrl());

        if (syncSettings & SyncRating)
        {
            int rating = info.rating();

            if (rating != -1 || (syncSettings & SyncHasNoRating))
            {
                kDebug() << "Setting rating" << info.rating() << res.uri() << res.isValid();
                res.setRating(digikamToNepomukRating(info.rating()));
                d->addIgnoreUri(res.uri(), NaoRating);
            }
        }

        if (syncSettings & SyncComment)
        {
            QString comment = info.comment();

            if (!comment.isEmpty())
            {
                kDebug() << "Setting comment" << info.comment() << res.uri() << res.isValid();
                res.setDescription(info.comment());
                d->addIgnoreUri(res.uri(), NaoDescription);
            }
        }
    }
}



void DkNepomukService::syncTagsToNepomuk(const QList<qlonglong>& imageIds,
                                         const QList<int>& tagIds, bool addOrRemove)
{
    foreach(int tagId, tagIds)
    {
        ChangingNepomuk changing(d);
        Nepomuk2::Tag tag = DkNepomukWrap::digikamToNepomukTag(tagId);
        kDebug() << tag.uri();

        if (tag.isValid())
        {
            foreach(const qlonglong& imageId, imageIds)
            {
                ImageInfo info(imageId);

                if (!info.isNull())
                {
                    Nepomuk2::Resource res(info.fileUrl());
                    kDebug() << res.uri() << addOrRemove; //<< res.properties();

                    DkNepomukWrap::setUnsetTag(res,tag, addOrRemove);

                    d->addIgnoreUri(res.uri(), NaoTags);
                    //kDebug() << "after change:" << res.properties();
                }
            }
        }
    }
}

void DkNepomukService::pushTagsToNepomuk(const QList<ImageInfo>& imageInfos)
{
    foreach(const ImageInfo& info, imageInfos)
    {
        ChangingNepomuk changing(d);

        if (!info.isNull())
        {
            foreach(int tagId, info.tagIds())
            {
                Nepomuk2::Tag tag = DkNepomukWrap::digikamToNepomukTag(tagId);

                if (tag.isValid())
                {
                    Nepomuk2::Resource res(info.fileUrl());

                    // the same as res.addTag(tag)
                    DkNepomukWrap::setUnsetTag(res,tag, true);

                    d->addIgnoreUri(res.uri(), NaoTags);
                }
            }
        }
    }
}

// ------------------- Sync Nepomuk -> Digikam ------------------------


void DkNepomukService::syncNepomukToDigikam()
{
    // wait until digikam -> nepomuk initial sync, if any, has finished
    if (d->fullSyncJobs)
    {
        d->triggerSyncToDigikam();
    }

    if(d->nepomukQuery.isNull())
        return;

    QDateTime lastSyncDate = lastSyncToDigikam();

    if (!lastSyncDate.isValid())
    {
        lastSyncDate = QDateTime::fromTime_t(0);
    }


    d->nepomukQuery->queryTags();
    d->nepomukQuery->queryImagesProperties();

    markAsSyncedToDigikam();

}

void DkNepomukService::syncImgRatingToDigikam(const KUrl& fileUrl, int rating)
{

    // If the path is not in digikam collections, info will be null.
    // It does the same check first that we would be doing here
    ImageInfo info(fileUrl);

    if(info.isNull())
    {
        return;
    }
        ChangingDB changing(d);

    DatabaseAccess access;
    DatabaseTransaction transaction(&access);

    info.setRating(nepomukToDigikamRating(rating));

}

void DkNepomukService::syncImgCommentToDigikam(const KUrl& fileUrl, const QString& comment)
{
    // If the path is not in digikam collections, info will be null.
    // It does the same check first that we would be doing here
    ImageInfo info(fileUrl);

    if(info.isNull())
    {
        return;
    }

    ChangingDB changing(d);

    DatabaseAccess access;
    DatabaseTransaction transaction(&access);

    ImageComments comments = info.imageComments(access);
    comments.addComment(comment);
}

void DkNepomukService::syncImgTagsToDigikam(const KUrl& fileUrl, const QList<QUrl>& tags)
{

    QList<int> tagIdsForInfo;
    ImageInfo info(fileUrl);

    // If the path is not in digikam collections, info will be null.
    // It does the same check first that we would be doing here
    if(info.isNull())
    {
        return;
    }

    const int size = tags.size();

    for (int i = 0; i < size; ++i)
    {

        QString tagName = tagnameForNepomukTag(tags.at(i));
        int tagId = bestDigikamTagForTagName(info, tagName);

        if (tagId)
        {
            tagIdsForInfo << tagId;
        }
    }

    if (!tagIdsForInfo.isEmpty())
    {
        ChangingDB changing(d);
        DatabaseAccess access;
        DatabaseTransaction transaction(&access);
        const int infosSize = tagIdsForInfo.size();
        for (int i = 0; i < infosSize; ++i)
        {
            info.setTag(tagIdsForInfo.at(i));
        }
    }
}

void DkNepomukService::removeImgTagInDigikam(const KUrl& fileUrl, const QUrl& tag)
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

    foreach(int candidate, candidates)
    {
        if (tags.contains(candidate))
        {
            info.removeTag(candidate);
        }
    }
}

int DkNepomukService::bestDigikamTagForTagName(const ImageInfo& info, const QString& tagname)
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
        foreach(int tagId, candidates)
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

QString DkNepomukService::tagnameForNepomukTag(const QUrl& tagUri) const
{
    if (!tagUri.isEmpty())
    {
        Nepomuk2::Tag tag(tagUri);

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

void DkNepomukService::addTagInDigikam(const QUrl& tagUrl)
{
    Nepomuk2::Tag tag(tagUrl);
    QString tagName = tag.genericLabel();

    QList<int> existList = TagsCache::instance()->tagsForName(tagName);

    // Tag with the same name exist, do not add anything
    if(!existList.isEmpty())
    {
        return;
    }

    kDebug() << "Adding tag to digikam " << tagName;
    DatabaseAccess().db()->addTag(0, tagName, QString(), 0);
}

// ------------------- Utilities ------------------------

QDateTime DkNepomukService::lastSyncToDigikam() const
{
    QString timeString = DatabaseAccess().db()->getSetting("SyncNepomukToDigikam-1-Time");

    if (!timeString.isNull())
    {
        return QDateTime::fromString(timeString, Qt::ISODate);
    }

    return QDateTime();
}

bool DkNepomukService::hasSyncToNepomuk()
{
    return DatabaseAccess().db()->getSetting("InitialSyncDigikamToNepomuk-1") == "yes";
}

void DkNepomukService::markAsSyncedToDigikam()
{
    DatabaseAccess().db()->setSetting("SyncNepomukToDigikam-1-Time",
                                      QDateTime::currentDateTime().toString(Qt::ISODate));
}

void DkNepomukService::markAsSyncedToNepomuk()
{
    DatabaseAccess().db()->setSetting("InitialSyncDigikamToNepomuk-1", "yes");
}

void DkNepomukService::clearSyncedToDigikam()
{
    DatabaseAccess().db()->setSetting("SyncNepomukToDigikam-1-Time", QString());
}

void DkNepomukService::clearSyncedToNepomuk()
{
    DatabaseAccess().db()->setSetting("InitialSyncDigikamToNepomuk-1", QString());
}

DatabaseParameters DkNepomukService::databaseParameters() const
{
    // Check running digikam instance first
    QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply       = interface->registeredServiceNames();

    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        QLatin1String digikamService("org.kde.digikam-");
        foreach(const QString& service, serviceNames)
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

KSharedConfig::Ptr DkNepomukService::digikamConfig() const
{
    return KSharedConfig::openConfig(KComponentData("digikam", QByteArray(),
                                                    KComponentData::SkipMainComponentRegistration));
}

} // namespace Digikam

NEPOMUK_EXPORT_SERVICE( Digikam::DkNepomukService, "digikamnepomukservice")

