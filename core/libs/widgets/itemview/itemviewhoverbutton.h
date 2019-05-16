/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEM_VIEW_HOVER_BUTTON_H
#define DIGIKAM_ITEM_VIEW_HOVER_BUTTON_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "digikam_export.h"

class QTimeLine;

namespace Digikam
{

class DIGIKAM_EXPORT ItemViewHoverButton : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ItemViewHoverButton(QAbstractItemView* const parentView);

    void initIcon();
    void reset();
    void setIndex(const QModelIndex& index);
    QModelIndex index() const;
    void setVisible(bool visible) override;

    /// Reimplement to match the size of your icon
    virtual QSize sizeHint() const override = 0;

protected:

    /**
     * to call in children class contructors to init signal/slot connections.
     */
    void setup();

    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    /// Return your icon here. Will be queried again on toggle.
    virtual QIcon icon() = 0;
    /// Optionally update tooltip here. Will be called again on state change.
    virtual void updateToolTip();

protected Q_SLOTS:

    void setFadingValue(int value);
    void refreshIcon();
    void startFading();
    void stopFading();

protected:

    QPersistentModelIndex m_index;
    bool                  m_isHovered;
    int                   m_fadingValue;
    QIcon                 m_icon;
    QTimeLine*            m_fadingTimeLine;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_VIEW_HOVER_BUTTON_H
