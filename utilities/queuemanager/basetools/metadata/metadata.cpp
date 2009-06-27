/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : metadata edit batch tool.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadata.h"
#include "metadata.moc"

// Qt includes

#include <QWidget>
#include <QLabel>

// KDE includes

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

// Local includes

#include "dimg.h"
#include "templateselector.h"
#include "templatemanager.h"
#include "template.h"
#include "dmetadata.h"

namespace Digikam
{

Metadata::Metadata(QObject* parent)
        : BatchTool("Metadata", BaseTool, parent)
{
    setToolTitle(i18n("Apply Metadata Template"));
    setToolDescription(i18n("A tool to apply template metadata"));
    setToolIcon(KIcon(SmallIcon("application-xml")));

    KVBox *vbox = new KVBox;
    m_selector  = new TemplateSelector(vbox);

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_selector, SIGNAL(signalTemplateSelected()),
            this, SLOT(slotSettingsChanged()));
}

Metadata::~Metadata()
{
}

BatchToolSettings Metadata::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("TemplateTitle", TemplateManager::defaultManager()->unknowTemplate().templateTitle());
    return settings;
}

void Metadata::assignSettings2Widget()
{
    QString title = settings()["TemplateTitle"].toString();
    Template t    = TemplateManager::defaultManager()->findByTitle(title);
    m_selector->setTemplate(t);
}

void Metadata::slotSettingsChanged()
{
    BatchToolSettings settings;
    QString title = m_selector->getTemplate().templateTitle();
    settings.insert("TemplateTitle", title);
    setSettings(settings);
}

bool Metadata::toolOperations()
{
    if (!loadToDImg()) return false;

    QString title = settings()["TemplateTitle"].toString();
    Template t    = TemplateManager::defaultManager()->findByTitle(title);

    DMetadata meta;
    meta.setExif(image().getExif());
    meta.setIptc(image().getIptc());
    meta.setXmp(image().getXmp());

    if (t == TemplateManager::defaultManager()->removeTemplate())
    {
        meta.removeMetadataTemplate();
    }
    else if (t == TemplateManager::defaultManager()->unknowTemplate())
    {
        // Nothing to do.
    }
    else
    {
        meta.removeMetadataTemplate();
        meta.setMetadataTemplate(t);
    }

    image().setExif(meta.getExif());
    image().setIptc(meta.getIptc());
    image().setXmp(meta.getXmp());

    return (savefromDImg());
}

}  // namespace Digikam
