/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata key like a title
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mdkeylistviewitem.moc"

// Qt includes

#include <QPalette>
#include <QFont>
#include <QPainter>

// KDE includes

#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "thememanager.h"

namespace Digikam
{

MdKeyListViewItem::MdKeyListViewItem(QTreeWidget* const parent, const QString& key)
    : QObject(parent), QTreeWidgetItem(parent)
{
    m_key          = key;
    m_decryptedKey = key;

    // Standard Exif key descriptions.
    if      (key == "Iop")
    {
        m_decryptedKey = i18n("Interoperability");
    }
    else if (key == "Image")
    {
        m_decryptedKey = i18n("Image Information");
    }
    else if (key == "Photo")
    {
        m_decryptedKey = i18n("Photograph Information");
    }
    else if (key == "GPSInfo")
    {
        m_decryptedKey = i18n("Global Positioning System");
    }
    else if (key == "Thumbnail")
    {
        m_decryptedKey = i18n("Embedded Thumbnail");
    }

    // Standard IPTC key descriptions.
    else if (key == "Envelope")
    {
        m_decryptedKey = i18n("IIM Envelope");
    }
    else if (key == "Application2")
    {
        m_decryptedKey = i18n("IIM Application 2");
    }

    // Standard XMP key descriptions.
    else if (key == "aux")
    {
        m_decryptedKey = i18n("Additional Exif Properties");
    }
    else if (key == "crs")
    {
        m_decryptedKey = i18n("Camera Raw");
    }
    else if (key == "dc")
    {
        m_decryptedKey = i18n("Dublin Core");
    }
    else if (key == "digiKam")
    {
        m_decryptedKey = i18n("digiKam schema");
    }
    else if (key == "exif")
    {
        m_decryptedKey = i18n("Exif-specific Properties");
    }
    else if (key == "iptc")
    {
        m_decryptedKey = i18n("IPTC Core");
    }
    else if (key == "iptcExt")
    {
        m_decryptedKey = i18n("IPTC Extension schema");
    }
    else if (key == "MicrosoftPhoto")
    {
        m_decryptedKey = i18n("Microsoft Photo");
    }
    else if (key == "pdf")
    {
        m_decryptedKey = i18n("Adobe PDF");
    }
    else if (key == "photoshop")
    {
        m_decryptedKey = i18n("Adobe Photoshop");
    }
    else if (key == "plus")
    {
        m_decryptedKey = i18n("PLUS License Data Format Schema");
    }
    else if (key == "tiff")
    {
        m_decryptedKey = i18n("TIFF Properties");
    }
    else if (key == "xmp")
    {
        m_decryptedKey = i18n("Basic Schema");
    }
    else if (key == "xmpBJ")
    {
        m_decryptedKey = i18n("Basic Job Ticket");
    }
    else if (key == "xmpDM")
    {
        m_decryptedKey = i18n("Dynamic Media");
    }
    else if (key == "xmpMM")
    {
        m_decryptedKey = i18n("Media Management ");
    }
    else if (key == "xmpRights")
    {
        m_decryptedKey = i18n("Rights Management");
    }
    else if (key == "xmpTPg")
    {
        m_decryptedKey = i18n("Paged-Text");
    }

    // Additional XMP key descriptions.
    else if (key == "mwg-rs")
    {
        m_decryptedKey = i18n("Metadata Working Group Regions");
    }
    else if (key == "dwc")
    {
        m_decryptedKey = i18n("Darwin Core");
    }

    // Reset all item flags: item is not selectable.
    setFlags(Qt::ItemIsEnabled);

    setDisabled(false);
    setExpanded(true);

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

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

MdKeyListViewItem::~MdKeyListViewItem()
{
}

QString MdKeyListViewItem::getKey() const
{
    return m_key;
}

QString MdKeyListViewItem::getDecryptedKey() const
{
    return m_decryptedKey;
}

void MdKeyListViewItem::slotThemeChanged()
{
    setBackground(0, QBrush(kapp->palette().color(QPalette::Highlight)));
    setBackground(1, QBrush(kapp->palette().color(QPalette::Highlight)));
    setForeground(0, QBrush(kapp->palette().color(QPalette::HighlightedText)));
    setForeground(1, QBrush(kapp->palette().color(QPalette::HighlightedText)));
}

}  // namespace Digikam
