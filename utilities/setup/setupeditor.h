/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-03
 * Description : setup tab for ImageEditor.
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

// Qt includes.

#include <qwidget.h>

class QLabel;
class QStringList;
class QCheckBox;

class KColorButton;
class KIntNumInput;
class KListView;

class SetupEditor : public QWidget
{
    Q_OBJECT
    
public:

    SetupEditor(QWidget* parent = 0);
    ~SetupEditor();

    void applySettings();
    QStringList getImagePluginsListEnable();

private:

    QStringList   m_availableImagePluginList;
    QStringList   m_enableImagePluginList;
    
    KColorButton *m_backgroundColor;
    
    KIntNumInput *m_JPEGcompression;
    KIntNumInput *m_PNGcompression;
    
    QCheckBox    *m_TIFFcompression;
    QCheckBox    *m_hideToolBar;

    QLabel       *m_pluginsNumber;
    
    KListView    *m_pluginList;
    
    void readSettings();
    void initImagePluginsList();
    void updateImagePluginsList(QStringList lista, QStringList listl);
    
};

#endif // SETUPEDITOR_H 
