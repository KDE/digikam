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

#ifndef DIGIKAMNEPOMUKSERVICE_H
#define DIGIKAMNEPOMUKSERVICE_H

#include <QString>

// KDE includes

#include <ksharedconfig.h>
#include <kurl.h>
#include <Nepomuk2/Service>

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


class DkNepomukService : public Nepomuk2::Service
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DigikamNepomukService")
public:

    DkNepomukService(QObject* parent, const QVariantList&);
    ~DkNepomukService();

    void getNepomukTags();

public Q_SLOTS:

    /**
     *  @brief setDatabase        -  Sets the digikam database to watch.
     *                               The parameter is the url() of a KUrl which
     *                               contains the database parameters serialized
     *                               by DatabaseParameters.
     */
    Q_SCRIPTABLE void setDatabase(const QString& parameters);

    /**
     * @brief enableSyncToDigikam - enables Nepomuk to push image properties
     *                              and tags to digiKam when it's resource
     *                              watcher reports changes
     *                              Q_SCRIPTABLE enables call from D-Bus
     */
    Q_SCRIPTABLE void enableSyncToDigikam(bool syncToDigikam);

    /**
     * @brief enableSyncToDigikam - enables digiKam to push image properties
     *                              and tags to Nepomuk when database emits
     *                              corresponding signals.
     *                              Q_SCRIPTABLE enables call from D-Bus
     */
    Q_SCRIPTABLE void enableSyncToNepomuk(bool syncToNepomuk);

    /**
     * @brief triggerResync       - will trigger both digiKam -> Nepomuk
     *                              and Nepomuk -> digiKam full sync
     *                              Usually called by digiKam via D-bus
     *                              call
     */
    Q_SCRIPTABLE void triggerResync();

    /**
     * @brief syncImgTagsToDigikam   - sync image tags to digiKam database.
     *                                 Called by NepomukWatcher or NepomukQuery
     *                                 class.
     *
     * @param filePath               - path to image to which tags should be added
     *
     * @param tags                   - list of tags that should be added
     *
     */
    void syncImgTagsToDigikam(const KUrl& filePath, const QList<QUrl>& tags);

    /**
     * @brief syncImgRatingToDigikam   - sync image rating to digiKam database.
     *                                   Called by NepomukWatcher of NepomukQuery
     *                                   class.
     * @param filePath                 - path to image to which rating should
     *                                   be added
     *
     * @param rating                   - current rating value
     */
    void syncImgRatingToDigikam(const KUrl& filePath, const int ratings);

    /**
     * @brief syncImgCommentToDigikam - sync image comment to digiKam database.
     *                                  Called by NepomukWatcher of NepomukQuery
     *                                  class.
     * @param filePath                - path to image to which comment should be
     *                                  added
     *
     * @param comment                 - comment to be set
     */
    void syncImgCommentToDigikam(const KUrl& filePath, const QString& comment);

    /**
     * @brief removeImgTagToDigikam    - remove tag from image properties in digiKam
     *                                   database.
     * @param filePath                 - path to image to which tag should be
     *                                   removed
     *
     * @param tag                      - tag to be removed
     */
    void removeImgTagInDigikam(const KUrl& fileUrl, const QUrl& tag);

    /**
     * @brief addTagInDigikam         - add Nepomuk tag into digiKam.
     *                                  All added tags will be listed under
     *                                  /Nepomuk
     */
    void addTagInDigikam(const QUrl& tagUrl);

protected Q_SLOTS:

    /**
     * @brief slotImageChange - sync image comment & rating when images comment
     *                          or rating is changed in digikam
     * @param changeset       - image changeset from digikam database
     */
    void slotImageChange(const ImageChangeset& changeset);

    /**
     * @brief slotImageTagChange  - sync image tags when tags are added or
     *                              removed
     * @param changeset           - Database class that hold info about image
     *                              and tag operations
     */
    void slotImageTagChange(const ImageTagChangeset& changeset);

    /**
     * @brief slotTagChange   - sync tag to Nepomuk when a digikam tag
     *                          is changed: add, rename, delete
     * @param changeset       - Database structure that holds information about
     *                          tag and change operation
     */
    void slotTagChange(const TagChangeset& changeset);

    void slotTagDeleted(QString string);

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

    void syncToNepomuk(const QList<qlonglong>& imageid,
                       SyncToNepomukSettings syncSettings);
    void syncToNepomuk(const QList<ImageInfo>& infos,
                       SyncToNepomukSettings syncSettings);
    void syncTagsToNepomuk(const QList<qlonglong>& imageIds,
                           const QList<int>& tagIds, bool addOrRemove);

    void syncAddedImagesToDigikam(const QList<qlonglong>& ids);

    /**
     * @brief pushTagsToNepomuk   - add images tags from digiKam
     *                              to Nepomuk's resources
     * @param imageInfos          - images that contain tags
     */
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

}
#endif

