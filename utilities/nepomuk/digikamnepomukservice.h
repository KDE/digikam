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

#ifndef DIGIKAMNEPOMUKSERVICE_H
#define DIGIKAMNEPOMUKSERVICE_H

// KDE includes

#include <ksharedconfig.h>
#include <kurl.h>
#include <Nepomuk/Service>

// Local includes

class KJob;
namespace KIO
{
class Job;
}

namespace Soprano
{
class Statement;
}

namespace Digikam
{

class CollectionImageChangeset;
class DatabaseParameters;
class ImageInfo;
class ImageChangeset;
class ImageTagChangeset;
class TagChangeset;

class NepomukService : public Nepomuk::Service
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DigikamNepomukService")

public:

    NepomukService(QObject* parent, const QVariantList&);
    ~NepomukService();

public Q_SLOTS:

    /** Sets the digikam database to watch.
     *  The parameter is the url() of a KUrl which contains the database parameters
     *  serialized by DatabaseParameters.
     */
    Q_SCRIPTABLE void setDatabase(const QString& parameters);
    Q_SCRIPTABLE void enableSyncToDigikam(bool syncToDigikam);
    Q_SCRIPTABLE void enableSyncToNepomuk(bool syncToNepomuk);
    Q_SCRIPTABLE void triggerResync();

protected Q_SLOTS:

    void slotImageChange(const ImageChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotTagChange(const TagChangeset& changeset);
    //void slotCollectionImageChange(const CollectionImageChangeset& changeset);

    void slotStatementAdded(const Soprano::Statement& statement);
    void slotStatementRemoved(const Soprano::Statement& statement);

    void syncNepomukToDigikam();
    void fullSyncDigikamToNepomuk();

    void cleanIgnoreList();

    void slotFullSyncJobResult(KJob* job);
    void slotFullSyncJobData(KIO::Job*, const QByteArray& data);

protected:

    void connectToDatabase(const DatabaseParameters& params);

    enum SyncToNepomukSettings
    {
        SyncNothing     = 0x00,
        SyncRating      = 0x01,
        SyncHasNoRating = 0x02,
        SyncComment     = 0x04
    };

    void readConfig();
    void syncToNepomuk(const QList<qlonglong>& imageid, SyncToNepomukSettings syncSettings);
    void syncToNepomuk(const QList<ImageInfo>& infos, SyncToNepomukSettings syncSettings);
    void syncTagsToNepomuk(const QList<qlonglong>& imageIds, const QList<int>& tagIds, bool addOrRemove);
    void syncRatingToDigikam(const KUrl::List& filePaths, const QList<int>& ratings);
    void syncCommentToDigikam(const KUrl::List& filePaths, const QStringList& ratings);
    void syncTagsToDigikam(const KUrl::List& filePaths, const QList<QUrl>& tags);
    void syncAddedImagesToDigikam(const QList<qlonglong>& ids);
    void removeTagInDigikam(const KUrl& fileUrl, const QUrl& tag);
    void pushTagsToNepomuk(const QList<ImageInfo>& imageInfos);

    int bestDigikamTagForTagName(const ImageInfo& info, const QString& tagName);
    void markAsSyncedToDigikam();
    void clearSyncedToDigikam();
    bool hasSyncToNepomuk();
    void markAsSyncedToNepomuk();
    void clearSyncedToNepomuk();
    QString tagnameForNepomukTag(const QUrl& tagUri) const;
    QDateTime lastSyncToDigikam() const;
    DatabaseParameters databaseParameters() const;
    KSharedConfig::Ptr digikamConfig() const;

public:

    // Declared as public due to use in ChangingNepomuk and ChangingDB classes.
    class NepomukServicePriv;

private:

    NepomukServicePriv* const d;
};

} // namespace Digikam

#endif // DIGIKAMNEPOMUKSERVICE_H
