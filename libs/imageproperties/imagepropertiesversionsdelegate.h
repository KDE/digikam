/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Item delegate for image versions list view
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEPROPERTIESVERSIONSDELEGATE_H
#define IMAGEPROPERTIESVERSIONSDELEGATE_H

// Qt includes

#include <QStyledItemDelegate>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class WorkingWidget;

class DIGIKAM_EXPORT ImagePropertiesVersionsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ImagePropertiesVersionsDelegate(QObject* parent = 0);
    ~ImagePropertiesVersionsDelegate();

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    WorkingWidget* getWidget() const;
    void           resetThumbsCounter();

public Q_SLOTS:

    void slotAnimationStep();

Q_SIGNALS:

    void throbberUpdated();

private:

    class ImagePropertiesVersionsDelegatePriv;
    ImagePropertiesVersionsDelegatePriv* const d;
};

} // namespace Digikam

#endif // IMAGEPROPERTIESVERSIONSDELEGATE_H
