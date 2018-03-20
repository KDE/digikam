/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CollectionLocation;
class AlbumRootChangeset;
class CollectionManagerPrivate;

class DIGIKAM_DATABASE_EXPORT CollectionManager : public QObject
{
    Q_OBJECT

public:

    static CollectionManager* instance();
    static void cleanUp();

    /**
     * Disables the collection watch.
     * It will be reenabled as soon as refresh() is called
     * or any other action triggered.
     */
    void setWatchDisabled();

    /**
     * Clears all locations and re-reads the lists of collection locations.
     * Enables the watch.
     */
    void refresh();

    /** CollectionLocation objects returned are simple data containers.
     *  If the corresponding location is returned, the data is still safe to access,
     *  but does not represent anything.
     *  Therefore, do not store returned objects, but prefer to retrieve them freshly.
     */

    /**
     * Add the given file system location as new collection location.
     * Type and availability will be detected.
     * On failure returns null. This would be the case if the given
     * url is already contained in another collection location.
     * You may pass an optional user-visible label that will be stored in the database.
     * The label has no further meaning and can be freely chosen.
     */
    CollectionLocation addLocation(const QUrl& fileUrl, const QString& label = QString());
    CollectionLocation addNetworkLocation(const QUrl& fileUrl, const QString& label = QString());

    enum LocationCheckResult
    {
        /// The check did not succeed, status unknown
        LocationInvalidCheck,
        /// All right. The accompanying message may be empty.
        LocationAllRight,
        /// Location can be added, but the user should be aware of a problem
        LocationHasProblems,
        /// Adding the location will fail (e.g. there is already a location for the path)
        LocationNotAllowed
    };

    /**
     * Analyzes the given file path. Creates an info message
     * describing the result of identification or possible problems.
     * The text is i18n'ed and can be presented to the user.
     * The returned result enum describes the test result.
     */
    LocationCheckResult checkLocation(const QUrl& fileUrl, QList<CollectionLocation> assumeDeleted,
                                      QString* message = 0, QString* suggestedMessageIconName = 0);
    LocationCheckResult checkNetworkLocation(const QUrl& fileUrl, QList<CollectionLocation> assumeDeleted,
                                             QString* message = 0, QString* suggestedMessageIconName = 0);

    /**
     * Removes the given location. This means that all images contained on the
     * location will be removed from the database, all tags will be lost.
     */
    void removeLocation(const CollectionLocation& location);

    /**
     * Sets the label of the given location
     */
    void setLabel(const CollectionLocation& location, const QString& label);

    /**
     * Changes the CollectionLocation::Type of the given location
     */
    void changeType(const CollectionLocation& location, int type);

    /**
     * Checks the locations of type HardWired. If one of these is not available currently,
     * it is added to the list of disappared locations.
     * This case may happen if a file system is changed, a backup restored or other actions
     * taken that change the UUID, although the data may still be available and mounted.
     * If there are hard-wired volumes available which are candidates for a newly appeared
     * volume (in fact those that do not contain any collections currently), they are
     * added to the map, identifier -> i18n'ed user presentable description.
     * The identifier can be used for changeVolume.
     */
    QList<CollectionLocation> checkHardWiredLocations();

    /**
     * For a given disappeared location (retrieved from checkHardWiredLocations())
     * retrieve a user-presentable technical description (excluding the CollectionLocation's label)
     * and a list of identifiers and corresponding user presentable strings of candidates
     * to where the given location may have been moved.
     */
    void migrationCandidates(const CollectionLocation& disappearedLocation,
                             QString* const technicalDescription,
                             QStringList* const candidateIdentifiers,
                             QStringList* const candidateDescriptions);

    /**
     * Migrates the existing collection to a new volume, identified by an internal identifier
     * as returned by checkHardWiredLocations().
     * Use this _only_ to react to changes like those detailed for checkHardWiredLocations;
     * the actual data pointed to shall be unchanged.
     */
    void migrateToVolume(const CollectionLocation& location, const QString& identifier);

    /**
     * Returns a list of all CollectionLocations stored in the database
     */
    QList<CollectionLocation> allLocations();
    /**
     * Returns a list of all currently available CollectionLocations
     */
    QList<CollectionLocation> allAvailableLocations();
    /**
     * Returns a list of the paths of all currently available CollectionLocations
     */
    QStringList allAvailableAlbumRootPaths();

    /**
     * Returns the location for the given album root id
     */
    CollectionLocation locationForAlbumRootId(int id);

    /**
     * Returns the CollectionLocation that contains the given album root.
     * The path must be an album root with isAlbumRoot() == true.
     * Returns 0 if no collection location matches.
     * Only available (or hidden, but available) locations are guaranteed to be found.
     */
    CollectionLocation locationForAlbumRoot(const QUrl& fileUrl);
    CollectionLocation locationForAlbumRootPath(const QString& albumRootPath);

    /**
     * Returns the CollectionLocation that contains the given path.
     * Equivalent to calling locationForAlbumRoot(albumRoot(fileUrl)).
     * Only available (or hidden, but available) locations are guaranteed to be found.
     */
    CollectionLocation locationForUrl(const QUrl& fileUrl);
    CollectionLocation locationForPath(const QString& filePath);

    /**
     * Returns the album root path for the location with the given id.
     * Returns a null QString if the location does not exist or is not available.
     */
    QString albumRootPath(int id);

    /**
     * Returns the album root label for the location with the given id.
     * Returns a null QString if the location does not exist or is not available.
     */
    QString albumRootLabel(int id);

    /**
     * For a given path, the part of the path that forms the album root is returned,
     * ending without a slash. Example: "/media/fotos/Paris 2007" gives "/media/fotos".
     * Only available (or hidden, but available) album roots are guaranteed to be found.
     */
    QUrl    albumRoot(const QUrl& fileUrl);
    QString albumRootPath(const QUrl& fileUrl);
    QString albumRootPath(const QString& filePath);
    /**
     * Returns true if the given path forms an album root.
     * It will return false if the path is a path below an album root,
     * or if the path does not belong to an album root.
     * Example: "/media/fotos/Paris 2007" is an album with album root "/media/fotos".
     *          "/media/fotos" returns true, "/media/fotos/Paris 2007" and "/media" return false.
     * Only available (or hidden, but available) album roots are guaranteed to be found.
     */
    bool    isAlbumRoot(const QUrl& fileUrl);
    /// the file path should not end with the directory slash. Using CoreDbUrl's method is fine.
    bool    isAlbumRoot(const QString& filePath);

    /**
     * Returns the album part of the given file path, i.e. the album root path
     * at the beginning is removed and the second part, starting with "/", ending without a slash,
     * is returned. Example: "/media/fotos/Paris 2007" gives "/Paris 2007"
     * Returns a null QString if the file path is not located in an album root.
     * Returns "/" if the file path is an album root.
     * Note that trailing slashes are removed in the return value, regardless if there was
     * one or not.
     * Note that you have to feed a path/url pointing to a directory. File names cannot
     * be recognized as such by this method, and will be treated as a directory.
     */
    QString album(const QUrl& fileUrl);
    QString album(const QString& filePath);
    QString album(const CollectionLocation& location, const QUrl& fileUrl);
    QString album(const CollectionLocation& location, const QString& filePath);

    /**
     * Returns just one album root, out of the list of available location,
     * the one that is most suitable to serve as a default, e.g.
     * to suggest as default place when the user wants to add files.
     */
    QUrl oneAlbumRoot();
    QString oneAlbumRootPath();

Q_SIGNALS:

    /** Emitted when the status of a collection location changed.
     *  This means that the location became available, hidden or unavailable.
     *
     *  An added location will change its status after addition,
     *  from Null to Available, Hidden or Unavailable.
     *
     *  A removed location will change its status to Deleted
     *  during the removal; in this case, you shall not use the object
     *  passed with this signal with any method of CollectionManager.
     *
     *  The second signal argument is of type CollectionLocation::Status
     *  and describes the status before the state change occurred
     */
    void locationStatusChanged(const CollectionLocation& location, int oldStatus);

    /** Emitted when the label of a collection location is changed */
    void locationPropertiesChanged(const CollectionLocation& location);

private Q_SLOTS:

    void deviceAdded(const QString&);
    void deviceRemoved(const QString&);
    void accessibilityChanged(bool, const QString&);
    void slotAlbumRootChange(const AlbumRootChangeset& changeset);

private:

    CollectionManager();
    ~CollectionManager();

    static CollectionManager* m_instance;
    void updateLocations();

    void clear_locked();

    Q_PRIVATE_SLOT(d, void slotTriggerUpdateVolumesList())

    CollectionManagerPrivate* const d;
    friend class CollectionManagerPrivate;
    friend class CoreDbWatch;
    friend class CoreDbAccess;

Q_SIGNALS: // private

    void triggerUpdateVolumesList();
};

}  // namespace Digikam

#endif // COLLECTIONMANAGER_H
