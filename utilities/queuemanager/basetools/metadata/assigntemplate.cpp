/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : assign metadata template batch tool.
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assigntemplate.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocale.h>
#include <kvbox.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "template.h"
#include "templatemanager.h"
#include "templateselector.h"
#include "templateviewer.h"

namespace Digikam
{

AssignTemplate::AssignTemplate(QObject* parent)
    : BatchTool("AssignTemplate", MetadataTool, parent)
{
    m_templateSelector = 0;
    m_templateViewer   = 0;

    setToolTitle(i18n("Apply Metadata Template"));
    setToolDescription(i18n("Apply template metadata"));
    setToolIconName("application-xml");
}

AssignTemplate::~AssignTemplate()
{
}

void AssignTemplate::registerSettingsWidget()
{
    KVBox* const vbox  = new KVBox;
    m_templateSelector = new TemplateSelector(vbox);
    m_templateViewer   = new TemplateViewer(vbox);
    m_settingsWidget   = vbox;

    connect(m_templateSelector, SIGNAL(signalTemplateSelected()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AssignTemplate::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("TemplateTitle", QString());
    return settings;
}

void AssignTemplate::slotAssignSettings2Widget()
{
    QString title = settings()["TemplateTitle"].toString();

    Template t;

    if (title == Template::removeTemplateTitle())
    {
        t.setTemplateTitle(Template::removeTemplateTitle());
    }
    else if (title.isEmpty())
    {
        // Nothing to do.
    }
    else
    {
        t = TemplateManager::defaultManager()->findByTitle(title);
    }

    m_templateSelector->setTemplate(t);
    m_templateViewer->setTemplate(t);
}

void AssignTemplate::slotSettingsChanged()
{
    m_templateViewer->setTemplate(m_templateSelector->getTemplate());
    BatchToolSettings settings;
    settings.insert("TemplateTitle", m_templateSelector->getTemplate().templateTitle());
    BatchTool::slotSettingsChanged(settings);
}

bool AssignTemplate::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    QString title = settings()["TemplateTitle"].toString();

    DMetadata meta(image().getMetadata());

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

    image().setMetadata(meta.data());

    return (savefromDImg());
}

}  // namespace Digikam
