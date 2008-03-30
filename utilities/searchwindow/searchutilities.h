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

// Qt includes

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QComboBox>

// KDE includes

#include <ksqueezedtextlabel.h>

// Local includes


class QTreeView;

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

    enum Direction
    {
        Right,
        Up,
        Down
    };

    void setDirection(Direction dir);
    virtual QSize sizeHint () const;

signals:

    void leftClicked();

protected:

    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

    Direction m_direction;
    int       m_size;
};

class TreeViewComboBox : public QComboBox
{
    Q_OBJECT

public:

    /** This class provides a QComboBox with a QTreeView
     *  instead of the usual QListView.
     *  The text in the line edit can be adjusted. The combo box will
     *  open on a click on the line edit.
     *  You need three steps:
     *  Contruct the object, call setModel() with an appropriate
     *  QAbstractItemModel, then call installView() to replace
     *  the standard combo box view with a QTreeView.
     */
    TreeViewComboBox(QWidget *parent = 0);

    /** Replace the standard combo box list view with a QTreeView. */
    void installView();

    /** Set the text of the line edit (the text that is visible
        if the popup is not opened) */
    void setLineEditText(const QString &text);

protected:

    virtual bool eventFilter(QObject *watched, QEvent *event);

    QTreeView           *m_view;
    QLineEdit           *m_comboLineEdit;
};

}

#endif

