/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablepreview.moc"

// Qt includes

#include <QList>
#include <QPainter>
#include <QString>
#include <QFontMetrics>
#include <QPixmap>
#include <QPalette>

// KDE includes

#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "albumdb.h"
#include "ddragobjects.h"
#include "dimg.h"
#include "dimgpreviewitem.h"

namespace Digikam
{

LightTablePreview::LightTablePreview(QWidget* const parent)
    : ImagePreviewView(parent, ImagePreviewView::LightTablePreview)
{
    setDragAndDropEnabled(true);
    showDragAndDropMessage();
}

LightTablePreview::~LightTablePreview()
{
}

void LightTablePreview::setDragAndDropEnabled(bool b)
{
    setAcceptDrops(b);
    viewport()->setAcceptDrops(b);
}

void LightTablePreview::showDragAndDropMessage()
{
    if (acceptDrops())
    {
        QString msg    = i18n("Drag and drop an image here");
        QFontMetrics fontMt(font());
        QRect fontRect = fontMt.boundingRect(0, 0, width(), height(), 0, msg);
        QPixmap pix(fontRect.size());
        pix.fill(kapp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   msg);
        p.end();
        previewItem()->setImage(DImg(pix.toImage()));
    }
}

void LightTablePreview::dragEnterEvent(QDragEnterEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->accept();
    }
}

void LightTablePreview::dragMoveEvent(QDragMoveEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->accept();
    }
}

bool LightTablePreview::dragEventWrapper(const QMimeData* data) const
{
    if (acceptDrops())
    {
        int              albumID;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;
        KUrl::List       urls, kioURLs;

        if (DItemDrag::decode(data, urls, kioURLs, albumIDs, imageIDs) ||
            DAlbumDrag::decode(data, urls, albumID)                    ||
            DTagListDrag::canDecode(data))
        {
            return true;
        }
    }

    return false;
}

void LightTablePreview::dropEvent(QDropEvent* e)
{
    if (acceptDrops())
    {
        int              albumID;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;
        KUrl::List       urls, kioURLs;

        if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            emit signalDroppedItems(ImageInfoList(imageIDs));
            e->accept();
            return;
        }
        else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
        {
            QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);

            emit signalDroppedItems(ImageInfoList(itemIDs));
            e->accept();
            return;
        }
        else if (DTagListDrag::canDecode(e->mimeData()))
        {
            QList<int> tagIDs;

            if (!DTagListDrag::decode(e->mimeData(), tagIDs))
            {
                return;
            }

            QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagIDs.first(), true);
            ImageInfoList imageInfoList;

            emit signalDroppedItems(ImageInfoList(itemIDs));
            e->accept();
            return;
        }
    }
}

}  // namespace Digikam
