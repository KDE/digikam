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

#ifndef BRACKETSTACK_H
#define BRACKETSTACK_H

// Qt includes

#include <QTreeWidget>
#include <QString>
#include <QPixmap>
#include <QUrl>

// Local includes

#include "loadingdescription.h"

namespace Digikam
{

class BracketStackItem : public QTreeWidgetItem
{
public:

    explicit BracketStackItem(QTreeWidget* const parent);
    virtual ~BracketStackItem();

    void setUrl(const QUrl& url);
    const QUrl& url() const;

    void setOn(bool b);
    bool isOn() const;

    void setThumbnail(const QPixmap& pix);
    void setExposure(const QString& exp);

private:

    bool operator< (const QTreeWidgetItem& other) const;

private:

    QUrl m_url;
};

// ---------------------------------------------------------------------

class BracketStackList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit BracketStackList(QWidget* const parent);
    virtual ~BracketStackList();

    void addItems(const QList<QUrl>& list);

    QList<QUrl> urls();
    BracketStackItem* findItem(const QUrl& url);

Q_SIGNALS:

    void signalAddItems(const QList<QUrl>&);

private Q_SLOTS:

    void slotThumbnail(const LoadingDescription&, const QPixmap& pix);
};

}  // namespace Digikam

#endif /* BRACKETSTACK_H */
