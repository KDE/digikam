/* ============================================================
 * File  : showfoto.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Copyright 2004 by Renchi Raju
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
 * ============================================================ */

#ifndef SHOWFOTO_H
#define SHOWFOTO_H

#include <kmainwindow.h>
#include <kurl.h>

class KAction;
class KToggleAction;

class Canvas;
class ThumbBarView;

class ShowFoto : public KMainWindow
{
    Q_OBJECT
    
public:

    ShowFoto(const KURL::List& urlList);
    ~ShowFoto();

private slots:

    void slotOpenFile();
    void slotNext();
    void slotPrev();
    void slotAutoFit();
    void slotOpenURL(const KURL& url);
    
private:

    void setupActions();

private:

    Canvas*         m_canvas;
    ThumbBarView*   m_bar;
    KToggleAction*  m_zoomFitAction;
    KAction*        m_zoomPlusAction;
    KAction*        m_zoomMinusAction;
};

#endif /* SHOWFOTO_H */
