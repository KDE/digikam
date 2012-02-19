/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to interface digiKam with kipi library.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KIPIINTERFACE_H
#define KIPIINTERFACE_H

// Qt includes

#include <QList>
#include <QString>
#include <QPixmap>
#include <QVariant>

// KDE includes

#include <kurl.h>

// LibKIPI includes

#include <libkipi/version.h>
#include <libkipi/interface.h>

// Local includes

#include "albummanager.h"
#include "loadingdescription.h"
#include "kipiimagecollectionselector.h"
#include "kipiuploadwidget.h"

namespace KIPI
{
    class ImageInfo;
    class ImageCollection;
    class ImageCollectionSelector;
    class UploadWidget;
}

namespace Digikam
{

class KipiInterface : public KIPI::Interface
{
    Q_OBJECT

public:

    explicit KipiInterface(QObject* parent, const char* name=0);
    ~KipiInterface();

    KIPI::ImageCollection        currentAlbum();
    KIPI::ImageCollection        currentSelection();
    QList<KIPI::ImageCollection> allAlbums();
    KIPI::ImageInfo              info(const KUrl&);

    bool addImage(const KUrl&, QString& errmsg);
    void delImage(const KUrl&);
    void refreshImages(const KUrl::List& urls);

    int  features() const;

    void thumbnail(const KUrl& url, int size);
    void thumbnails(const KUrl::List& list, int size);

    KIPI::ImageCollectionSelector* imageCollectionSelector(QWidget* parent);
    KIPI::UploadWidget*            uploadWidget(QWidget* parent);
    QAbstractItemModel*            getTagTree() const;

    QVariant hostSetting(const QString& settingName);

#if KIPI_VERSION >= 0x010500
    QString progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb) const;
    void    progressValueChanged(const QString& id, float percent);
    void    progressStatusChanged(const QString& id, const QString& status);
    void    progressThumbnailChanged(const QString& id, const QPixmap& thumb);
    void    progressCompleted(const QString& id);
    KIPI::FileReadWriteLock* createReadWriteLock(const KUrl& url);
#endif // KIPI_VERSION >= 0x010500

public Q_SLOTS:

    void slotSelectionChanged(int count);
    void slotCurrentAlbumChanged(Album* palbum);

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class KipiInterfacePrivate;
    KipiInterfacePrivate* const d;
};

}  // namespace Digikam

#endif  // KIPIINTERFACE_H
