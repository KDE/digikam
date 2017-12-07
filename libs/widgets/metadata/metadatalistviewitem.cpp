/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatalistviewitem.h"

// Qt includes

#include <QPalette>
#include <QFont>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"

namespace Digikam
{

MetadataListViewItem::MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
        const QString& title, const QString& value)
    : QTreeWidgetItem(parent),
      m_key(key)
{
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setText(0, title);
    setToolTip(0, title);
    setDisabled(false);
    QString tagVal = value.simplified();

    if (tagVal.length() > 512)
    {
        tagVal.truncate(512);
        tagVal.append(QLatin1String("..."));
    }

    setText(1, tagVal);

    DToolTipStyleSheet cnt;
    setToolTip(1, QLatin1String("<qt><p>") + cnt.breakString(tagVal) + QLatin1String("</p></qt>"));
}

MetadataListViewItem::MetadataListViewItem(QTreeWidgetItem* const parent, const QString& key,
        const QString& title)
    : QTreeWidgetItem(parent),
      m_key(key)
{
    setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    setText(0, title);
    setToolTip(0, title);
    setDisabled(true);
    setText(1, i18n("Unavailable"));
    QFont fnt = font(1);
    fnt.setItalic(true);
    setFont(1, fnt);
}

MetadataListViewItem::~MetadataListViewItem()
{
}

QString MetadataListViewItem::getKey()
{
    return m_key;
}

QString MetadataListViewItem::getTitle()
{
    return text(0);
}

QString MetadataListViewItem::getValue()
{
    return text(1);
}

}  // namespace Digikam
