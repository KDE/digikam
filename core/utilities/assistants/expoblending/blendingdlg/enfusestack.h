/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#ifndef ENFUSESTACK_H
#define ENFUSESTACK_H

// Qt includes

#include <QTreeWidget>
#include <QString>
#include <QPixmap>
#include <QPoint>
#include <QList>
#include <QIcon>

// Local includes

#include "enfusesettings.h"

namespace Digikam
{

class EnfuseStackItem : public QTreeWidgetItem
{
public:

    explicit EnfuseStackItem(QTreeWidget* const parent);
    virtual ~EnfuseStackItem();

    /** Return the preview image url assigned to item.
     */
    const QUrl& url() const;

    void setEnfuseSettings(const EnfuseSettings& settings);
    EnfuseSettings enfuseSettings() const;

    void setOn(bool b);
    bool isOn() const;

    void setProgressAnimation(const QPixmap& pix);
    void setThumbnail(const QPixmap& pix);
    void setProcessedIcon(const QIcon& icon);
    bool asValidThumb() const;

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------------------

class EnfuseStackList : public QTreeWidget
{
    Q_OBJECT

public:

    EnfuseStackList(QWidget* const parent);
    virtual ~EnfuseStackList();

    void setTemplateFileName(DSaveSettingsWidget::OutputFormat, const QString&);

    void setThumbnail(const QUrl& url, const QImage& img);
    void setOnItem(const QUrl& url, bool on);
    void removeItem(const QUrl& url);
    void clearSelected();
    void addItem(const QUrl& url, const EnfuseSettings& settings);
    void processingItem(const QUrl& url, bool run);
    void processedItem(const QUrl& url, bool success);

    QList<EnfuseSettings> settingsList();

Q_SIGNALS:

    void signalItemClicked(const QUrl&);

private:

    EnfuseStackItem* findItemByUrl(const QUrl& url);

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem*);
    void slotContextMenu(const QPoint&);
    void slotRemoveItem();
    void slotProgressTimerDone();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ENFUSESTACK_H */
