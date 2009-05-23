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

#ifndef COMBOBOXUTILITIES_H
#define COMBOBOXUTILITIES_H

// Qt includes

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QComboBox>
#include <QtCore/QPersistentModelIndex>

// KDE includes

// Local includes

#include "digikam_export.h"

class QVBoxLayout;
class QTreeView;

namespace Digikam
{

class DIGIKAM_EXPORT ProxyLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    /**
     * This class will not act as a QLineEdit at all,
     * but present another widget (any kind of widget)
     * instead in the space assigned to the QLineEdit.
     * Use this class if you need to pass a QLineEdit but
     * want actually to use a different widget.
     */

    ProxyLineEdit(QWidget *parent = 0);
    /// After constructing, set the actual widget here
    void setWidget(QWidget *widget);

protected:

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void paintEvent(QPaintEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *event);
    void changeEvent(QEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void inputMethodEvent(QInputMethodEvent *event);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    QWidget     *m_widget;
    QVBoxLayout *m_layout;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ProxyClickLineEdit : public ProxyLineEdit
{
    Q_OBJECT

public:

    /**
     * A ProxyLineEdit that emits leftClicked() on
     * mouse press event.
     * Press on the held widget will result in the signal
     * if the widget does not accept() them.
     */

    ProxyClickLineEdit(QWidget *parent = 0);

Q_SIGNALS:

    void leftClicked();

protected:

    void mouseReleaseEvent(QMouseEvent *event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ModelIndexBasedComboBox : public QComboBox
{
public:

    /**
     * QComboBox has a current index based on a single integer.
     * This is not sufficient for more complex models.
     * This class is a combo box that stores a current index
     * based on QModelIndex.
     */

    ModelIndexBasedComboBox(QWidget *parent = 0);

    virtual void hidePopup();
    virtual void showPopup();

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& index);

protected:

    QPersistentModelIndex m_currentIndex;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT StayPoppedUpComboBox : public ModelIndexBasedComboBox
{
    Q_OBJECT

public:

    /** This class provides an abstract QComboBox with a custom view
     *  (which is created by implementing subclasses)
     *  instead of the usual QListView.
     *  The Pop-up of the combo box will stay open after selecting an item;
     *  it will be closed by clicking outside, but not inside the widget.
     *  You need three steps:
     *  Construct the object, call setModel() with an appropriate
     *  QAbstractItemModel, then call installView() to replace
     *  the standard combo box view with a view.
     */
    StayPoppedUpComboBox(QWidget *parent = 0);

protected:

    /** Implement in subclass:
     *  Send the given event to the viewportEvent() method of m_view.
     *  This method is protected for a usual QAbstractItemView.
     */
    virtual void sendViewportEventToView(QEvent *e) = 0;

    /** Replace the standard combo box list view with the given view.
     *  The view will be set as the view of the combo box
     *  (including re-parenting) and be stored in the m_view variable */
    void installView(QAbstractItemView *view);

    virtual bool eventFilter(QObject *watched, QEvent *event);

    QAbstractItemView   *m_view;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT TreeViewComboBox : public StayPoppedUpComboBox
{
    Q_OBJECT

public:

    /** This class provides a QComboBox with a QTreeView
     *  instead of the usual QListView.
     *  You need three steps:
     *  Construct the object, call setModel() with an appropriate
     *  QAbstractItemModel, then call installView() to replace
     *  the standard combo box view with a QTreeView.
     */
    TreeViewComboBox(QWidget *parent = 0);

    /** Replace the standard combo box list view with a QTreeView.
     *  Call this after installing an appropriate model. */
    virtual void installView();

    /** Returns the QTreeView of this class. Valid after installView() has been called */
    QTreeView *view() const;

protected:

    virtual void sendViewportEventToView(QEvent *e);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ListViewComboBox : public StayPoppedUpComboBox
{
    Q_OBJECT

public:

    /** This class provides an implementation of a StayPoppedUpComboBox
     *  with a QListView. This is the standard view of a QComboBox,
     *  but in conjunction with StayPoppedUpComboBox some extra steps are needed.
     *  You need three steps:
     *  Construct the object, call setModel() with an appropriate
     *  QAbstractItemModel, then call installView().
     */
    ListViewComboBox(QWidget *parent = 0);

    /** Replace the standard combo box list view with a QTreeView.
     *  Call this after installing an appropriate model. */
    virtual void installView();

    /** Returns the QTreeView of this class. Valid after installView() has been called */
    QListView *view() const;

protected:

    virtual void sendViewportEventToView(QEvent *e);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT TreeViewLineEditComboBox : public TreeViewComboBox
{
public:
    /** This class provides a TreeViewComboBox
     *  with a read-only line edit.
     *  The text in the line edit can be adjusted. The combo box will
     *  open on a click on the line edit.
     *  You need three steps:
     *  Construct the object, call setModel() with an appropriate
     *  QAbstractItemModel, then call installView() to replace
     *  the standard combo box view with a QTreeView.
     */

    TreeViewLineEditComboBox(QWidget *parent = 0);

    /** Replace the standard combo box list view with a QTreeView.
     *  Call this after installing an appropriate model. */
    virtual void installView();

    /** Set the text of the line edit (the text that is visible
        if the popup is not opened) */
    void setLineEditText(const QString& text);

protected:

    QLineEdit *m_comboLineEdit;
};

} // namespace Digikam

#endif // COMBOBOXUTILITIES_H
