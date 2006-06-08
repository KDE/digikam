/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-18
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
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

// C++ includes.

#include <ctime>

// Qt includes.

#include <qfile.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qfont.h>

// KDE includes.

#include <kmimetype.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes.

#include "cameraui.h"
#include "gpiteminfo.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "renamecustomizer.h"
#include "icongroupitem.h"

namespace Digikam
{

CameraIconView::CameraIconView(CameraUI* ui, QWidget* parent)
              : IconView(parent), m_renamer(0), m_ui(ui),
                m_groupItem(new IconGroupItem(this))
{
    setHScrollBarMode(QScrollView::AlwaysOff);
    setMinimumSize(450, 400);

    CameraIconViewItem::m_newEmblem = new QPixmap(CameraIconViewItem::new_xpm);
    
    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
            
    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint&)),
            this, SLOT(slotContextMenu(IconItem*, const QPoint&)));
            
    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    updateItemRectsPixmap();
}

CameraIconView::~CameraIconView()
{
    clear();
    delete CameraIconViewItem::m_newEmblem;
    CameraIconViewItem::m_newEmblem = 0;
}

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    m_renamer = renamer;
    
    connect(m_renamer, SIGNAL(signalChanged()),
            this, SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    KMimeType::Ptr mime;

    // Just to have a generic image thumb from desktop with KDE < 3.5.0
    mime = KMimeType::mimeType( info.mime == QString("image/x-raw") ? QString("image/tiff") : info.mime );

    QPixmap pix = mime->pixmap( KIcon::Desktop, 100, KIcon::DefaultState);
    QString downloadName;

    if (m_renamer)
    {
        if (!m_renamer->useDefault())
        {
            downloadName = getTemplatedName( m_renamer->nameTemplate(), &info, m_itemDict.count() );
        }
        else
        {
            downloadName = getCasedName( m_renamer->changeCase(), &info);
        }
    }

    CameraIconViewItem* item = new CameraIconViewItem(m_groupItem, info, pix, downloadName);
    m_itemDict.insert(info.folder+info.name, item);
}

void CameraIconView::removeItem(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = m_itemDict.find(folder+file);
    if (!item)
        return;
    m_itemDict.remove(folder+file);

    setDelayedUpdate(true);
    delete item;
    setDelayedUpdate(false);
}

CameraIconViewItem* CameraIconView::findItem(const QString& folder, const QString& file)
{
    return m_itemDict.find(folder+file);
}

void CameraIconView::setThumbnail(const QString& folder, const QString& filename, const QImage& image)
{
    CameraIconViewItem* item = m_itemDict.find(folder+filename);
    if (!item)
        return;

    QPixmap pixmap(image);
    item->setPixmap(pixmap);
    item->repaint();
}

void CameraIconView::slotDownloadNameChanged()
{
    bool useDefault = true;
    QString nameTemplate;

    if (m_renamer)
    {
        useDefault   = m_renamer->useDefault();
        nameTemplate = m_renamer->nameTemplate();
    }
    
    viewport()->setUpdatesEnabled(false);

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);

        QString downloadName;

        if (!useDefault)
            downloadName = getTemplatedName( nameTemplate, viewItem->itemInfo(),
                                             m_groupItem->index(viewItem) );
        else
            downloadName = getCasedName( m_renamer->changeCase(), viewItem->itemInfo() );

        viewItem->setDownloadName( downloadName );
    }
    
    rearrangeItems();
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
    slotSelectionChanged();
}

QString CameraIconView::getTemplatedName(const QString& templ, const GPItemInfo* itemInfo, int position)
{
    if (templ.isEmpty())
        return QString::null;
    
    QString dname(templ);
    
    QString ext = itemInfo->name;
    int pos = ext.findRev('.');
    if (pos < 0)
        ext = "";
    else
        ext = ext.right( ext.length() - pos - 1);

    struct tm* time_tm = ::localtime(&itemInfo->mtime);
    char s[100];
    strftime(s, 100, QFile::encodeName(dname), time_tm);

    dname  = s;
    dname.replace("%s", "");
    
    dname.sprintf(QFile::encodeName(dname), position+1);
    dname.replace("/","");

    dname += '.';
    dname += ext;
    
    return dname;
}

QString CameraIconView::getCasedName(const RenameCustomizer::Case ccase,
                                     const GPItemInfo* itemInfo)
{
    QString dname;

    switch (ccase)
    {
        case(RenameCustomizer::UPPER):
        {
            dname = itemInfo->name.upper();
            break;
        }
        case(RenameCustomizer::LOWER):
        {
            dname = itemInfo->name.lower();
            break;
        }
        default:
            break;
    };

    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected = false;
    CameraIconViewItem* camItem = 0;
    
    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem = static_cast<CameraIconViewItem*>(item);
            selected = true;
            break;
        }
    }

    emit signalSelected(camItem, selected);
}

void CameraIconView::slotContextMenu(IconItem * item, const QPoint&)
{
    if (!item)
        return;

    // don't popup context menu if the camera is busy
    if (m_ui->isBusy())
        return;

    CameraIconViewItem* camItem = static_cast<CameraIconViewItem*>(item);
    
    QPopupMenu menu;
    menu.insertItem(SmallIcon("editimage"), i18n("&View"), 0);
    menu.insertSeparator();
    menu.insertItem(SmallIcon("down"),i18n("Download"), 1);
    menu.insertItem(SmallIcon("editdelete"), i18n("Delete"), 2);

    int result = menu.exec(QCursor::pos());

    switch (result)
    {
        case(0):
        {
            emit signalFileView(camItem);
            break;
        }
        case(1):
        {
            emit signalDownload();
            break;
        }
        case(2):
        {
            emit signalDelete();
            break;
        }
        default:
            break;
    }
}

void CameraIconView::slotDoubleClicked(IconItem* item)
{
    if (!item)
        return;
    
    if (m_ui->isBusy())
        return;

    emit signalFileView(static_cast<CameraIconViewItem*>(item));
}

void CameraIconView::slotSelectAll()
{
    selectAll();    
}

void CameraIconView::slotSelectNone()
{
    clearSelection();
}

void CameraIconView::slotSelectInvert()
{
    invertSelection();
}

void CameraIconView::slotSelectNew()
{
    blockSignals(true);
    clearSelection();

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem =
            static_cast<CameraIconViewItem*>(item);
        if (viewItem->itemInfo()->downloaded == 0)
        {
            viewItem->setSelected(true, false);
        }
    }

    blockSignals(false);
    emit signalSelectionChanged();
}

void CameraIconView::startDrag()
{
}

QRect CameraIconView::itemRect() const
{
    return m_itemRect;
}

void CameraIconView::updateItemRectsPixmap()
{
    const int thumbSize = 128;

    QRect pixRect;
    QRect textRect;
    QRect extraRect;

    pixRect.setWidth(thumbSize);
    pixRect.setHeight(thumbSize);

    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    "XXXXXXXXX"));
    textRect.setWidth(r.width());
    textRect.setHeight(r.height());

    QFont fn(font());
    if (fn.pointSize() > 0)
    {
        fn.setPointSize(QMAX(fn.pointSize()-2, 6));
    }

    fm = QFontMetrics(fn);
    r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                              Qt::AlignHCenter | Qt::AlignTop,
                              "XXXXXXXXX"));
    extraRect.setWidth(r.width());
    extraRect.setHeight(r.height());

    r = QRect();
    r.setWidth(QMAX(QMAX(pixRect.width(), textRect.width()),
                       extraRect.width()) + 4);
    r.setHeight(pixRect.height() +
                textRect.height() +
                extraRect.height() + 4);

    m_itemRect = r;
}

}  // namespace Digikam

#include "cameraiconview.moc"
