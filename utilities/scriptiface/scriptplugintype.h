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

#ifndef SCRIPTPLUGINTYPE_H
#define SCRIPTPLUGINTYPE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QPointer>

namespace Digikam
{

class ScriptPluginType : public QObject
{
    Q_OBJECT

public:

    ScriptPluginType();
    ScriptPluginType(const QString& name, const QString& path, bool modified);
    ~ScriptPluginType();

    void setName(const QString& name);
    void setPath(const QString& path);
    void setModified(bool modified);

    QString name() const;
    QString path() const;
    bool modified() const;

    void createPlugin(const QString& name, const QString& path, bool modified);

private:

    class ScriptPluginTypePriv;
    ScriptPluginTypePriv* const d;
};

} // namespace digikam

#endif /* SCRIPTPLUGINTYPE_H */
