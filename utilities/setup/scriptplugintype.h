/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * File Name : scriptplugintype.h
 * Creation Date : 21-08-2010
 * Description : script interface for digiKam
 * Last Modified : Thu 02 Sep 2010 08:35:38 AM EDT
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

#ifndef SCRIPTPLUGINTYPE_H
#define SCRIPTPLUGINTYPE_H

//qt includes
#include <QObject>
#include <QString>
#include <QPointer>

namespace Digikam
{

    class ScriptPluginTypePriv;

    class ScriptPluginType : public QObject
    {
        Q_OBJECT

        public:

            ScriptPluginType();
            ScriptPluginType(QString& name,QString& path,bool modified);
            ~ScriptPluginType();

            void setName(QString& name);
            void setPath(QString& path);
            void setModified(bool modified);

            QString name();
            QString path();
            bool    modified();

            void createPlugin(QString name,QString path,bool modified);//use const reference

        private:

            ScriptPluginTypePriv* const d;
    };

}//namespace digikam

#endif /* SCRIPTPLUGINTYPE_H */
