/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-3
 * Description : Showfoto item loader
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTOITEMLOADER_H
#define SHOWFOTOITEMLOADER_H

// Qt includes

#include <QThread>
#include <QString>
#include <QFileInfo>
#include <QCustomEvent>
#include <QPixmap>
#include <QObject>
#include <QFileInfo>

#include "kurl.h"

// Local includes
#include "showfotoiteminfo.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

using namespace Digikam;

namespace ShowFoto {

typedef QPair<ShowfotoItemInfo, QPixmap> CachedItem;

class ShowfotoItemLoader : public QObject
{
    Q_OBJECT
public:
    ShowfotoItemLoader(ThumbnailLoadThread* thumbLoadThread);
    ~ShowfotoItemLoader();

    void openFolder(const KUrl& url);
    void openFile(const KUrl& url);
    QPixmap mimeTypeThumbnail(const QString& itemName, int thumbSize) const;
    bool loadThumbnailForItem(const ShowfotoItemInfo& info, CachedItem& item, ThumbnailSize thumbSize ,bool thumbChanged) const;

Q_SIGNALS:
    void signalNoCurrentItem();
    void signalSorry();
    void signalToggleNav(int i);
    void signalToggleAction(bool);
    void signalSetSelceted(KUrl url);
    void signalSetCurrentItem();
    void signalNewThumbItem(KUrl url);
    void signalLastDirectory(KUrl::List::const_iterator);
    void signalInfoList(ShowfotoItemInfoList&);

public Q_SLOTS:
    void slotLoadCurrentItem(const KUrl::List& urlList);
    void slotOpenFolder(const KUrl& url);
    void slotOpenFile(const KUrl::List &urls);

private:

    class Private;
    Private* const d;
};
} // namespace Showfoto
#endif // ShowfotoItemLoader_H
