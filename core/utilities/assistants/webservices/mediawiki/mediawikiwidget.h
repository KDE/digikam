/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * Copyright (C) 2013      by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MEDIAWIKI_WIDGET_H
#define MEDIAWIKI_WIDGET_H

// Qt includes

#include <QWidget>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QUrl>

// KDE includes

#include <kconfig.h>

// Local includes

#include "dinfointerface.h"
#include "dprogresswdg.h"
#include "dimageslist.h"

namespace Digikam
{

enum MediaWikiDownloadType
{
    MediaWikiMyAlbum = 0,
    MediaWikiFriendAlbum,
    MediaWikiPhotosMe,
    MediaWikiPhotosFriend
};

class MediaWikiWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MediaWikiWidget(DInfoInterface* const iface, QWidget* const parent);
    ~MediaWikiWidget();

public:

    void updateLabels(const QString& userName = QString(), const QString& wikiName = QString(), const QString& url = QString());
    void invertAccountLoginBox();

    DImagesList*  imagesList()  const;
    DProgressWdg* progressBar() const;

    int  dimension()        const;
    int  quality()          const;
    bool resize()           const;
    bool removeMeta()       const;
    bool removeGeo()        const;

    QString author()        const;
    QString source()        const;
    QString genCategories() const;
    QString genText()       const;
    QString genComments()   const;
    QString license()       const;
    QString categories()    const;
    QString title()         const;
    QString description()   const;
    QString date()          const;
    QString latitude()      const;
    QString longitude()     const;

    QMap <QString, QMap <QString, QString> > allImagesDesc();

    void clearImagesDesc();
    void readSettings(KConfigGroup& group);
    void saveSettings(KConfigGroup& group);
    void loadImageInfoFirstLoad();
    void loadImageInfo(const QUrl& url);
    void clearEditFields();

Q_SIGNALS:

    void signalChangeUserRequest();
    void signalLoginRequest(const QString& login, const QString& pass, const QString& wikiName, const QUrl& wikiUrl);

private Q_SLOTS:

    void slotResizeChecked();
    void slotRemoveMetaChecked();
    void slotChangeUserClicked();
    void slotLoginClicked();
    void slotNewWikiClicked();
    void slotAddWikiClicked();
    void slotLoadImagesDesc(QTreeWidgetItem* item);
    void slotRemoveImagesDesc();
    void slotRestoreExtension();
    void slotApplyTitle();
    void slotApplyDate();
    void slotApplyCategories();
    void slotApplyDescription();
    void slotApplyLatitude();
    void slotApplyLongitude();

private:

    class Private;
    Private* const d;

    friend class WmWindow;
};

} // namespace Digikam

#endif // MEDIAWIKI_WIDGET_H
