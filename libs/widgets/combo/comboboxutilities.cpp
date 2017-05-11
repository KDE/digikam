/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "comboboxutilities.h"

// Qt includes

#include <QAbstractItemView>
#include <QAbstractListModel>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOption>
#include <QTreeView>
#include <QVBoxLayout>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

ProxyLineEdit::ProxyLineEdit(QWidget* const parent)
    : QLineEdit(parent),
      m_widget(0)
{
    m_layout = new QVBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(QMargins());
    setLayout(m_layout);

    // unset text edit cursor
    unsetCursor();

    // unset clear button per default
    setClearButtonShown(false);

    connect(this, SIGNAL(textChanged(QString)),
            this, SLOT(slotTextChanged(QString)));
}

void ProxyLineEdit::setWidget(QWidget* widget)
{
    if (m_widget)
    {
        delete m_widget;
    }

    m_widget = widget;
    m_widget->setParent(this);
    m_layout->addWidget(m_widget);
}

void ProxyLineEdit::setClearButtonShown(bool show)
{
    setClearButtonEnabled(show);

    int rightMargin = show ? height() : 0;
    m_layout->setContentsMargins(0, 0, rightMargin, 0);
}

void ProxyLineEdit::slotTextChanged(const QString& text)
{
    if (text.isEmpty() && isClearButtonEnabled())
    {
        emit signalClearButtonPressed();
    }
}

// NOTE: see bug #326718: We need to use QLineEdit parent class with these methods
// to have clear button working fine.
void ProxyLineEdit::mousePressEvent(QMouseEvent* event)
{
    QLineEdit::mousePressEvent(event);
}

void ProxyLineEdit::mouseReleaseEvent(QMouseEvent* event)
{
    QLineEdit::mouseReleaseEvent(event);
}

/**
 * We just re-implement all relevant QWidget event handlers and call
 * the QWidget implementation, not the QLineEdit one.
 */
void ProxyLineEdit::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
}

void ProxyLineEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void ProxyLineEdit::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
}

void ProxyLineEdit::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
}

void ProxyLineEdit::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
}

void ProxyLineEdit::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void ProxyLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
    QWidget::dragEnterEvent(event);
}

void ProxyLineEdit::dragMoveEvent(QDragMoveEvent* event)
{
    QWidget::dragMoveEvent(event);
}

void ProxyLineEdit::dragLeaveEvent(QDragLeaveEvent* event)
{
    QWidget::dragLeaveEvent(event);
}

void ProxyLineEdit::dropEvent(QDropEvent* event)
{
    QWidget::dropEvent(event);
}

void ProxyLineEdit::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
}

void ProxyLineEdit::contextMenuEvent(QContextMenuEvent* event)
{
    QWidget::contextMenuEvent(event);
}

void ProxyLineEdit::inputMethodEvent(QInputMethodEvent* event)
{
    QWidget::inputMethodEvent(event);
}

QSize ProxyLineEdit::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

QSize ProxyLineEdit::sizeHint() const
{
    return QWidget::sizeHint();
}

// -------------------------------------------------------------------------

ProxyClickLineEdit::ProxyClickLineEdit(QWidget* const parent)
    : ProxyLineEdit(parent)
{
}

void ProxyClickLineEdit::mouseReleaseEvent(QMouseEvent* event)
{
    ProxyLineEdit::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        event->accept();
    }
}

// -------------------------------------------------------------------------

ModelIndexBasedComboBox::ModelIndexBasedComboBox(QWidget* const parent)
    : QComboBox(parent)
{
}

void ModelIndexBasedComboBox::hidePopup()
{
    m_currentIndex = view()->selectionModel()->currentIndex();
    QComboBox::hidePopup();
}

void ModelIndexBasedComboBox::showPopup()
{
    QComboBox::showPopup();

    if (m_currentIndex.isValid())
    {
        view()->selectionModel()->setCurrentIndex(m_currentIndex, QItemSelectionModel::ClearAndSelect);
    }
}

QModelIndex ModelIndexBasedComboBox::currentIndex() const
{
    return m_currentIndex;
}

void ModelIndexBasedComboBox::setCurrentIndex(const QModelIndex& index)
{
    m_currentIndex = index;
    view()->selectionModel()->setCurrentIndex(m_currentIndex, QItemSelectionModel::ClearAndSelect);
}

// -------------------------------------------------------------------------

StayPoppedUpComboBox::StayPoppedUpComboBox(QWidget* const parent)
    : ModelIndexBasedComboBox(parent)
{
    m_view = 0;
}

void StayPoppedUpComboBox::installView(QAbstractItemView* view)
{
    if (m_view)
    {
        return;
    }

    // Create view
    m_view = view;

    // set on combo box
    setView(m_view);

    // Removing these event filters works just as the eventFilter() solution below,
    // but is much more dependent on Qt internals and not guaranteed to work in the future.
    //m_view->removeEventFilter(m_view->parent());
    //m_view->viewport()->removeEventFilter(m_view->parent());

    // Install event filters, _after_ setView() is called
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);
}

bool StayPoppedUpComboBox::eventFilter(QObject* o, QEvent* e)
{
    // The combo box has installed an event filter on the view.
    // If it catches a valid mouse button release there, it will hide the popup.
    // Here we prevent this by eating the event ourselves,
    // and then dispatching it to its destination.
    if (o == m_view || o == m_view->viewport())
    {
        switch (e->type())
        {
            case QEvent::MouseButtonRelease:
            {
                QMouseEvent* m = static_cast<QMouseEvent*>(e);

                if (m_view->isVisible() && m_view->rect().contains(m->pos()))
                {
                    if (o == m_view)
                    {
                        o->event(e);
                    }
                    else
                        // Viewport: Calling event() does not work, viewportEvent() is needed.
                        // This is the event that gets redirected to the QTreeView finally!
                    {
                        sendViewportEventToView(e);
                    }

                    // we have dispatched the event privately; we filter it out from the main dispatching
                    return true;
                }
                break;
            }
            case QEvent::ContextMenu:
            {
                if (o != m_view)
                {
                    // for whatever reason, the position of the event is slightly wrong
                    QContextMenuEvent* m = static_cast<QContextMenuEvent*>(e);
                    QPoint correctPos    = m_view->viewport()->mapFromGlobal(m->globalPos());
                    QContextMenuEvent corrected(m->reason(), correctPos, m->globalPos(), m->modifiers());
                    sendViewportEventToView(&corrected);
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }

    return QComboBox::eventFilter(o, e);
}

// -------------------------------------------------------------------------

class TreeViewComboBoxTreeView : public QTreeView
{
public:

    // Needed to make viewportEvent() public

    TreeViewComboBoxTreeView()
        : QTreeView()
    {
    }

    virtual bool viewportEvent(QEvent* event)
    {
        return QTreeView::viewportEvent(event);
    }
};

TreeViewComboBox::TreeViewComboBox(QWidget* const parent)
    : StayPoppedUpComboBox(parent)
{
}

void TreeViewComboBox::installView(QAbstractItemView* view)
{
    // parent does the heavy work
    StayPoppedUpComboBox::installView(view ? view : new TreeViewComboBoxTreeView);
}

void TreeViewComboBox::sendViewportEventToView(QEvent* e)
{
    static_cast<TreeViewComboBoxTreeView*>(m_view)->viewportEvent(e);
}

QTreeView* TreeViewComboBox::view() const
{
    return static_cast<QTreeView*>(m_view);
}

// -------------------------------------------------------------------------

class ListViewComboBoxListView : public QListView
{
public:

    // Needed to make viewportEvent() public

    ListViewComboBoxListView()
        : QListView()
    {
    }

    virtual bool viewportEvent(QEvent* event)
    {
        return QListView::viewportEvent(event);
    }
};

ListViewComboBox::ListViewComboBox(QWidget* const parent)
    : StayPoppedUpComboBox(parent)
{
}

void ListViewComboBox::installView(QAbstractItemView* view)
{
    // parent does the heavy work
    StayPoppedUpComboBox::installView(view ? view
                                           : new ListViewComboBoxListView);
}

void ListViewComboBox::sendViewportEventToView(QEvent* e)
{
    static_cast<ListViewComboBoxListView*>(m_view)->viewportEvent(e);
}

QListView* ListViewComboBox::view() const
{
    return static_cast<QListView*>(m_view);
}

// -------------------------------------------------------------------------

class TreeViewComboBoxLineEdit : public QLineEdit
{
public:

    // This line edit works like a weblink:
    // Readonly; A mouse press shows the popup; Cursor is the pointing hand.

    explicit TreeViewComboBoxLineEdit(QComboBox* const box)
        : QLineEdit(box)
    {
        m_box = box;
        setReadOnly(true);
        setCursor(Qt::PointingHandCursor);
    }

    virtual void mouseReleaseEvent(QMouseEvent* event)
    {
        QLineEdit::mouseReleaseEvent(event);
        m_box->showPopup();
    }

    virtual void wheelEvent(QWheelEvent* /*event*/)
    {
        m_box->showPopup();
    }

public:

    QComboBox* m_box;
};

TreeViewLineEditComboBox::TreeViewLineEditComboBox(QWidget* const parent)
    : TreeViewComboBox(parent),
      m_comboLineEdit(0)
{
}

void TreeViewLineEditComboBox::setLineEditText(const QString& text)
{
    if (m_comboLineEdit)
    {
        m_comboLineEdit->setText(text);
    }
}

void TreeViewLineEditComboBox::installView(QAbstractItemView* view)
{
    // parent does the heavy work
    TreeViewComboBox::installView(view);

    installLineEdit();
}

void TreeViewLineEditComboBox::installLineEdit()
{
    if (!m_comboLineEdit)
    {
        setLineEdit(new TreeViewComboBoxLineEdit(this));
    }
}

void TreeViewLineEditComboBox::setLineEdit(QLineEdit* edit)
{
    m_comboLineEdit = edit;
    TreeViewComboBox::setLineEdit(edit);
}

} // namespace Digikam
