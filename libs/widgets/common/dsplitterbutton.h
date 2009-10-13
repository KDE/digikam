/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-12
 * Description : A button which appears on the side of a splitter handle 
 *               and allows easy collapsing of the widget on the opposite side.
 *
 * Copyright (C) 2009 by Gilles Caulier<caulier dot gilles at gmail dot com>
 * Copyright (c) 2009 Aurélien Gateau <agateau@kde.org>
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

#ifndef DSPLITTERBUTTON_H
#define DSPLITTERBUTTON_H

// Qt includes.

#include <QToolButton>

// Local includes

#include "digikam_export.h"

class QSplitter;

namespace Digikam
{

class DSplitterButtonPrivate;

/**
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 */
class DIGIKAM_EXPORT DSplitterButton : public QToolButton
{
    Q_OBJECT

public:

    DSplitterButton(QSplitter* sp, QWidget* widget);
    ~DSplitterButton();

    virtual QSize sizeHint() const;

protected:

    virtual bool eventFilter(QObject*, QEvent*);
    virtual void paintEvent(QPaintEvent*);

private:

    DSplitterButtonPrivate* const d;

private Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam

#endif /* DSPLITTERBUTTON_H */
