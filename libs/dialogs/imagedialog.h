/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kurl.h>
#include <kpreviewwidgetbase.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class LoadingDescription;

class DIGIKAM_EXPORT ImageDialogPreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:

    explicit ImageDialogPreview(QWidget* const parent=0);
    ~ImageDialogPreview();

    QSize sizeHint() const;

public Q_SLOTS:

    void showPreview(const KUrl& url);

private Q_SLOTS:

    void showPreview();
    void slotThumbnail(const LoadingDescription& desc, const QPixmap& pix);
    void clearPreview();

private:

    void resizeEvent(QResizeEvent* e);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDialog
{

public:

    ImageDialog(QWidget* const parent, const KUrl& url, bool singleSelect=false, const QString& caption=QString());
    ~ImageDialog();

    KUrl       url()          const;
    KUrl::List urls()         const;
    bool       singleSelect() const;
    QString    fileFormats()  const;

    static KUrl::List getImageURLs(QWidget* const parent, const KUrl& url, const QString& caption=QString());
    static KUrl getImageURL(QWidget* const parent, const KUrl& url, const QString& caption=QString());

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMAGEDIALOG_H */
