/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDE_TOOL_BAR_H
#define SLIDE_TOOL_BAR_H

// Qt includes

#include <QWidget>
#include <QKeyEvent>

// Local includes

#include "digikam_export.h"
#include "slideshowsettings.h"
#include "dlayoutbox.h"

class QAction;

namespace Digikam
{

class DIGIKAM_EXPORT SlideToolBar : public DHBox
{
    Q_OBJECT

public:

    explicit SlideToolBar(const SlideShowSettings& settings, QWidget* const parent);
    virtual ~SlideToolBar();

    bool isPaused() const;
    void pause(bool val);

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

    void signalScreenSelected(int);

private Q_SLOTS:

    void slotPlayBtnToggled();
    void slotNexPrevClicked();
    void slotScreenSelected(QAction*);

private:

    class Private;
    Private* const d;

    friend class SlideShow;
};

} // namespace Digikam

#endif // SLIDE_TOOL_BAR_H
