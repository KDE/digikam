/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-02
 * Description : setup tab for showfoto plugins options.
 * 
 * Copyright 2005 by Gilles Caulier
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

class QLabel;
class QStringList;
class QCheckBox;

class KListView;

namespace ShowFoto
{

class SetupPlugins : public QWidget
{
    Q_OBJECT
    
public:

    SetupPlugins(QWidget* parent = 0);
    ~SetupPlugins();

    void applySettings();
    QStringList getImagePluginsListEnable();

private:

    QStringList   m_availableImagePluginList;
    QStringList   m_enableImagePluginList;
    
    QLabel       *m_pluginsNumber;
    
    KListView    *m_pluginList;
    
    void readSettings();
    void initImagePluginsList();
    void updateImagePluginsList(QStringList lista, QStringList listl);    
};

}   // namespace ShowFoto

#endif /* SETUPEDITOR_H */
