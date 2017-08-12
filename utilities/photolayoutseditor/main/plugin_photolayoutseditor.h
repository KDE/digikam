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

#ifndef PLUGIN_PHOTOLAYOUTSEDITOR_H
#define PLUGIN_PHOTOLAYOUTSEDITOR_H

#include <QWidget>
#include <QVariant>

#include <KIPI/Plugin>

namespace KIPI
{
    class Interface;
}

using namespace KIPI;

namespace PhotoLayoutsEditor
{

class PhotoLayoutsEditor;

class Plugin_PhotoLayoutsEditor : public Plugin
{
    Q_OBJECT

public:

    Plugin_PhotoLayoutsEditor(QObject* const parent, const QVariantList& args);
    virtual ~Plugin_PhotoLayoutsEditor();

    void setup(QWidget* const);

public Q_SLOTS:

    void slotActivate();

private:

    void setupActions();

private:

    QWidget*            m_parentWidget;

    QAction *            m_action;

    PhotoLayoutsWindow* m_manager;

    Interface*          m_interface;
};

} // namespace PhotoLayoutsEditor

#endif // PLUGIN_PHOTOLAYOUTSEDITOR_H
