 /* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rating icon view item at mouse hover
 *
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagslineeditoverlay.moc"

// Qt includes

#include <QMouseEvent>

// KDE includes

#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>

// Local includes

#include "imagedelegate.h"
#include "imagemodel.h"
#include "imagecategorizedview.h"
#include "addtagslineedit.h"

namespace Digikam
{

TagsLineEditOverlay::TagsLineEditOverlay(QObject* parent)
                   : AbstractWidgetDelegateOverlay(parent)
{
}

AddTagsLineEdit* TagsLineEditOverlay::addTagsLineEdit() const
{
    return static_cast<AddTagsLineEdit*>(m_widget);
}

QWidget* TagsLineEditOverlay::createWidget()
{
    //const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    AddTagsLineEdit* w = new AddTagsLineEdit(parentWidget());

    return w;
}

void TagsLineEditOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
    addTagsLineEdit()->setEnabled(true);
    addTagsLineEdit()->setClickMessage("Type the name of this person");
    addTagsLineEdit()->setReadOnly(false);

    if (active)
    {
        connect(addTagsLineEdit(), SIGNAL(taggingActionActivated(TaggingAction)),
                this, SLOT(slotTagChanged()));

        if (view()->model())
            connect(view()->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(slotDataChanged(const QModelIndex &, const QModelIndex &)));
    }
    else
    {
        // widget is deleted

        if (view() && view()->model())
            disconnect(view()->model(), 0, this, 0);
    }
}

void TagsLineEditOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
        updatePosition();
}

void TagsLineEditOverlay::hide()
{
    //delegate()->setRatingEdited(QModelIndex());
    AbstractWidgetDelegateOverlay::hide();
}

void TagsLineEditOverlay::updatePosition()
{
    if (!m_index.isValid())
        return;

    QRect thumbrect = delegate()->ratingRect();
    kDebug() << "Rect is : " << thumbrect;
    QRect rect      = thumbrect;

    if (rect.width() > addTagsLineEdit()->width() )
    {
        int offset = (rect.width() - addTagsLineEdit()->width()) / 2;
        rect.adjust(offset, 0, -offset, 0);
    }
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

void TagsLineEditOverlay::updateTag()
{
    if (!m_index.isValid())
        return;
    ImageInfo info = ImageModel::retrieveImageInfo(m_index);
    //TODO: ADD ratingWidget()->setRating(info.rating());
}

void TagsLineEditOverlay::slotTagChanged(int tagId)
{
    if (m_widget && m_widget->isVisible() && m_index.isValid())
        emit this->tagEdited(m_index, tagId);
}

void TagsLineEditOverlay::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
        updateTag();
}

void TagsLineEditOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    // see bug 228810, this is a small workaround
    if (m_widget && m_widget->isVisible() && m_index.isValid() && index == m_index)
        addTagsLineEdit()->setVisible(true);

    m_index = index;

    updatePosition();
    updateTag();

    //delegate()->setRatingEdited(m_index);
    view()->update(m_index);
}

} // namespace Digikam
