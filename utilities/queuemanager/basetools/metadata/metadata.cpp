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
#include "templateviewer.h"
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

    KVBox *vbox        = new KVBox;
    m_templateSelector = new TemplateSelector(vbox);
    m_templateViewer   = new TemplateViewer(vbox);

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_templateSelector, SIGNAL(signalTemplateSelected()),
            this, SLOT(slotSettingsChanged()));
}

Metadata::~Metadata()
{
}

BatchToolSettings Metadata::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("TemplateTitle", QString());
    return settings;
}

void Metadata::assignSettings2Widget()
{
    QString title = settings()["TemplateTitle"].toString();
    Template t    = TemplateManager::defaultManager()->findByTitle(title);
    m_templateSelector->setTemplate(t);
}

void Metadata::slotSettingsChanged()
{
    m_templateViewer->setTemplate(m_templateSelector->getTemplate());
    BatchToolSettings settings;
    settings.insert("TemplateTitle", m_templateSelector->getTemplate().templateTitle());
    setSettings(settings);
}

bool Metadata::toolOperations()
{
    if (!loadToDImg()) return false;

    QString title = settings()["TemplateTitle"].toString();

    DMetadata meta;
    meta.setExif(image().getExif());
    meta.setIptc(image().getIptc());
    meta.setXmp(image().getXmp());

    if (title == Template::removeTemplateTitle())
    {
        meta.removeMetadataTemplate();
    }
    else if (title.isEmpty())
    {
        // Nothing to do.
    }
    else
    {
        Template t = TemplateManager::defaultManager()->findByTitle(title);
        meta.removeMetadataTemplate();
        meta.setMetadataTemplate(t);
    }

    image().setExif(meta.getExif());
    image().setIptc(meta.getIptc());
    image().setXmp(meta.getXmp());

    return (savefromDImg());
}

}  // namespace Digikam
