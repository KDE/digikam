/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : assign metadata template batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assigntemplate.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "template.h"
#include "templatemanager.h"
#include "templateselector.h"
#include "templateviewer.h"

namespace Digikam
{

AssignTemplate::AssignTemplate(QObject* const parent)
    : BatchTool(QLatin1String("AssignTemplate"), MetadataTool, parent)
{
    m_templateSelector = 0;
    m_templateViewer   = 0;

    setToolTitle(i18n("Apply Metadata Template"));
    setToolDescription(i18n("Apply template metadata"));
    setToolIconName(QLatin1String("text-xml"));
}

AssignTemplate::~AssignTemplate()
{
}

void AssignTemplate::registerSettingsWidget()
{
    DVBox* const vbox  = new DVBox;
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
    settings.insert(QLatin1String("TemplateTitle"), QString());
    return settings;
}

void AssignTemplate::slotAssignSettings2Widget()
{
    QString title = settings()[QLatin1String("TemplateTitle")].toString();

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
    settings.insert(QLatin1String("TemplateTitle"), m_templateSelector->getTemplate().templateTitle());
    BatchTool::slotSettingsChanged(settings);
}

bool AssignTemplate::toolOperations()
{
    DMetadata meta;

    if (image().isNull())
    {
        if (!meta.load(inputUrl().toLocalFile()))
        {
            return false;
        }
    }
    else
    {
        meta.setData(image().getMetadata());
    }

    QString title = settings()[QLatin1String("TemplateTitle")].toString();

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

    bool ret = true;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = QFile::copy(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret && !title.isEmpty())
        {
            ret = meta.save(outputUrl().toLocalFile());
        }
    }
    else
    {
        if (!title.isEmpty())
        {
            image().setMetadata(meta.data());
        }

        ret = savefromDImg();
    }

    return ret;
}

} // namespace Digikam
