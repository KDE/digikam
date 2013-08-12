/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-08
 * Description : Overlays for the showfoto
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTOOVERLAYS_H
#define SHOWFOTOOVERLAYS_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"
#include "itemviewshowfotodelegate.h"
#include "ratingwidget.h"

namespace ShowFoto
{

class ShowfotoOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ShowfotoOverlayWidget(QWidget* const parent = 0);

protected:

    virtual void paintEvent(QPaintEvent*);
};

// --------------------------------------------------------------------

class ShowfotoLockOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewShowfotoDelegate)

public:

    explicit ShowfotoLockOverlay(QObject* const parent);
    ShowfotoOverlayWidget* buttonWidget() const;

protected:

    void updatePosition();

    virtual QWidget* createWidget();
    virtual void setActive(bool active);
    virtual void visualChange();
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void slotEntered(const QModelIndex& index);

protected:

    QPersistentModelIndex m_index;
};

// --------------------------------------------------------------------

//class ShowfotoDownloadOverlay : public AbstractWidgetDelegateOverlay
//{
//    Q_OBJECT
//    REQUIRE_DELEGATE(ItemViewShowfotoDelegate)

//public:

//    explicit ShowfotoDownloadOverlay(QObject* const parent);
//    ShowfotoOverlayWidget* buttonWidget() const;

//protected:

//    void updatePosition();

//    virtual QWidget* createWidget();
//    virtual void setActive(bool active);
//    virtual void visualChange();
//    virtual bool checkIndex(const QModelIndex& index) const;
//    virtual void slotEntered(const QModelIndex& index);

//protected:

//    QPersistentModelIndex m_index;
//};

// ------------------------------------------------------------------------------------------------

class ShowfotoRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewShowfotoDelegate)

public:

    explicit ShowfotoRatingOverlay(QObject* const parent);
    RatingWidget* ratingWidget() const;

Q_SIGNALS:

    void ratingEdited(const QList<QModelIndex>& indexes, int rating);

protected Q_SLOTS:

    void slotRatingChanged(int);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    virtual QWidget* createWidget();
    virtual void setActive(bool);
    virtual void visualChange();
    virtual void hide();
    virtual void slotEntered(const QModelIndex& index);
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

    void updatePosition();
    void updateRating();

    QPersistentModelIndex m_index;
};

// ------------------------------------------------------------------------------------------------

enum ShowfotoRotateOverlayDirection
{
    ShowfotoRotateOverlayLeft,
    ShowfotoRotateOverlayRight
};

class ShowfotoRotateOverlayButton : public ItemViewHoverButton
{
public:

    ShowfotoRotateOverlayButton(ShowfotoRotateOverlayDirection dir, QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QPixmap icon();
    virtual void updateToolTip();

protected:

    ShowfotoRotateOverlayDirection const m_direction;
};

// --------------------------------------------------------------------

class ShowfotoRotateOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ShowfotoRotateOverlay(ShowfotoRotateOverlayDirection dir, QObject* const parent);
    virtual void setActive(bool active);

    ShowfotoRotateOverlayDirection direction() const     { return m_direction;                                               }
    bool isLeft() const                                { return m_direction  == ShowfotoRotateOverlayLeft;                   }
    bool isRight() const                               { return m_direction == ShowfotoRotateOverlayRight;                   }

    static ShowfotoRotateOverlay* left(QObject* parent)  { return new ShowfotoRotateOverlay(ShowfotoRotateOverlayLeft, parent);  }
    static ShowfotoRotateOverlay* right(QObject* parent) { return new ShowfotoRotateOverlay(ShowfotoRotateOverlayRight, parent); }

Q_SIGNALS:

    void signalRotate(const QList<QModelIndex>& indexes);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

private Q_SLOTS:

    void slotClicked();

private:

    ShowfotoRotateOverlayDirection const m_direction;
};

} // namespace ShowFoto


#endif // SHOWFOTOOVERLAYS_H
