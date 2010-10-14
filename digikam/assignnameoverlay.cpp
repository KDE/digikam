 /* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-14
 * Description : overlay for assigning names to faces
 *
 * Copyright (C) 2010 by Aditya Bhatt <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
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

#include "assignnameoverlay.moc"

// Qt includes

#include <QVBoxLayout>

// KDE includes

#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"
#include "assignnamewidget.h"
#include "databaseface.h"
#include "faceiface.h"
#include "facepipeline.h"
#include "imagedelegate.h"
#include "imagemodel.h"
#include "imagecategorizedview.h"
#include "taggingaction.h"
#include "tagscache.h"

namespace Digikam
{

class AssignNameOverlay::AssignNameOverlayPriv
{
public:

    AssignNameOverlayPriv()
        : tagModel(AbstractAlbumModel::IgnoreRootAlbum)
    {
    }

    TagModel                   tagModel;
    CheckableAlbumFilterModel  filterModel;
    TagPropertiesFilterModel   filteredModel;

    FaceIface                  faceIface;
    FacePipeline               trainPipeline;

    QPersistentModelIndex      index;
};

class AssignNameWidgetContainer : public QWidget
{
public:

    AssignNameWidgetContainer(AssignNameWidget *widget)
        : m_widget(widget)
    {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(m_widget);
        setLayout(layout);
    };

    AssignNameWidget *widget() const { return m_widget; }

protected:

    AssignNameWidget *m_widget;
};

AssignNameOverlay::AssignNameOverlay(QObject* parent)
                   : AbstractWidgetDelegateOverlay(parent), d(new AssignNameOverlayPriv)
{
    d->trainPipeline.plugTrainer();
    d->trainPipeline.construct();
}

AssignNameOverlay::~AssignNameOverlay()
{
    delete d;
}

AssignNameWidget* AssignNameOverlay::assignNameWidget() const
{
    AssignNameWidgetContainer *container = static_cast<AssignNameWidgetContainer*>(m_widget);
    if (container)
        return container->widget();
    return 0;
}

QWidget* AssignNameOverlay::createWidget()
{
    AssignNameWidget* assignWidget = new AssignNameWidget;
    assignWidget->setMode(AssignNameWidget::UnconfirmedEditMode);
    assignWidget->setVisualStyle(AssignNameWidget::TranslucentDarkRound);
    assignWidget->setLayoutMode(AssignNameWidget::Compact);
    assignWidget->setModel(&d->tagModel, &d->filteredModel, &d->filterModel);

    return new AssignNameWidgetContainer(assignWidget);
}

void AssignNameOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(assignNameWidget(), SIGNAL(assigned(const TaggingAction&, const ImageInfo&, const QVariant&)),
                this, SLOT(slotAssigned(const TaggingAction&, const ImageInfo&, const QVariant&)));

        connect(assignNameWidget(), SIGNAL(rejected(const ImageInfo&, const QVariant&)),
                this, SLOT(slotRejected(const ImageInfo&, const QVariant&)));

        /*if (view()->model())
            connect(view()->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(slotDataChanged(const QModelIndex &, const QModelIndex &)));
        */
    }
    else
    {
        // widget is deleted

        /*if (view() && view()->model())
            disconnect(view()->model(), 0, this, 0);
        */
    }
}

void AssignNameOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
        updatePosition();
}

void AssignNameOverlay::hide()
{
    AbstractWidgetDelegateOverlay::hide();
}

void AssignNameOverlay::updatePosition()
{
    if (!d->index.isValid())
        return;

    QRect rect      = delegate()->imageInformationRect();

    if (rect.width() < m_widget->minimumSizeHint().width())
    {
        int offset = (m_widget->minimumSizeHint().width() - rect.width()) / 2;
        rect.adjust(-offset, 0, offset, 0);
    }

    QRect visualRect = m_view->visualRect(d->index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width(), rect.height());
    m_widget->move(rect.topLeft());
}

void AssignNameOverlay::updateFace()
{
    if (!d->index.isValid() || !assignNameWidget())
        return;

    assignNameWidget()->setFace(ImageModel::retrieveImageInfo(d->index),
                                d->index.data(ImageModel::ExtraDataRole));
}

/*
void AssignNameOverlay::slotDataChanged(const QModelIndex& / *topLeft* /, const QModelIndex& / *bottomRight* /)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
        updateTag();
}
*/

bool AssignNameOverlay::checkIndex(const QModelIndex& index) const
{
    QVariant extraData = index.data(ImageModel::ExtraDataRole);
    if (extraData.isNull())
        return false;
    return DatabaseFace::fromVariant(extraData).isUnconfirmedType();
}

void AssignNameOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    /*
     * add again when fading in
    // see bug 228810, this is a small workaround
    if (m_widget && m_widget->isVisible() && m_index.isValid() && index == m_index)
        addTagsLineEdit()->setVisibleImmediately;
    */

    d->index = index;
    updatePosition();
    updateFace();
}

void AssignNameOverlay::slotAssigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier)
{
    DatabaseFace face = DatabaseFace::fromVariant(faceIdentifier);

    if (face.isConfirmedName() || !action.isValid())
        return;

    if (action.shallAssignTag())
    {
        face = d->faceIface.confirmName(face, action.tagId());
    }
    else if (action.shallCreateNewTag())
    {
        int tagId = d->faceIface.getOrCreateTagForPerson(action.newTagName(), action.parentTagId());
        face = d->faceIface.confirmName(face, tagId);
    }
    d->trainPipeline.train(QList<DatabaseFace>() << face, info);

    //TODO fast-remove if filtered by unconfirmed face etc.
    hide();
}

void AssignNameOverlay::slotRejected(const ImageInfo&, const QVariant& faceIdentifier)
{
    DatabaseFace face = DatabaseFace::fromVariant(faceIdentifier);
    d->faceIface.removeFace(face);
    hide();
}


} // namespace Digikam

