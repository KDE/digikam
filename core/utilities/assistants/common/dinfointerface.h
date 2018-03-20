/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permit to re-use tools on Showfoto.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DINFO_INTERFACE_H
#define DINFO_INTERFACE_H

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

public:

    // Low level items and albums methods

    virtual QList<QUrl> currentSelectedItems()            const;
    virtual QList<QUrl> currentAlbumItems()               const;

    virtual QList<QUrl> albumItems(int)                   const;
    virtual QList<QUrl> albumsItems(const DAlbumIDs&)     const;
    virtual QList<QUrl> allAlbumItems()                   const;

    virtual DInfoMap    albumInfo(int)                    const;
    virtual DInfoMap    itemInfo(const QUrl&)             const;

public:

    // Albums chooser view methods (to use items from albums before to process).

    virtual QWidget*  albumChooser(QWidget* const parent) const;
    virtual DAlbumIDs albumChooserItems()                 const;
    virtual bool      supportAlbums()                     const;

    Q_SIGNAL void signalAlbumChooserSelectionChanged();

public:

    // Album selector view methods (to upload items from an external place).

    virtual QWidget* uploadWidget(QWidget* const parent) const;
    virtual QUrl     uploadUrl()                  const;

    Q_SIGNAL void signalUploadUrlChanged();
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT DItemInfo
{

public:

    explicit DItemInfo(const DInfoInterface::DInfoMap&);
    ~DItemInfo();

public:

    QString            name()         const;
    QString            comment()      const;
    QString            title()        const;
    int                orientation()  const;
    QSize              dimensions()   const;
    QDateTime          dateTime()     const;
    QStringList        tagsPath()     const;
    QStringList        keywords()     const;
    int                rating()       const;
    int                colorLabel()   const;
    int                pickLabel()    const;
    double             latitude()     const;
    double             longitude()    const;
    double             altitude()     const;
    qlonglong          fileSize()     const;
    QStringList        creators()     const;
    QString            credit()       const;
    QString            rights()       const;
    QString            source()       const;
    QString            exposureTime() const;
    QString            sensitivity()  const;
    QString            aperture()     const;
    QString            focalLength()  const;

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

#endif // DINFO_INTERFACE_H
