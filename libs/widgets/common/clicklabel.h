/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CLICKLABEL_H
#define CLICKLABEL_H

// Qt includes

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>

// KDE includes

#include <ksqueezedtextlabel.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DLabelExpanderPriv;

class DIGIKAM_EXPORT ClickLabel : public QLabel
{
    Q_OBJECT

public:

    ClickLabel(QWidget *parent = 0);
    ClickLabel(const QString& text, QWidget *parent = 0);
    ~ClickLabel(){};

Q_SIGNALS:

    /// Emitted when activated by left mouse click
    void leftClicked();
    /// Emitted when activated, by mouse or key press
    void activated();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT SqueezedClickLabel : public KSqueezedTextLabel
{
    Q_OBJECT

public:

    SqueezedClickLabel(QWidget *parent = 0);
    SqueezedClickLabel(const QString& text, QWidget *parent = 0);
    ~SqueezedClickLabel(){};

Q_SIGNALS:

    void leftClicked();
    void activated();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    ArrowClickLabel(QWidget *parent = 0);
    ~ArrowClickLabel(){};

    void setArrowType(Qt::ArrowType arrowType);
    virtual QSize sizeHint () const;

Q_SIGNALS:

    void leftClicked();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

protected:

    Qt::ArrowType m_arrowType;
    int           m_size;
    int           m_margin;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DLabelExpander : public QWidget
{
    Q_OBJECT

public:

    DLabelExpander(QWidget *parent = 0);
    ~DLabelExpander();

    void setLineVisible(bool b);
    void setText(const QString& text);
    void setPixmap(const QPixmap& pix);
    void setContainer(QWidget* widget);

    void setExpanded(bool b);
    bool isExpanded();

private slots:

    void slotToggleContainer();

private:

    DLabelExpanderPriv* const d;
};

} // namespace Digikam

#endif // CLICKLABEL_H
