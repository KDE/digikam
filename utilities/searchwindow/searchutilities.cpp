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

// Qt includes

#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTreeView>

// KDE includes

// Local includes

#include "ddebug.h"
#include "albummodel.h"
#include "searchutilities.h"
#include "searchutilities.moc"

namespace Digikam
{


VisibilityController::VisibilityController(QObject *parent)
    : QObject(parent), m_status(Unknown), m_containerWidget(0)
{
}

void VisibilityController::addObject(VisibilityObject *object)
{
    m_objects << object;

    // create clean state
    if (m_status == Unknown)
    {
        if (object->isVisible())
            m_status = Shown;
        else
            m_status = Hidden;
    }

    // set state on object
    if (m_status == Shown || m_status == Showing)
        object->setVisible(true);
    else
        object->setVisible(false);
}

class VisibilityWidgetWrapper : public QObject, public VisibilityObject
{
public:

    VisibilityWidgetWrapper(VisibilityController *parent, QWidget *widget)
        : QObject(parent), m_widget(widget)
    {
    }

    virtual void setVisible(bool visible)
    {
        return m_widget->setVisible(visible);
    }

    virtual bool isVisible()
    {
        return m_widget->isVisible();
    }

    QWidget *m_widget;
};

void VisibilityController::addWidget(QWidget *widget)
{
    addObject(new VisibilityWidgetWrapper(this, widget));
}

void VisibilityController::setContainerWidget(QWidget *widget)
{
    m_containerWidget = widget;
}

void VisibilityController::setVisible(bool shallBeVisible)
{
    if (shallBeVisible)
    {
        if (m_status == Shown || m_status == Showing)
            return;
        m_status = Showing;
        beginStatusChange();
    }
    else
    {
        if (m_status == Hidden || m_status == Hiding)
            return;
        m_status = Hiding;
        beginStatusChange();
    }
}

void VisibilityController::show()
{
    setVisible(true);
}

void VisibilityController::hide()
{
    setVisible(false);
}

void VisibilityController::triggerVisibility()
{
    if (m_status == Shown || m_status == Showing || m_status == Unknown)
        setVisible(false);
    else
        setVisible(true);
}

void VisibilityController::beginStatusChange()
{
    allSteps();
}

void VisibilityController::step()
{
    if (m_status == Showing)
    {
        foreach(VisibilityObject *o, m_objects)
        {
            if (!o->isVisible())
            {
                o->setVisible(true);
                return;
            }
        }
    }
    else if (m_status == Hiding)
    {
        foreach(VisibilityObject *o, m_objects)
        {
            if (o->isVisible())
            {
                o->setVisible(false);
                return;
            }
        }
    }
}

void VisibilityController::allSteps()
{
    if (m_status == Showing)
    {
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(false);
        foreach(VisibilityObject *o, m_objects)
            o->setVisible(true);
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(true);
    }
    else if (m_status == Hiding)
    {
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(false);
        foreach(VisibilityObject *o, m_objects)
            o->setVisible(false);
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(true);
    }
}

// ----------------------------------- //

SearchClickLabel::SearchClickLabel(QWidget *parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

SearchClickLabel::SearchClickLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SearchClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

// ----------------------------------- //

SearchSqueezedClickLabel::SearchSqueezedClickLabel(QWidget *parent)
    : KSqueezedTextLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

SearchSqueezedClickLabel::SearchSqueezedClickLabel(const QString &text, QWidget *parent)
    : KSqueezedTextLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SearchSqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    KSqueezedTextLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

// ----------------------------------- //

ArrowClickLabel::ArrowClickLabel(QWidget *parent)
    : QWidget(parent),
      m_direction(Right)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_size = 8;
}

void ArrowClickLabel::setDirection(Direction dir)
{
    m_direction = dir;
    update();
}

void ArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

void ArrowClickLabel::paintEvent(QPaintEvent*)
{
    QStyle::PrimitiveElement e;
    QRect r(0, 0, m_size, m_size);

    switch (m_direction)
    {
        case Up:
            e = QStyle::PE_IndicatorArrowUp;
            break;
        case Down:
            e = QStyle::PE_IndicatorArrowDown;
            //r.translate(0, height() - 8);
            break;
        default:
        case Right:
            e = QStyle::PE_IndicatorArrowRight;
            //r.translate(width() - 8 , 0);
            break;
    }
    QStyleOption opt;
    opt.initFrom(this);
    opt.rect = r;

    QPainter p(this);
    style()->drawPrimitive(e, &opt, &p, this);
}

QSize ArrowClickLabel::sizeHint() const
{
    return QSize(m_size, m_size);
}

// ----------------------------------- //

class TreeViewComboBoxTreeView : public QTreeView
{
public:

    // Needed to make viewportEvent() public

    TreeViewComboBoxTreeView() : QTreeView() {}

    virtual bool viewportEvent(QEvent *event)
    {
        return QTreeView::viewportEvent(event);
    }
};

class TreeViewComboBoxLineEdit : public QLineEdit
{
public:

    // This line edit works like a weblink:
    // Readonly; A mouse press shows the popup; Cursor is the pointing hand.

    TreeViewComboBoxLineEdit(QComboBox *box) : QLineEdit()
    {
        m_box = box;
        setReadOnly(true);
        setCursor(Qt::PointingHandCursor);
    }

    virtual void mouseReleaseEvent(QMouseEvent *event)
    {
        QLineEdit::mouseReleaseEvent(event);
        m_box->showPopup();
    }

    QComboBox *m_box;
};

TreeViewComboBox::TreeViewComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void TreeViewComboBox::installView()
{
    // Create tree view
    m_view = new TreeViewComboBoxTreeView;

    // set on combo box
    setView(m_view);

    // Removing these event filters works just as the eventFilter() solution below,
    // but is much more dependent on Qt internals and not guaranteed to work in the future.
    //m_view->removeEventFilter(m_view->parent());
    //m_view->viewport()->removeEventFilter(m_view->parent());

    // Install event filters, _after_ setView() is called
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);

    m_comboLineEdit = new TreeViewComboBoxLineEdit(this);
    setLineEdit(m_comboLineEdit);
}

void TreeViewComboBox::setLineEditText(const QString &text)
{
    m_comboLineEdit->setText(text);
}

bool TreeViewComboBox::eventFilter(QObject *o, QEvent *e)
{
    // The combo box has installed an event filter on the view.
    // If it catches a valid mouse button release there, it will hide the popup.
    // Here we prevent this by eating the event ourselves,
    // and then dispatching it to its destination.
    if (o == m_view || o == m_view->viewport())
    {
        switch (e->type()) {
            case QEvent::MouseButtonRelease: {
                QMouseEvent *m = static_cast<QMouseEvent *>(e);
                if (m_view->isVisible() && m_view->rect().contains(m->pos()))
                {
                    if (o == m_view)
                        o->event(e);
                    else
                        // Viewport: Calling event() does not work, viewportEvent() is needed.
                        // This is the event that gets redirected to the QTreeView finally!
                        static_cast<TreeViewComboBoxTreeView*>(m_view)->viewportEvent(e);
                    return true;
                }
            }
            default:
                break;
        }
    }
    return QComboBox::eventFilter(o, e);
}

}

