/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Setup view panel for dplugins.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOW_FOTO_SETUP_PLUGINS_H
#define SHOW_FOTO_SETUP_PLUGINS_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class SetupPlugins : public QScrollArea
{
    Q_OBJECT

public:

    enum PluginTab
    {
        Generic = 0,
        Editor
    };
    
public:

    explicit SetupPlugins(QWidget* const parent = 0);
    ~SetupPlugins();

    void applySettings();

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_SETUP_PLUGINS_H
