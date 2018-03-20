/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : action categorized view
 *
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

#ifndef ACTIONCATEGORIZEDVIEW_H
#define ACTIONCATEGORIZEDVIEW_H

// Local includes

#include "digikam_export.h"
#include "dcategorizedview.h"

class QWidget;
class QEvent;
class QScrollBar;
class QMouseEvent;
class QPropertyAnimation;

namespace Digikam
{

class DIGIKAM_EXPORT ActionCategorizedView : public DCategorizedView
{
public:

    explicit ActionCategorizedView(QWidget* const parent=0, bool autoScroll=false);
    ~ActionCategorizedView();

    void setupIconMode();
    void adjustGridSize();

protected:

    int  autoScrollDuration(float relativeDifference, QPropertyAnimation* animation);
    void autoScroll(float relativePos, QScrollBar* scrollBar, QPropertyAnimation* animation);
    void mouseMoveEvent(QMouseEvent* e);
    void leaveEvent(QEvent* e);

protected:

    QPropertyAnimation* m_verticalScrollAnimation;
    QPropertyAnimation* m_horizontalScrollAnimation;
    bool                m_autoScroll;
};

} // namespace Digikam

#endif // ACTIONCATEGORIZEDVIEW_H
