/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qfile.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <kmimetype.h>
#include <klocale.h>
#include <kiconloader.h>

extern "C"
{
#include <time.h>
}

#include "cameraui.h"
#include "gpiteminfo.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "renamecustomizer.h"

CameraIconView::CameraIconView(CameraUI* ui, QWidget* parent)
    : ThumbView(parent), m_renamer(0), m_ui(ui)
{
    setHScrollBarMode(QScrollView::AlwaysOff);

    CameraIconViewItem::m_newEmblem = new QPixmap(CameraIconViewItem::new_xpm);
    
    connect(this, SIGNAL(signalSelectionChanged()),
            SLOT(slotSelectionChanged()));
    connect(this, SIGNAL(signalRightButtonClicked(ThumbItem*, const QPoint&)),
            SLOT(slotContextMenu(ThumbItem*, const QPoint&)));
    connect(this, SIGNAL(signalDoubleClicked(ThumbItem*)),
            SLOT(slotDoubleClicked(ThumbItem*)));
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
            SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    KMimeType::Ptr mime;
    mime = KMimeType::mimeType( info.mime );
    QPixmap pix = mime->pixmap( KIcon::Desktop, 100, KIcon::DefaultState);

    QString downloadName;
    if (m_renamer && !m_renamer->useDefault())
    {
        downloadName = getTemplatedName( m_renamer->nameTemplate(), &info,
                                         m_itemDict.count() );
    }

    CameraIconViewItem* item = new CameraIconViewItem(this, info, pix, downloadName);
    m_itemDict.insert(info.folder+info.name, item);
}

void CameraIconView::removeItem(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = m_itemDict.find(folder+file);
    if (!item)
        return;

    delete item;
    triggerUpdate();
}

CameraIconViewItem* CameraIconView::findItem(const QString& folder, const QString& file)
{
    return m_itemDict.find(folder+file);
}

void CameraIconView::setThumbnail(const QString& folder, const QString& filename,
                                  const QPixmap& pixmap)
{
    CameraIconViewItem* item = m_itemDict.find(folder+filename);
    if (!item)
        return;

    item->setPixmap(pixmap);
    item->repaint();
}

void CameraIconView::slotDownloadNameChanged()
{
    bool    useDefault   = true;
    QString nameTemplate;

    if (m_renamer)
    {
        useDefault   = m_renamer->useDefault();
        nameTemplate = m_renamer->nameTemplate();
    }
    
    viewport()->setUpdatesEnabled(false);
    for (ThumbItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem =
            static_cast<CameraIconViewItem*>(item);
        viewItem->setDownloadName( useDefault ? QString::null :
                                   getTemplatedName( nameTemplate,
                                                     viewItem->itemInfo(),
                                                     index(viewItem) ) );
    }
    rearrangeItems();
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QString CameraIconView::getTemplatedName(const QString& templ,
                                         const GPItemInfo* itemInfo,
                                         int position)
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

    dname.sprintf(QFile::encodeName(dname), position+1);
    dname.replace("/","");

    dname += '.';
    dname += ext;
    
    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected = false;
    
    for (ThumbItem* item = firstItem(); item;
         item = item->nextItem())
    {
        if (item->isSelected())
        {
            selected = true;
            break;
        }
    }

    emit signalSelected(selected);
}

void CameraIconView::slotContextMenu(ThumbItem * item, const QPoint&)
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
    menu.insertItem(i18n("Properties"), 1);
    menu.insertItem(SmallIcon("text_block"), i18n("EXIF Information"), 2);
    menu.insertSeparator();
    menu.insertItem(SmallIcon("down"),i18n("Download"), 3);
    menu.insertItem(SmallIcon("editdelete"), i18n("Delete"), 4);

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
        emit signalFileProperties(camItem);
        break;
    }
    case(2):
    {
        emit signalFileExif(camItem);
        break;
    }
    case(3):
    {
        emit signalDownload();
        break;
    }
    case(4):
    {
        emit signalDelete();
        break;
    }
    default:
        break;
    }
}

void CameraIconView::slotDoubleClicked(ThumbItem* item)
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
    for (ThumbItem* item = firstItem(); item;
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

#include "cameraiconview.moc"
