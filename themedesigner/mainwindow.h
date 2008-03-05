/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-14
 * Description : main digiKam theme designer window
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes.

#include <qwidget.h>
#include <qmap.h>

class QComboBox;
class QCheckBox;
class QLabel;
class KColorButton;

namespace Digikam
{

class ImagePropertiesTab;
class FolderView;
class ThemedIconView;
class Theme;

class MainWindow : public QWidget
{
    Q_OBJECT

public:

    enum PROPERTY 
    {
        BASE = 0,
        REGULARTEXT,
        SELECTEDTEXT,
        REGULARSPECIALTEXT,
        SELECTEDSPECIALTEXT,
        BANNER,
        THUMBNAILREGULAR,
        THUMBNAILSELECTED,
        LISTVIEWREGULAR,
        LISTVIEWSELECTED
    };

    enum BEVEL 
    {
        FLAT = 0,
        RAISED,
        SUNKEN
    };

    enum GRADIENT 
    {
        SOLID = 0,
        HORIZONTAL,
        VERTICAL,
        DIAGONAL
    };

public:

    MainWindow();
    ~MainWindow();

private slots:

    void slotLoad();
    void slotSave();
    void slotPropertyChanged();
    void slotUpdateTheme();

private:

    QLabel             *m_bevelLabel;
    QLabel             *m_gradientLabel;
    QLabel             *m_begColorLabel;
    QLabel             *m_endColorLabel;
    QLabel             *m_borderColorLabel;

    QComboBox          *m_propertyCombo;
    QComboBox          *m_bevelCombo;
    QComboBox          *m_gradientCombo;

    QCheckBox          *m_addBorderCheck;

    QMap<int,int>       m_bevelMap;
    QMap<int,int>       m_bevelReverseMap;
    QMap<int,int>       m_gradientMap;
    QMap<int,int>       m_gradientReverseMap;

    KColorButton       *m_endColorBtn;
    KColorButton       *m_begColorBtn;
    KColorButton       *m_borderColorBtn;

    FolderView         *m_folderView;
    ThemedIconView     *m_iconView;
    ImagePropertiesTab *m_propView;
    Theme              *m_theme;
};

}  // NameSpace Digikam

#endif // MAINWINDOW_H
