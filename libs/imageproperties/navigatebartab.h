/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2007-01-04
 * Description : A parent tab class with a navigation bar
 *
 * Copyright 2006 by Gilles Caulier
 * Copyright 2007 by Gilles Caulier, Marcel Wiesweg
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

#ifndef NAVIGATEBARTAB_H
#define NAVIGATEBARTAB_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"
#include "imagepropertiessidebar.h"

class QVBoxLayout;

namespace Digikam
{

class NavigateBarWidget;
class NavigateBarTabPriv;

class DIGIKAM_EXPORT NavigateBarTab : public QWidget
{
    Q_OBJECT

public:

    NavigateBarTab(QWidget* parent);
    ~NavigateBarTab();

    void setNavigateBarState(bool hasPrevious, bool hasNext);
    void setNavigateBarState(int itemType);
    void setNavigateBarFileName(const QString &name = QString());
    void setLabelText(const QString &text);

signals:

    void signalFirstItem(void);
    void signalPrevItem(void);
    void signalNextItem(void);
    void signalLastItem(void);

protected:

    void setupNavigateBar(bool withBar);

protected:

    QVBoxLayout        *m_navigateBarLayout;
    NavigateBarTabPriv *d;

};

}  // NameSpace Digikam

#endif /* NAVIGATEBARTAB_H */
