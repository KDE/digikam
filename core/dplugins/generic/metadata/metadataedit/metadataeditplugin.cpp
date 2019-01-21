/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to edit items metadata.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "metadataeditplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metadataedit.h"

namespace Digikam
{

MetadataEditPlugin::MetadataEditPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

MetadataEditPlugin::~MetadataEditPlugin()
{
}

QString MetadataEditPlugin::name() const
{
    return i18n("Metadata Edit");
}

QString MetadataEditPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MetadataEditPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("format-text-code"));
}

QString MetadataEditPlugin::description() const
{
    return i18n("A tool to edit items metadata");
}

QString MetadataEditPlugin::details() const
{
    return i18n("<p>This tool permit to changes plenty of metadata from items.</p>"
                "<p>Most common Exif, Iptc, and Xmp tags used in photography are listed for editing with standardized values.</p>"
                "<p>For photo agencies, pre-configured subjects canb eused to describe the items contents based on Iptc reference codes.</p>");
}

QList<DPluginAuthor> MetadataEditPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Victor Dodon"),
                             QString::fromUtf8("victor dot dodon at cti dot pub dot ro"),
                             QString::fromUtf8("(C) 2010-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2019"))
            ;
}

void MetadataEditPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Edit Metadata..."));
    ac->setObjectName(QLatin1String("metadata_edit"));
    ac->setActionCategory(DPluginAction::GenericMetadata);
    ac->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_M);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotEditMetadata()));

    addAction(ac);
}

void MetadataEditPlugin::slotEditMetadata()
{
    DInfoInterface* const iface = infoIface(sender());

    if (!iface)
        return;

    QList<QUrl> urls = iface->currentSelectedItems();

    if (urls.isEmpty())
        return;

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(0, iface);
    dialog->setPlugin(this);
    dialog->exec();
    delete dialog;
}

} // namespace Digikam
