/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-04
 * Description : a message box to manage camera items
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERAMESSAGEBOX_H
#define CAMERAMESSAGEBOX_H

// Qt includes

#include <QWidget>
#include <QTreeWidget>

// KDE includes

#include <kmessagebox.h>
#include <kstandardguiitem.h>

// Local includes

#include "camerathumbsctrl.h"

namespace Digikam
{

class CameraItem : public QTreeWidgetItem
{

public:

    CameraItem(QTreeWidget* parent, const CamItemInfo& info);
    virtual ~CameraItem();

    bool hasValidThumbnail() const;
    CamItemInfo info() const;

    void setThumb(const QPixmap& pix, bool hasThumb=true);

private:

    class CameraItemPriv;
    CameraItemPriv* const d;
};

// -----------------------------------------------------------

class CameraItemList : public QTreeWidget
{
    Q_OBJECT

public:

    CameraItemList(QWidget* parent=0);
    virtual ~CameraItemList();

    void setThumbCtrl(CameraThumbsCtrl* ctrl);
    void setItems(const CamItemInfoList& items);

private :

    void drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const;

private Q_SLOTS:

    void slotThumbnailLoaded(const CamItemInfo&);

private:

    class CameraItemListPriv;
    CameraItemListPriv* const d;
};

// -----------------------------------------------------------

class CameraMessageBox
{

public:

    static void informationList(CameraThumbsCtrl* ctrl,
                                QWidget *parent,
                                const QString& text,
                                const CamItemInfoList& items,
                                const QString& caption = QString(),
                                const QString& dontShowAgainName = QString());

    static int warningContinueCancelList(CameraThumbsCtrl* ctrl,
                                         QWidget* parent,
                                         const QString& text,
                                         const CamItemInfoList& items,
                                         const QString& caption = QString(),
                                         const KGuiItem& buttonContinue = KStandardGuiItem::cont(),
                                         const KGuiItem& buttonCancel = KStandardGuiItem::cancel(),
                                         const QString& dontAskAgainName = QString());
    
    static int createMessageBox(CameraThumbsCtrl* ctrl,
                                KDialog* dialog,
                                const QIcon& icon,
                                const QString& text,
                                const CamItemInfoList& items,
                                const QString& ask, 
                                bool* checkboxReturn,
                                KMessageBox::Options options,
                                const QString& details=QString(),
                                QMessageBox::Icon notifyType=QMessageBox::Information
                               );
};

} // namespace Digikam

#endif // CAMERAMESSAGEBOX_H
