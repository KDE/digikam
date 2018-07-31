/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_WS_IMAGES_PAGE_H
#define DIGIKAM_WS_IMAGES_PAGE_H

// Qt includes

#include <QObject>
#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Local includes

#include "dwizardpage.h"
#include "wsitem.h"

namespace Digikam
{

class WSImagesPage : public DWizardPage
{
    Q_OBJECT
    
public:

    explicit WSImagesPage(QWizard* const dialog, const QString& title);
    ~WSImagesPage();

    void    initializePage();
    bool    validatePage();
    bool    isComplete() const;

    void    setItemsList(const QList<QUrl>& urls);

private:

    void addChildToTreeView(QTreeWidgetItem* const parent,
                            const QMap<QString, AlbumSimplified>& albumTree, 
                            const QStringList& childrenAlbums);
    void setCurrentAlbumId(const QString& currentAlbumId);

private Q_SLOTS:

    void slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree, 
                            const QStringList& rootAlbums,
                            const QString& currentAlbumId);
    void slotReloadListAlbums();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_WS_IMAGES_PAGE_H
