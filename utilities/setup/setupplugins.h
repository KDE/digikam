//////////////////////////////////////////////////////////////////////////////
//
//    SETUPPLUGINS.H
//
//    Copyright (C) 2004 Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SETUPPLUGINS_H
#define SETUPPLUGINS_H

// Qt includes.

#include <qwidget.h>

class KListView;

class SetupPlugins : public QWidget
{
    Q_OBJECT
    
public:

    SetupPlugins(QWidget* parent = 0);
    ~SetupPlugins();

    void initPlugins(QStringList lista, QStringList listl);
    QStringList getPluginList();

private:

    KListView* m_pluginList;
};

#endif // SETUPPLUGINS_H 
