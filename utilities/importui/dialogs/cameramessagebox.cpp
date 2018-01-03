/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-04
 * Description : a message box to manage camera items
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameramessagebox.h"

// Qt includes

#include <QPainter>
#include <QPointer>
#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QIcon>
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmessagebox.h"
#include "digikam_debug.h"

namespace Digikam
{

class CameraItem::Private
{

public:

    Private()
    {
        hasThumb = false;
    }

    bool        hasThumb;

    CamItemInfo info;
};

CameraItem::CameraItem(QTreeWidget* const parent, const CamItemInfo& info)
    : QTreeWidgetItem(parent),
      d(new Private)
{
    d->info = info;
    setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(parent->iconSize().width(), QIcon::Disabled), false);
    setText(1, d->info.name);
}

CameraItem::~CameraItem()
{
    delete d;
}

bool CameraItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

CamItemInfo CameraItem::info() const
{
    return d->info;
}

void CameraItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2)  - (pix.width() / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);

    QIcon icon = QIcon(pixmap);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);

    d->hasThumb = hasThumb;
}

//----------------------------------------------------------------------------

class CameraItemList::Private
{

public:

    Private()
        : iconSize(64)
    {
        ctrl = 0;
    }

    const int         iconSize;

    CameraThumbsCtrl* ctrl;
};

CameraItemList::CameraItemList(QWidget* const parent)
    : QTreeWidget(parent),
      d(new Private)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18n("Thumb") << i18n("File Name"));
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

CameraItemList::~CameraItemList()
{
    delete d;
}

void CameraItemList::setItems(const CamItemInfoList& items)
{
    foreach(CamItemInfo info, items)
    {
        new CameraItem(this, info);
    }
}

void CameraItemList::setThumbCtrl(CameraThumbsCtrl* const ctrl)
{
    d->ctrl = ctrl;

    connect(d->ctrl, SIGNAL(signalThumbInfoReady(CamItemInfo)),
            this, SLOT(slotThumbnailLoaded(CamItemInfo)));
}

void CameraItemList::slotThumbnailLoaded(const CamItemInfo& info)
{
    QTreeWidgetItemIterator it(this);
    bool                    valid;
    CachedItem              citem;

    while (*it)
    {
        CameraItem* const item = dynamic_cast<CameraItem*>(*it);

        if (item && item->info().url() == info.url())
        {
            valid = d->ctrl->getThumbInfo(info, citem);
            item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
            return;
        }

        ++it;
    }
}

void CameraItemList::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    CameraItem* const item = dynamic_cast<CameraItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        CachedItem citem;
        bool valid = d->ctrl->getThumbInfo(item->info(), citem);
        item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
    }

    QTreeWidget::drawRow(p, opt, index);
}

// --------------------------------------------------------------------------------------------------------

void CameraMessageBox::informationList(CameraThumbsCtrl* const ctrl,
                                       QWidget* const parent,
                                       const QString& caption,
                                       const QString& text,
                                       const CamItemInfoList& items,
                                       const QString& dontShowAgainName)
{
    CameraItemList* const listWidget = new CameraItemList();
    listWidget->setThumbCtrl(ctrl);
    listWidget->setItems(items);

    DMessageBox::showInformationWidget(QMessageBox::Information, parent, caption, text, listWidget, dontShowAgainName);
}

int CameraMessageBox::warningContinueCancelList(CameraThumbsCtrl* const ctrl,
                                                QWidget* const parent,
                                                const QString& caption,
                                                const QString& text,
                                                const CamItemInfoList& items,
                                                const QString& dontAskAgainName)
{
    CameraItemList* const listWidget = new CameraItemList();
    listWidget->setThumbCtrl(ctrl);
    listWidget->setItems(items);

    return (DMessageBox::showContinueCancelWidget(QMessageBox::Warning, parent, caption, text, listWidget, dontAskAgainName));
}

} // namespace Digikam
