/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : A widget to host settings as expander box
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

#ifndef REXPANDERBOX_H
#define REXPANDERBOX_H

// Qt includes

#include <QtCore/QObject>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QScrollArea>

// KDE includes

#include <ksqueezedtextlabel.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RClickLabel : public QLabel
{
    Q_OBJECT

public:

    RClickLabel(QWidget *parent = 0);
    RClickLabel(const QString& text, QWidget *parent = 0);
    ~RClickLabel(){};

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

class DIGIKAM_EXPORT RSqueezedClickLabel : public KSqueezedTextLabel
{
    Q_OBJECT

public:

    RSqueezedClickLabel(QWidget *parent = 0);
    RSqueezedClickLabel(const QString& text, QWidget *parent = 0);
    ~RSqueezedClickLabel(){};

Q_SIGNALS:

    void leftClicked();
    void activated();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT RArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    RArrowClickLabel(QWidget *parent = 0);
    ~RArrowClickLabel(){};

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

class RLabelExpanderPriv;

class DIGIKAM_EXPORT RLabelExpander : public QWidget
{
    Q_OBJECT

public:

    RLabelExpander(QWidget *parent = 0);
    ~RLabelExpander();

    void setLineVisible(bool b);
    void setText(const QString& text);
    void setPixmap(const QPixmap& pix);
    void setContainer(QWidget* widget);

    void setExpanded(bool b);
    bool isExpanded();

    void setExpandByDefault(bool b);
    bool expandByDefault();

private slots:

    void slotToggleContainer();

private:

    bool eventFilter(QObject *obj, QEvent *ev);

private:

    RLabelExpanderPriv* const d;
};

// -------------------------------------------------------------------------

class RExpanderBoxPriv;

class DIGIKAM_EXPORT RExpanderBox : public QScrollArea
{
    Q_OBJECT

public:

    RExpanderBox(QWidget *parent = 0);
    ~RExpanderBox();

    /** Add a DLabelExpander item to the box with all settings :
        'w'               : the widget hosted by DLabelExpander.
        'pix'             : pixmap used as icon to item title.
        'txt'             : text used as item title.
        'objName'         : item object name used to read/save expanded settings to rc file.
        'expandBydefault' : item state by default (expanded or not).
     */
    void addItem(QWidget *w, const QPixmap& pix, const QString& txt,
                 const QString& objName, bool expandBydefault);
    void addItem(QWidget *w, const QString& txt,
                 const QString& objName, bool expandBydefault);

    void removeItem(int index);

    void setItemIcon(int index, const QPixmap& pix);
    void addStretch();

    int  count();

    RLabelExpander* widget(int index) const;

    void setItemExpanded(int index, bool b);
    bool itemIsExpanded(int index);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private:

    RExpanderBoxPriv* const d;
};

} // namespace Digikam

#endif // REXPANDERBOX_H
