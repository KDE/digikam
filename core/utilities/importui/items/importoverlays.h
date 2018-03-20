/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-08-21
 * Description : Overlays for the import interface
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMPORTOVERLAYS_H
#define IMPORTOVERLAYS_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"
#include "itemviewimportdelegate.h"
#include "ratingwidget.h"

namespace Digikam
{

class ImportOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ImportOverlayWidget(QWidget* const parent = 0);

protected:

    virtual void paintEvent(QPaintEvent*);
};

// --------------------------------------------------------------------

class ImportCoordinatesOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportCoordinatesOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget() const;

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

class ImportLockOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportLockOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget() const;

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

class ImportDownloadOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportDownloadOverlay(QObject* const parent);
    ImportOverlayWidget* buttonWidget() const;

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

// ------------------------------------------------------------------------------------------------

class ImportRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImportDelegate)

public:

    explicit ImportRatingOverlay(QObject* const parent);
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

enum ImportRotateOverlayDirection
{
    ImportRotateOverlayLeft,
    ImportRotateOverlayRight
};

class ImportRotateOverlayButton : public ItemViewHoverButton
{
public:

    ImportRotateOverlayButton(ImportRotateOverlayDirection dir, QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();

protected:

    ImportRotateOverlayDirection const m_direction;
};

// --------------------------------------------------------------------

class ImportRotateOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ImportRotateOverlay(ImportRotateOverlayDirection dir, QObject* const parent);
    virtual void setActive(bool active);

    ImportRotateOverlayDirection direction() const     { return m_direction;                                               }
    bool isLeft() const                                { return m_direction  == ImportRotateOverlayLeft;                   }
    bool isRight() const                               { return m_direction == ImportRotateOverlayRight;                   }

    static ImportRotateOverlay* left(QObject* parent)  { return new ImportRotateOverlay(ImportRotateOverlayLeft, parent);  }
    static ImportRotateOverlay* right(QObject* parent) { return new ImportRotateOverlay(ImportRotateOverlayRight, parent); }

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

    ImportRotateOverlayDirection const m_direction;
};

} // namespace Digikam

#endif // IMPORTOVERLAYS_H
