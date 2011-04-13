/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 19-07-2010
 * Description : script manager for digiKam
 *
 * Copyright (C) 2010 by Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPSCRIPTMANAGER_H
#define SETUPSCRIPTMANAGER_H

//Qt includes

#include <QScrollArea>

//local includes

#include "scriptplugintype.h"

namespace Digikam
{

class SetupScriptManager : public QScrollArea
{
    Q_OBJECT

public:

    SetupScriptManager(QWidget* parent = 0);
    ~SetupScriptManager();

    void applySettings();

private:

    void readSettings();
    void addEntry(ScriptPluginType* plugin);

private:

    class SetupScriptManagerPriv;
    SetupScriptManagerPriv* const d;
};

}//namespace digikam

#endif /* SETUPSCRIPTMANAGER_H */
