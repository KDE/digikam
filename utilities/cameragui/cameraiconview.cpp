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

// C Ansi includes.

extern "C"
{
#include <time.h>
}

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

    CameraIconViewItem::m_newEmblem = new QPixmap(CameraIconViewItem::new_xpm);
    
    connect(this, SIGNAL(signalSelectionChanged()),
            SLOT(slotSelectionChanged()));
    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint&)),
            SLOT(slotContextMenu(IconItem*, const QPoint&)));
    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            SLOT(slotDoubleClicked(IconItem*)));
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

    if (m_renamer)
    {
        if (!m_renamer->useDefault())
        {
            downloadName = getTemplatedName( m_renamer->nameTemplate(), &info,
                                             m_itemDict.count() );
        }
        else
        {
            downloadName = getCasedName( m_renamer->changeCase(), &info);
        }
    }

    CameraIconViewItem* item = new CameraIconViewItem(m_groupItem, info,
                                                      pix, downloadName);
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
    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem =
            static_cast<CameraIconViewItem*>(item);

        QString downloadName;
        if (!useDefault)
            downloadName = getTemplatedName( nameTemplate,
                                             viewItem->itemInfo(),
                                             m_groupItem->index(viewItem) );
        else
            downloadName = getCasedName( m_renamer->changeCase(),
                                         viewItem->itemInfo() );

        viewItem->setDownloadName( downloadName );
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
    
    for (IconItem* item = firstItem(); item;
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
    if (!firstItem())
        return QRect();

    CameraIconViewItem* item = static_cast<CameraIconViewItem*>(firstItem());

    const int thumbSize = 128;

    QRect pixRect;
    QRect textRect;
    QRect extraRect;

    pixRect.setWidth(thumbSize);
    pixRect.setHeight(thumbSize);
    
    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop |
                                    Qt::WordBreak | Qt::BreakAnywhere,
                                    item->itemInfo()->name));
    textRect.setWidth(r.width());
    textRect.setHeight(r.height());

    if (!item->getDownloadName().isEmpty())
    {
        QFont fn(font());
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));
        }

        fm = QFontMetrics(fn);
        r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                  Qt::AlignHCenter | Qt::WordBreak |
                                  Qt::BreakAnywhere | Qt::AlignTop,
                                  item->getDownloadName()));
        extraRect.setWidth(r.width());
        extraRect.setHeight(r.height());
    }

    r = QRect();
    r.setWidth(QMAX(QMAX(pixRect.width(), textRect.width()),
                       extraRect.width()) + 4);
    r.setHeight(pixRect.height() +
                textRect.height() +
                extraRect.height() + 4);

    return r;
}

}  // namespace Digikam

#include "cameraiconview.moc"
