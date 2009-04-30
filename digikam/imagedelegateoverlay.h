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

// KDE includes

// Local includes


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
    virtual void mouseMoved(QMouseEvent *e, const QRect &visualRect, const QModelIndex &index);
    virtual void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index);

    void setView(ImageCategorizedView *view);
    ImageCategorizedView *view() const;
    void setDelegate(ImageDelegate *delegate);
    ImageDelegate *delegate() const;

Q_SIGNALS:

    void update(const QModelIndex &index);

protected Q_SLOTS:

    /** Called when any change from the delegate occurs - when the overlay is installed,
     *  when size hints, styles or fonts change */
    virtual void visualChange();

protected:

    ImageCategorizedView *m_view;
    ImageDelegate        *m_delegate;
};

// -------------------------------------------------------------------------------------------

class HoverWidgetDelegateOverlay : public ImageDelegateOverlay
{
    Q_OBJECT

public:

    HoverWidgetDelegateOverlay(QObject *parent);

    /** Will call createButton(). */
    virtual void setActive(bool active);

protected:

    /** Create your widget here. Pass view() as parent. */
    virtual ItemViewHoverButton *createButton() = 0;
    /** Called when a new index is entered. Reposition your button here,
     *  adjust and store state. */
    virtual void updateButton(const QModelIndex &index) = 0;

    virtual void visualChange();

protected Q_SLOTS:

    void slotReset();
    void slotEntered(const QModelIndex &index);
    void slotViewportEntered();
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);

protected:

    ItemViewHoverButton *m_button;
};

} // namespace Digikam

#endif /* IMAGEDELEGATEOVERLAY_H */
