/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "plugin_photolayoutseditor.h"

// KDE includes

#include "digikam_debug.h"
#include <kgenericfactory.h>
#include <QIcon>
#include <QAction>
#include <kactioncollection.h>
#include <kaboutdata.h>

// Libkipi includes

#include <KIPI/Interface>
#include <KIPI/ImageCollection>

// Local includes

#include "photolayoutswindow.h"
#include "PLEAboutData.h"

namespace PhotoLayoutsEditor
{

K_PLUGIN_FACTORY ( PhotoFrmesEditorFactory, registerPlugin<Plugin_PhotoLayoutsEditor>(); )
K_EXPORT_PLUGIN( PhotoFrmesEditorFactory( "kipiplugin_photolayouteditor" ))

Plugin_PhotoLayoutsEditor::Plugin_PhotoLayoutsEditor(QObject* const parent, const QVariantList&)
    : Plugin( PhotoFrmesEditorFactory::componentData(), parent, "photolayoutseditor" )
{
    m_interface    = 0;
    m_action       = 0;
    m_parentWidget = 0;
    m_manager      = 0;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Plugin_PhotoLayoutsEditor plugin loaded";

    setUiBaseName("kipiplugin_photolayoutseditorui.rc");
    setupXML();
}

Plugin_PhotoLayoutsEditor::~Plugin_PhotoLayoutsEditor()
{
}

void Plugin_PhotoLayoutsEditor::setup(QWidget* const widget)
{
    m_parentWidget = widget;
    Plugin::setup(m_parentWidget);

    setupActions();

    m_interface = interface();
    if (!m_interface)
    {
       qCCritical(DIGIKAM_GENERAL_LOG) << "Kipi interface is null!";
       return;
    }

    m_action->setEnabled(true);
}

void Plugin_PhotoLayoutsEditor::setupActions()
{
    setDefaultCategory(ToolsPlugin);

    m_action = new QAction(this);
    m_action->setText(i18n("Create photo layouts..."));
    m_action->setIcon(QIcon::fromTheme("photolayoutseditor"));
    m_action->setEnabled(false);

    connect(m_action, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));

    addAction("photolayoutseditor", m_action);
}

void Plugin_PhotoLayoutsEditor::slotActivate()
{
    if (!m_interface)
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Kipi interface is null!";
        return;
    }

    ImageCollection images = m_interface->currentSelection();

    m_manager = PhotoLayoutsEditor::instance(m_parentWidget);
    m_manager->open();
    if (images.isValid() || !images.images().isEmpty())
        m_manager->setItemsList(images.images());
    m_manager->setInterface(m_interface);
    m_manager->show();
}

} // namespace PhotoLayoutsEditor
