/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-02
 * Description : setup tab for showfoto image editor options.
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

#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

// Qt includes.

#include <qwidget.h>

class QCheckBox;

class KColorButton;
class KIntNumInput;

class SetupEditor : public QWidget
{
    Q_OBJECT
    
public:

    SetupEditor(QWidget* parent = 0);
    ~SetupEditor();

    void applySettings();

private:

    KColorButton *m_backgroundColor;
    
    KIntNumInput *m_JPEGcompression;
    KIntNumInput *m_PNGcompression;
    
    QCheckBox    *m_TIFFcompression;
    QCheckBox    *m_hideToolBar;
    QCheckBox    *m_hideThumbBar;
    QCheckBox    *m_showSplashCheck;
    QCheckBox    *m_useTrashCheck;
    
    void readSettings();
};

#endif /* SETUPEDITOR_H */
