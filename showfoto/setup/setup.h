/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-02
 * Description : showfoto setup dialog.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
 
#ifndef SETUP_H
#define SETUP_H

// KDE includes.

#include <kdialogbase.h>

class QFrame;

namespace Digikam
{
class SetupImgPlugins;
class SetupIOFiles;
class SetupSlideShow;
class SetupICC;
}

namespace ShowFoto
{

class SetupEditor;
class SetupPrivate;

class Setup : public KDialogBase 
{
    Q_OBJECT

public:

    enum Page 
    {
        LastPageUsed = -1,
        EditorPage=0,
        IOFilesPage,
        PluginsPage,
        SlideshowPage,
        ICCPage
    };
    
    Setup(QWidget* parent=0, const char* name=0, Page page=LastPageUsed);
    ~Setup();

    Digikam::SetupImgPlugins *imagePluginsPage();
    
private slots:

    void slotOkClicked();
        
private:

    SetupPrivate* d;
        
};

}   // namespace ShowFoto

#endif  /* SETUP_H  */
