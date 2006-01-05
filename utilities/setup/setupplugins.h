/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-01-02
 * Description : setup Kipi plugins tab.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef SETUPPLUGINS_H
#define SETUPPLUGINS_H

// Qt includes.

#include <qwidget.h>

// Includes files for plugins support.

#include <libkipi/pluginloader.h>

class KListView;
class QLabel;

namespace Digikam
{

class SetupPlugins : public QWidget
{
    Q_OBJECT
    
public:

    SetupPlugins(QWidget* parent = 0);
    ~SetupPlugins();

    void initPlugins(int kipiPluginsNumber);
    void applyPlugins();

private:
    
    QLabel*             m_pluginsNumber;
    KListView*          m_pluginList;
    KIPI::ConfigWidget* m_Kipiconfig;
};

}  // namespace Digikam

#endif // SETUPPLUGINS_H 

