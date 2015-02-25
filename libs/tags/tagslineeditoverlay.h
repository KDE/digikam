/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : line edit for addition of tags on mouse hover
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef TAGSLINEEDITOVERLAY_H
#define TAGSLINEEDITOVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "imagedelegateoverlay.h"
#include "itemviewimagedelegate.h"

namespace Digikam
{

class AddTagsLineEdit;

class TagsLineEditOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImageDelegate)

public:

    explicit TagsLineEditOverlay(QObject* const parent);

    AddTagsLineEdit* addTagsLineEdit() const;

Q_SIGNALS:

    void tagEdited(const QModelIndex& index, int rating);
    void tagEdited(const QModelIndex& index, const QString&);

protected Q_SLOTS:

    void slotTagChanged(int);
    void slotTagChanged(const QString&);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    virtual QWidget* createWidget();
    virtual void setActive(bool);
    virtual void visualChange();
    virtual void slotEntered(const QModelIndex& index);
    virtual void hide();

    void updatePosition();
    void updateTag();

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam

#endif /* IMAGERATINGOVERLAY_H */
