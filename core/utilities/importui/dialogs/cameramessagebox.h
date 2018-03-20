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

#ifndef CAMERAMESSAGEBOX_H
#define CAMERAMESSAGEBOX_H

// Qt includes

#include <QWidget>
#include <QTreeWidget>

// Local includes

#include "camerathumbsctrl.h"
#include "digikam_export.h"

class QDialog;
class QDialogButtonBox;

namespace Digikam
{

class DIGIKAM_EXPORT CameraItem : public QTreeWidgetItem
{

public:

    CameraItem(QTreeWidget* const parent, const CamItemInfo& info);
    virtual ~CameraItem();

    bool hasValidThumbnail() const;
    CamItemInfo info()       const;

    void setThumb(const QPixmap& pix, bool hasThumb = true);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------

class DIGIKAM_EXPORT CameraItemList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit CameraItemList(QWidget* const parent = 0);
    virtual ~CameraItemList();

    void setThumbCtrl(CameraThumbsCtrl* const ctrl);
    void setItems(const CamItemInfoList& items);

private :

    void drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const;

private Q_SLOTS:

    void slotThumbnailLoaded(const CamItemInfo&);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------

class DIGIKAM_EXPORT CameraMessageBox
{

public:

    /** Show List of camera items into an informative message box.
     */
    static void informationList(CameraThumbsCtrl* const ctrl,
                                QWidget* const parent,
                                const QString& caption,
                                const QString& text,
                                const CamItemInfoList& items,
                                const QString& dontShowAgainName = QString());

    /** Show List of camera items to processs into a message box and wait user feedback.
     *  Return QMessageBox::Yes or QMessageBox::Cancel
     */
    static int warningContinueCancelList(CameraThumbsCtrl* const ctrl,
                                         QWidget* const parent,
                                         const QString& caption,
                                         const QString& text,
                                         const CamItemInfoList& items,
                                         const QString& dontAskAgainName = QString());
};

} // namespace Digikam

#endif // CAMERAMESSAGEBOX_H
