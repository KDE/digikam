/* ============================================================
 * File  : cameraiconview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
    : QIconView(parent), m_renamer(0), m_ui(ui)
{
    setAutoArrange(true);    
    setSorting(true);
    setItemsMovable(false);
    setResizeMode(Adjust);
    setShowToolTips(false);
    setSpacing(5);
    setAcceptDrops(false);
    setSelectionMode(Extended);
    setHScrollBarMode(QScrollView::AlwaysOff);

    connect(this, SIGNAL(selectionChanged()),
            SLOT(slotSelectionChanged()));
    connect(this, SIGNAL(contextMenuRequested(QIconViewItem*, const QPoint&)),
            SLOT(slotContextMenu(QIconViewItem*, const QPoint&)));
    connect(this, SIGNAL(doubleClicked(QIconViewItem*)),
            SLOT(slotDoubleClicked(QIconViewItem*)));
}

CameraIconView::~CameraIconView()
{
    
}

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    m_renamer = renamer;
    connect(m_renamer, SIGNAL(signalChanged()),
            SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    CameraIconViewItem* item = new CameraIconViewItem(this, info);
    item->setDragEnabled(false);
    m_itemDict.insert(info.folder+info.name, item);

    if (m_renamer && !m_renamer->useDefault())
    {
        item->setDownloadName( getTemplatedName( m_renamer->nameTemplate(),
                                                 item ) );
    }

    KMimeType::Ptr mime;
    mime = KMimeType::mimeType( info.mime );
    QPixmap p = mime->pixmap( KIcon::Desktop, 100, KIcon::DefaultState);
    item->setPixmap(p);
}

void CameraIconView::removeItem(const QString& folder, const QString& file)
{
    QIconViewItem* item = m_itemDict.find(folder+file);
    if (!item)
        return;

    delete item;
    arrangeItemsInGrid();
}

void CameraIconView::setThumbnail(const QString& folder, const QString& filename,
                                  const QPixmap& pixmap)
{
    QIconViewItem* item = m_itemDict.find(folder+filename);
    if (!item)
        return;

    item->setPixmap(pixmap);
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
    for (QIconViewItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem =
            static_cast<CameraIconViewItem*>(item);
        viewItem->setDownloadName( useDefault ? QString::null :
                                   getTemplatedName( nameTemplate, viewItem ) );
    }
    arrangeItemsInGrid();
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QString CameraIconView::getTemplatedName(const QString& templ,
                                         CameraIconViewItem* item)
{
    if (templ.isEmpty())
        return QString::null;
    
    QString dname(templ);
    
    QString ext = item->text();
    int pos = ext.findRev('.');
    if (pos < 0)
        ext = "";
    else
        ext = ext.right( ext.length() - pos - 1);

    struct tm* time_tm = gmtime(&item->m_itemInfo->mtime);
    char* s = new char[100];
    strftime(s, 100, QFile::encodeName(dname), time_tm);
    dname  = s;
    delete [] s;

    dname.sprintf(QFile::encodeName(dname), index(item)+1);
    dname.replace("/","");

    dname += '.';
    dname += ext;
    
    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected = false;
    
    for (QIconViewItem* item = firstItem(); item;
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

void CameraIconView::slotContextMenu(QIconViewItem * item, const QPoint&)
{
    if (!item)
        return;

    // don't popup context menu if the camera is busy
    if (m_ui->isBusy())
        return;

    CameraIconViewItem* camItem = static_cast<CameraIconViewItem*>(item);
    
    QPopupMenu menu;
    menu.insertItem(SmallIcon("editimage"), i18n("View"), 0);
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

void CameraIconView::slotDoubleClicked(QIconViewItem* item)
{
    if (!item)
        return;
    
    if (m_ui->isBusy())
        return;

    emit signalFileView(static_cast<CameraIconViewItem*>(item));
}

#include "cameraiconview.moc"
