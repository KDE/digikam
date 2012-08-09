/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QWidget>
#include <QKeyEvent>

// Local includes

#include "digikam_export.h"


namespace Digikam
{

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

    void keyPressEvent(QKeyEvent* e);

Q_SIGNALS:

    void signalNext();
    void signalPrev();
    void signalClose();
    void signalPlay();
    void signalPause();

private Q_SLOTS:

    void slotPlayBtnToggled();
    void slotNexPrevClicked();

private:


    class ToolBarPriv;
    ToolBarPriv* const d;

    friend class SlideShow;
};

}   // namespace Digikam

#endif /* TOOL_BAR_H */
