/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright 2006 by Gilles Caulier
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

// KDE includes.

#include <klocale.h>
#include <kcursor.h>

// Local includes.

#include "imagepreviewwidget.h"
#include "albumiconview.h"
#include "albumwidgetstack.h"

namespace Digikam
{

class AlbumWidgetStackPriv
{

public:

    AlbumWidgetStackPriv()
    {
        previewItemWidget  = 0;
        previewAlbumWidget = 0;
    }

    ImagePreviewWidget *previewItemWidget;

    AlbumIconView      *previewAlbumWidget;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new AlbumWidgetStackPriv;

    d->previewItemWidget  = new ImagePreviewWidget(this);
    d->previewAlbumWidget = new AlbumIconView(this);

    addWidget(d->previewItemWidget,  PreviewItemMode);
    addWidget(d->previewAlbumWidget, PreviewAlbumMode);

    setPreviewMode(PreviewAlbumMode);
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

AlbumIconView* AlbumWidgetStack::albumIconView()
{
    return d->previewAlbumWidget;
}

ImagePreviewWidget* AlbumWidgetStack::imagePreviewWidget()
{
    return d->previewItemWidget;
}

void AlbumWidgetStack::setPreviewItem(const QString& path)
{
    if (previewMode() == PreviewItemMode)
        d->previewItemWidget->setImagePath(path);
}

int AlbumWidgetStack::previewMode(void)
{
    return id(visibleWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewItemMode)
        return;

    raiseWidget(mode);
    visibleWidget()->setFocus();
}

}  // namespace Digikam

#include "albumwidgetstack.moc"
