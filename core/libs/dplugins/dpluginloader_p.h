/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time: private container
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

#include "dpluginloader.h"

// Qt includes

#include <QDir>
#include <QPluginLoader>

namespace Digikam
{

class Q_DECL_HIDDEN DPluginLoader::Private
{
public:

    explicit Private();
    ~Private();

    /** Try to find plugin files from Qt5 plugins install dir:
     */
    QStringList pluginEntriesList() const;

    bool        appendPlugin(QObject* const obj, QPluginLoader* const loader, QList<DPlugin*>& list);
    void        loadPlugins();

public:

    bool            pluginsLoaded;
    QList<DPlugin*> allPlugins;
    QStringList     blacklist;
    QStringList     whitelist;
};

} // namepace Digikam
