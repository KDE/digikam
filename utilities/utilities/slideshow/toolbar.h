/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-10-05
 * Description : a tool bar for slideshow 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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

#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include <qwidget.h>

// Local includes.

#include "digikam_export.h"

class QToolButton;

namespace Digikam
{

class ToolBarPriv;

class DIGIKAM_EXPORT ToolBar : public QWidget
{
    Q_OBJECT
    
public:

    ToolBar(QWidget* parent);
    ~ToolBar();

    bool canHide() const;
    bool isPaused() const;
    void setPaused(bool val);

    void setEnabledPlay(bool val);
    void setEnabledNext(bool val);
    void setEnabledPrev(bool val);

protected:

    void keyPressEvent(QKeyEvent *event);
    
signals:

    void signalNext();
    void signalPrev();
    void signalClose();
    void signalPlay();
    void signalPause();

private slots:

    void slotPlayBtnToggled();
    void slotNexPrevClicked();

private:

    ToolBarPriv *d;

    friend class SlideShow;
};

}   // Namespace Digikam

#endif /* TOOL_BAR_H */
