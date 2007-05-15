/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-14
 * Description : main digiKam theme designer window
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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
        
    MainWindow();
    ~MainWindow();

private:
    
    FolderView*     m_folderView;
    ThemedIconView* m_iconView;

    QComboBox*      m_propertyCombo;
    QLabel*         m_bevelLabel;
    QComboBox*      m_bevelCombo;
    QLabel*         m_gradientLabel;
    QComboBox*      m_gradientCombo;
    QLabel*         m_begColorLabel;
    KColorButton*   m_begColorBtn;
    QLabel*         m_endColorLabel;
    KColorButton*   m_endColorBtn;
    QCheckBox*      m_addBorderCheck;
    QLabel*         m_borderColorLabel;
    KColorButton*   m_borderColorBtn;

    Theme*          m_theme;

    QMap<int,int>   m_bevelMap;
    QMap<int,int>   m_bevelReverseMap;
    QMap<int,int>   m_gradientMap;
    QMap<int,int>   m_gradientReverseMap;
    
private slots:

    void slotLoad();
    void slotSave();
    
    void slotPropertyChanged();

    void slotUpdateTheme();
};

}  // NameSpace Digikam

#endif // MAINWINDOW_H
