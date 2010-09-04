/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * File Name        :scriptplugintype.cpp
 * Creation Date    :22-08-2010
 * Description      :Class definition for individual plugins in the script manager.
 * Last Modified    :Sun 22 Aug 2010 07:47:00 AM EDT
 *
 *
 * Copyright (C) 2010 Created By: Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 Created By: Gilles Caulier <caulier dot gilles at gmail dot com>
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

//local includes
#include "scriptplugintype.h"

namespace Digikam
{

    class ScriptPluginTypePriv
    {
        public:
            ScriptPluginTypePriv()
            {
                name = "";
                path = "";
                modified = false;
            }
            ScriptPluginTypePriv(QString& pname,QString& ppath,bool pmod)
            {
                name        =   pname;
                path        =   ppath;
                modified    =   pmod;
            }

            QString name;
            QString path;
            bool    modified;

    };

    ScriptPluginType::ScriptPluginType()
                    : d(new ScriptPluginTypePriv)
    {}

    ScriptPluginType::ScriptPluginType(QString& name,QString& path,bool modified)
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

    void ScriptPluginType::createPlugin(QString name,QString path,bool modified)
    {

        d->name = name;
        d->path = path;
        d->modified = modified;

    }

    void ScriptPluginType::setName(QString& name)
    {
        d->name=name;
    }

    QString ScriptPluginType::name()
    {
        return d->name;
    }

    void ScriptPluginType::setPath(QString& path)
    {
        d->path=path;
    }

    QString ScriptPluginType::path()
    {
        return d->path;
    }

    void ScriptPluginType::setModified(bool modified)
    {
        d->modified=modified;
    }

    bool ScriptPluginType::modified()
    {
        return d->modified;
    }

}

