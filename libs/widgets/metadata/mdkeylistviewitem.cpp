/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to 
 *               display metadata key like a title
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt include.

#include <QPalette>
#include <QFont>
#include <QPainter>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "themeengine.h"
#include "ddebug.h"
#include "mdkeylistviewitem.h"
#include "mdkeylistviewitem.moc"

namespace Digikam
{

MdKeyListViewItem::MdKeyListViewItem(QTreeWidget *parent, const QString& key)
                 : QObject(parent), QTreeWidgetItem(parent)
{
    m_decryptedKey = key;

    // Standard Exif key descriptions.
    if      (key == "Iop")       m_decryptedKey = i18n("Interoperability");
    else if (key == "Image")     m_decryptedKey = i18n("Image Information");
    else if (key == "Photo")     m_decryptedKey = i18n("Photograph Information");
    else if (key == "GPSInfo")   m_decryptedKey = i18n("Global Positioning System");
    else if (key == "Thumbnail") m_decryptedKey = i18n("Embedded Thumbnail");

    // Standard IPTC key descriptions.
    else if (key == "Envelope")     m_decryptedKey = i18n("IIM Envelope");
    else if (key == "Application2") m_decryptedKey = i18n("IIM Application 2");

    setDisabled(false);
    setSelected(false);
    setExpanded(true);
    setFlags(flags() & !Qt::ItemIsSelectable);

    // item is not selectable.

    setFirstColumnSpanned(true);
    setTextAlignment(0, Qt::AlignCenter);
    QFont fn0(font(0));
    fn0.setBold(true);
    fn0.setItalic(false);
    setFont(0, fn0);
    QFont fn1(font(1));
    fn1.setBold(true);
    fn1.setItalic(false);
    setFont(1, fn1);
    setText(0, m_decryptedKey);
    slotThemeChanged();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

MdKeyListViewItem::~MdKeyListViewItem()
{
}

QString MdKeyListViewItem::getMdKey()
{
    return m_decryptedKey;
}

void MdKeyListViewItem::slotThemeChanged()
{
    setBackground(0, QBrush(ThemeEngine::instance()->thumbSelColor()));
    setBackground(1, QBrush(ThemeEngine::instance()->thumbSelColor()));
    setForeground(0, QBrush(ThemeEngine::instance()->textRegColor()));
    setForeground(1, QBrush(ThemeEngine::instance()->textRegColor()));
}

}  // namespace Digikam
