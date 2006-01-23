/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-20
 * Description : main image editor GUI implementation
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

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kmainwindow.h>
#include <kurl.h>

class KToolBarPopupAction;

namespace Digikam
{

class Canvas;
class EditorWindowPriv;

class EditorWindow : public KMainWindow
{
    Q_OBJECT

public:

    EditorWindow(const char *name);
    ~EditorWindow();

    virtual void applySettings(){};

signals:

protected slots:

    virtual void slotFilePrint()=0;
    
    void slotImagePluginsHelp();
    void slotEditKeys();
    void slotResize();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();
    
protected:

    void printImage(KURL url);

protected:

    KToolBarPopupAction *m_undoAction;
    KToolBarPopupAction *m_redoAction;
    
    Canvas              *m_canvas;

private slots:


    
private:
    
    EditorWindowPriv *d;

};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
