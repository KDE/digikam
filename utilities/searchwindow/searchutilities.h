/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SEARCHUTILITIES_H
#define SEARCHUTILITIES_H

// Qt includes.

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QComboBox>

// KDE includes.

#include <ksqueezedtextlabel.h>

// Local includes.

#include "comboboxutilities.h"

class QVBoxLayout;
class QTextEdit;
class QTimeLine;
class KPushButton;

namespace Digikam
{

class VisibilityObject
{
public:
    virtual ~VisibilityObject() {}

    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible() = 0;
};

class VisibilityController : public QObject
{
    Q_OBJECT

public:

    VisibilityController(QObject *parent);
    void setContainerWidget(QWidget *widget);
    void addObject(VisibilityObject *object);
    void addWidget(QWidget *widget);

    bool isVisible() const;

public slots:

    void setVisible(bool visible);
    void show();
    void hide();
    void triggerVisibility();

protected:

    enum Status
    {
        Unknown,
        Hidden,
        Showing,
        Shown,
        Hiding
    };

    virtual void beginStatusChange();
    void step();
    void allSteps();

    Status                    m_status;
    QList<VisibilityObject *> m_objects;
    QWidget                  *m_containerWidget;
};

class SearchClickLabel : public QLabel
{
    Q_OBJECT

public:

    SearchClickLabel(QWidget *parent = 0);
    SearchClickLabel(const QString &text, QWidget *parent = 0);

signals:

    void leftClicked();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
};

class SearchSqueezedClickLabel : public KSqueezedTextLabel
{
    Q_OBJECT

public:

    SearchSqueezedClickLabel(QWidget *parent = 0);
    SearchSqueezedClickLabel(const QString &text, QWidget *parent = 0);

signals:

    void leftClicked();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
};

class ArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    ArrowClickLabel(QWidget *parent = 0);

    void setArrowType(Qt::ArrowType arrowType);
    virtual QSize sizeHint () const;

signals:

    void leftClicked();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

    Qt::ArrowType m_arrowType;
    int           m_size;
    int           m_margin;
};

class AnimatedClearButton : public QWidget
{
    Q_OBJECT

public:

    AnimatedClearButton(QWidget *parent = 0);

    QSize sizeHint () const;

    void setPixmap(const QPixmap& p);
    QPixmap pixmap();

    /// Set visible, possibly with animation
    void animateVisible(bool visible);
    /// Set visible without animation
    void setDirectlyVisible(bool visible);

    /** This parameter determines the behavior when the animation
     *  to hide the widget has finished:
     *  If stayVisible is true, the widget remains visible,
     *  but paints nothing.
     *  If stayVisible is false, setVisible(false) is called,
     *  which removes the widget for layouting etc.
     *  Default: false */
    void stayVisibleWhenAnimatedOut(bool stayVisible);

signals:

    void clicked();

protected:

    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

protected slots:

    void animationFinished();
    void updateAnimationSettings();

private:
    QTimeLine *m_timeline;
    QPixmap    m_pixmap;
    bool       m_stayAlwaysVisible;
};

class StyleSheetDebugger : public QWidget
{
    Q_OBJECT

public:

    /** This widget is for development purpose only:
     *  It allows the developer to change the style sheet
     *  on a widget dynamically.
     *  If you want to develop or debug the stylesheet on your widget,
     *  add temporary code:
     *  new StyleSheetDebugger(myWidget);
     *  That's all. Change the style sheet by editing it and pressing Ok. */

    StyleSheetDebugger(QWidget *object);

protected slots:

    void buttonClicked();

protected:

    QTextEdit      *m_edit;
    KPushButton    *m_okButton;
    QWidget        *m_widget;
};

}

#endif

