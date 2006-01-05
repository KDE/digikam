/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-02
 * Description : setup Image Editor plugins tab.
 * 
 * Copyright 2006 by Gilles Caulier
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

#ifndef SETUPIMGPLUGINS_H
#define SETUPIMGPLUGINS_H

// Qt includes.

#include <qwidget.h>

class QLabel;
class QStringList;
class QCheckBox;

class KColorButton;
class KIntNumInput;
class KListView;

namespace Digikam
{

class SetupImgPlugins : public QWidget
{
    Q_OBJECT
    
public:

    SetupImgPlugins(QWidget* parent = 0);
    ~SetupImgPlugins();

    void applySettings();
    QStringList getImagePluginsListEnable();

private:

    QStringList   m_availableImagePluginList;
    QStringList   m_enableImagePluginList;
    
    QLabel       *m_pluginsNumber;
    
    KListView    *m_pluginList;
    
private:

    void readSettings();
    void initImagePluginsList();
    void updateImagePluginsList(QStringList lista, QStringList listl);
    
};

}  // namespace Digikam

#endif // SETUPIMGPLUGINS_H 
