/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// KDE includes.

#include <kurl.h>
#include <kpreviewwidgetbase.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImageDialogPrivate;
class ImageDialogPreviewPrivate;

class DIGIKAM_EXPORT ImageDialogPreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:

    ImageDialogPreview(QWidget *parent=0);
    ~ImageDialogPreview();

    QSize sizeHint() const;

public slots:

    void showPreview(const KURL &url);

private slots:

    void showPreview();
    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
    void clearPreview();

private:

    void resizeEvent(QResizeEvent *e);

private:

    class ImageDialogPreviewPrivate *d;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDialog
{

public:

    ImageDialog(QWidget* parent, const KURL &url, bool singleSelect=false, const QString& caption=QString());
    ~ImageDialog();

    KURL       url() const;
    KURL::List urls() const;

    bool    singleSelect() const;
    QString fileformats() const;

    static KURL::List getImageURLs(QWidget* parent, const KURL& url, const QString& caption=QString());
    static KURL getImageURL(QWidget* parent, const KURL& url, const QString& caption=QString());

private:

    ImageDialogPrivate* d;
};

} // namespace Digikam

#endif /* IMAGEDIALOG_H */
