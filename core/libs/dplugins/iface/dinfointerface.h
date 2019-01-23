/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image information.
 *               This class do not depend of digiKam database library
 *               to permit to re-use tools on Showfoto.
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DINFO_INTERFACE_H
#define DIGIKAM_DINFO_INTERFACE_H

// Qt includes

#include <QMap>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QSize>
#include <QList>
#include <QDateTime>
#include <QDate>
#include <QAbstractItemModel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DInfoInterface : public QObject
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> DInfoMap;  // Map of properties name and value.
    typedef QList<int>              DAlbumIDs; // List of Album ids.

public:

    explicit DInfoInterface(QObject* const parent);
    ~DInfoInterface();

    // Slot to call when date time stamp from item is changed.
    Q_SLOT virtual void slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate);

    // Slot to call when something in metadata from item is changed.
    Q_SLOT virtual void slotMetadataChangedForUrl(const QUrl& url);

public:

    // Low level items and albums methods

    virtual QList<QUrl> currentSelectedItems()                 const;
    virtual QList<QUrl> currentAlbumItems()                    const;

    virtual QList<QUrl> albumItems(int)                        const;
    virtual QList<QUrl> albumsItems(const DAlbumIDs&)          const;
    virtual QList<QUrl> allAlbumItems()                        const;

    virtual DInfoMap albumInfo(int)                            const;
    virtual void     setAlbumInfo(int, const DInfoMap&)        const;

    virtual DInfoMap itemInfo(const QUrl&)                     const;
    virtual void     setItemInfo(const QUrl&, const DInfoMap&) const;

public:

    // Albums chooser view methods (to use items from albums before to process).

    virtual QWidget*  albumChooser(QWidget* const parent) const;
    virtual DAlbumIDs albumChooserItems()                 const;
    virtual bool      supportAlbums()                     const;

    Q_SIGNAL void signalAlbumChooserSelectionChanged();

public:

    // Album selector view methods (to upload items from an external place).

    virtual QWidget* uploadWidget(QWidget* const parent) const;
    virtual QUrl     uploadUrl()                         const;

    Q_SIGNAL void signalUploadUrlChanged();

    // Url to upload new items without to use album selector.
    virtual QUrl     defaultUploadUrl()                  const;

    Q_SIGNAL void signalImportedImage(const QUrl&);

public:

    // Return an instance of tag filter model if host application support this feature, else null pointer.
    virtual QAbstractItemModel* tagFilterModel();
};

// -----------------------------------------------------------------

/** DItemInfo is a class to get item information from host application (Showfoto or digiKam)
 *  The interface is re-implemented in host and depend how item infromation must be retrieved (from a database or by file metadata).
 *  The easy way to use this container is given below:
 *
 *  // READ INFO FROM HOST ---------------------------------------------
 *
 *  QUrl                     itemUrl;                                   // The item url that you want to retrieve information.
 *  DInfoInterface*          hostIface;                                 // The host application interface instance.
 *
 *  DInfoInterface::DInfoMap info = hostIface->itemInfo(itemUrl);       // First stage is to get the information map from host application.
 *  DItemInfo item(info);                                               // Second stage, is to create the DIntenInfo instance for this item by url.
 *  QString   title       = item.name();                                // Now you can retrieve the title,
 *  QString   description = item.comment();                             // The comment,
 *  QDateTime time        = item.dateTime();                            // The time stamp, etc.
 *
 *  // WRITE INFO TO HOST ----------------------------------------------
 *
 *  QUrl                     itemUrl;                                   // The item url that you want to retrieve information.
 *  DInfoInterface*          hostIface;                                 // The host application interface instance.
 *
 *  DInfoInterface::DInfoMap info;                                      // First stage is to create an empty information storage map for this item.
 *  DItemInfo item(info);                                               // Second stage, is to create the DIntenInfo instance for this item.
 *  item.setRating(3);                                                  // Store rating to info map.
 *  item.setColorLabel(1);                                              // Store color label to info map.
 *  hostIface->setItemInfo(url, info);                                  // Update item information to host using map.
 */

class DIGIKAM_EXPORT DItemInfo
{

public:

    explicit DItemInfo(const DInfoInterface::DInfoMap&);
    ~DItemInfo();

public:

    QString            name()             const;
    QString            comment()          const;
    QString            title()            const;
    QSize              dimensions()       const;
    QDateTime          dateTime()         const;
    QStringList        tagsPath()         const;
    QStringList        keywords()         const;

    int                orientation()      const;
    void               setOrientation(int);
    int                rating()           const;
    void               setRating(int);
    int                colorLabel()       const;
    void               setColorLabel(int);
    int                pickLabel()        const;
    void               setPickLabel(int);

    double             latitude()         const;
    double             longitude()        const;
    double             altitude()         const;
    qlonglong          fileSize()         const;
    QStringList        creators()         const;
    QString            credit()           const;
    QString            rights()           const;
    QString            source()           const;
    QString            make()             const;
    QString            model()            const;
    QString            exposureTime()     const;
    QString            sensitivity()      const;
    QString            aperture()         const;
    QString            focalLength()      const;
    QString            focalLength35mm()  const;
    QString            videoCodec()       const;

    bool hasGeolocationInfo() const;

private:

    QVariant parseInfoMap(const QString& key) const;

private:

    DInfoInterface::DInfoMap m_info;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT DAlbumInfo
{

public:

    explicit DAlbumInfo(const DInfoInterface::DInfoMap&);
    ~DAlbumInfo();

public:

    QString title()   const;
    QString caption() const;
    QDate   date()    const;
    QString path()    const;

private:

    DInfoInterface::DInfoMap m_info;
};

} // namespace Digikam

#endif // DIGIKAM_DINFO_INTERFACE_H
