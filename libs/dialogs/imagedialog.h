/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

// Qt includes

#include <QUrl>
#include <QScrollArea>
#include <QFileIconProvider>
#include <QIcon>
#include <QFileInfo>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class LoadingDescription;
class ThumbnailImageCatcher;

class DIGIKAM_EXPORT ImageDialogPreview : public QScrollArea
{
    Q_OBJECT

public:

    explicit ImageDialogPreview(QWidget* const parent=0);
    ~ImageDialogPreview();

    QSize sizeHint() const;

public Q_SLOTS:

    void slotShowPreview(const QUrl& url);

private Q_SLOTS:

    void showPreview();
    void slotClearPreview();
    void slotThumbnail(const LoadingDescription& desc, const QPixmap& pix);

private:

    void resizeEvent(QResizeEvent* e);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT DFileIconProvider : public QFileIconProvider
{

public:

    explicit DFileIconProvider();
    ~DFileIconProvider();

    virtual QIcon icon(IconType type) const;
    virtual QIcon icon(const QFileInfo& info) const;
/*
private:

    ThumbnailImageCatcher* m_catcher;
*/
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDialog
{

public:

    ImageDialog(QWidget* const parent, const QUrl& url, bool singleSelect=false, const QString& caption=QString());
    ~ImageDialog();

    QUrl        url()         const;
    QList<QUrl> urls()        const;
    QStringList fileFormats() const;

    static QUrl        getImageURL(QWidget* const parent, const QUrl& url, const QString& caption=QString());
    static QList<QUrl> getImageURLs(QWidget* const parent, const QUrl& url, const QString& caption=QString());

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMAGEDIALOG_H */
