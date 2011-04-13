/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 21-08-2010
 * Description : script interface for digiKam
 *
 * Copyright (C) 2010 Created By Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 Created By Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "scriptplugintype.moc"

namespace Digikam
{

class ScriptPluginType::ScriptPluginTypePriv
{
public:

    ScriptPluginTypePriv()
    {
        name     = QString();
        path     = QString();
        modified = false;
    }

    ScriptPluginTypePriv(QString& pname,QString& ppath,bool pmod)
    {
        name     = pname;
        path     = ppath;
        modified = pmod;
    }

    bool    modified;

    QString name;
    QString path;
};

ScriptPluginType::ScriptPluginType()
    : d(new ScriptPluginTypePriv)
{
}

ScriptPluginType::ScriptPluginType(const QString& name, const QString& path, bool modified)
    : d(new ScriptPluginTypePriv)
{
    d->name     = name;
    d->path     = path;
    d->modified = modified;
}

ScriptPluginType::~ScriptPluginType()
{
    delete d;
}

void ScriptPluginType::createPlugin(const QString& name, const QString& path, bool modified)
{
    d->name     = name;
    d->path     = path;
    d->modified = modified;
}

void ScriptPluginType::setName(const QString& name)
{
    d->name = name;
}

QString ScriptPluginType::name() const
{
    return d->name;
}

void ScriptPluginType::setPath(const QString& path)
{
    d->path = path;
}

QString ScriptPluginType::path() const
{
    return d->path;
}

void ScriptPluginType::setModified(bool modified)
{
    d->modified = modified;
}

bool ScriptPluginType::modified() const
{
    return d->modified;
}

} // namespace Digikam
