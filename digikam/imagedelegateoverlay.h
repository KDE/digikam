/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-29
 * Description : Qt item view for images - delegate additions
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEDELEGATEOVERLAY_H
#define IMAGEDELEGATEOVERLAY_H

// Qt includes

#include <QAbstractItemView>

namespace Digikam
{

class ImageCategorizedView;
class ImageDelegate;
class ItemViewHoverButton;

class ImageDelegateOverlay : public QObject
{
    Q_OBJECT

public:

    ImageDelegateOverlay(QObject *parent = 0);
    ~ImageDelegateOverlay();

    /** Called when the overlay was installed and shall begin working,
     *  and before it is removed and shall stop.
     *  Setup your connections to view and delegate here.
     *  You will be disconnected automatically on removal. */
    virtual void setActive(bool active);

    /** Only these two methods are implemented as virtual methods.
     *  For all other events, connect to the view's signals.
     *  There are a few signals specifically for overlays and all
     *  QAbstractItemView standard signals. */
    virtual void mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex& index);
    virtual void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index);

    void setView(ImageCategorizedView *view);
    ImageCategorizedView *view() const;
    void setDelegate(ImageDelegate *delegate);
    ImageDelegate *delegate() const;

Q_SIGNALS:

    void update(const QModelIndex& index);

protected Q_SLOTS:

    /** Called when any change from the delegate occurs - when the overlay is installed,
     *  when size hints, styles or fonts change */
    virtual void visualChange();

protected:

    ImageCategorizedView *m_view;
    ImageDelegate        *m_delegate;
};

// -------------------------------------------------------------------------------------------

class AbstractWidgetDelegateOverlay : public ImageDelegateOverlay
{
    Q_OBJECT

public:

    /** This class provides functionality for using a widget in an overlay.
     *  You must reimplement at least createWidget to return your widget.
     *  Per default it will be shown when the cursor enters an index and hidden when left.
     *  Reimplement slotEntered() and mouseMove() for more fine grained control. */
    AbstractWidgetDelegateOverlay(QObject *parent);

    /** If active is true, this will call createWidget(), initialize the widget for use,
     *  and setup connections for the virtual slots.
     *  If active is false, this will delete the widget and
     *  disconnect all signal from model and view to this object (!) */
    virtual void setActive(bool active);

protected:

    /** Create your widget here. When creating the object, pass parentWidget() as parent widget.
     *  Ownership of the object is passed. It will be deleted in setActive(false). */
    virtual QWidget *createWidget() = 0;
    /** Called when the widget shall be hidden (mouse cursor left index, viewport, uninstalled etc.).
     *  Default implementation hide()s m_widget. */
    virtual void hide();

    /// Returns the widget to be used as parent for your widget created in createWidget()
    QWidget *parentWidget() const;

protected Q_SLOTS:

    /** Default implementation shows or hides the widget if index is valid or not valid */
    virtual void slotEntered(const QModelIndex& index);
    /** Default implementations of these three slots call hide() */
    virtual void slotReset();
    virtual void slotViewportEntered();
    virtual void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    virtual void slotLayoutChanged();

protected:

    bool eventFilter(QObject* obj, QEvent* event);

    QWidget *m_widget;

    bool m_mouseButtonPressedOnWidget;
};

class HoverButtonDelegateOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT

public:

    HoverButtonDelegateOverlay(QObject *parent);

    /** Will call createButton(). */
    virtual void setActive(bool active);

    ItemViewHoverButton *button() const;

protected:

    /** Create your widget here. Pass view() as parent. */
    virtual ItemViewHoverButton *createButton() = 0;
    /** Called when a new index is entered. Reposition your button here,
     *  adjust and store state. */
    virtual void updateButton(const QModelIndex& index) = 0;

    virtual QWidget *createWidget();
    virtual void visualChange();


protected Q_SLOTS:

    virtual void slotEntered(const QModelIndex& index);
    virtual void slotReset();

};

} // namespace Digikam

#endif /* IMAGEDELEGATEOVERLAY_H */
