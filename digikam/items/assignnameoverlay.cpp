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

#include <QApplication>
#include <QVBoxLayout>

// KDE includes

#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>

// Local includes

#include "addtagscompletionbox.h"
#include "addtagslineedit.h"
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
#include "searchutilities.h"

namespace Digikam
{

class AssignNameOverlay::AssignNameOverlayPriv
{
public:

    AssignNameOverlayPriv()
        : tagModel(AbstractAlbumModel::IgnoreRootAlbum)
    {
        assignNameWidget = 0;
    }

    TagModel                  tagModel;
    CheckableAlbumFilterModel filterModel;
    TagPropertiesFilterModel  filteredModel;

    FaceIface                 faceIface;

    AssignNameWidget*         assignNameWidget;
    QPersistentModelIndex     index;
};

AssignNameOverlay::AssignNameOverlay(QObject* parent)
    : AbstractWidgetDelegateOverlay(parent), d(new AssignNameOverlayPriv)
{
    d->filteredModel.setSourceAlbumModel(&d->tagModel);
    d->filterModel.setSourceFilterModel(&d->filteredModel);
}

AssignNameOverlay::~AssignNameOverlay()
{
    delete d;
}

AssignNameWidget* AssignNameOverlay::assignNameWidget() const
{
    return d->assignNameWidget;
}

QWidget* AssignNameOverlay::createWidget()
{
    d->assignNameWidget = new AssignNameWidget;
    d->assignNameWidget->setMode(AssignNameWidget::UnconfirmedEditMode);
    d->assignNameWidget->setVisualStyle(AssignNameWidget::TranslucentThemedFrameless);
    d->assignNameWidget->setTagEntryWidgetMode(AssignNameWidget::AddTagsLineEditMode);
    d->assignNameWidget->setLayoutMode(AssignNameWidget::Compact);
    d->assignNameWidget->setModel(&d->tagModel, &d->filteredModel, &d->filterModel);

    //new StyleSheetDebugger(d->assignNameWidget);

    QWidget* container = new QWidget(parentWidget());
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(d->assignNameWidget);
    container->setLayout(layout);

    return container;
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

        /*
                if (view()->model())
                    connect(view()->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                            this, SLOT(slotDataChanged(const QModelIndex&, const QModelIndex&)));
        */
    }
    else
    {
        // widget is deleted

        /*
                if (view() && view()->model())
                    disconnect(view()->model(), 0, this, 0);
        */
    }
}

void AssignNameOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void AssignNameOverlay::hide()
{
    AbstractWidgetDelegateOverlay::hide();
}

void AssignNameOverlay::updatePosition()
{
    if (!d->index.isValid())
    {
        return;
    }

    QRect rect = delegate()->imageInformationRect();

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
    {
        return;
    }

    QVariant extraData = d->index.data(ImageModel::ExtraDataRole);
    assignNameWidget()->setCurrentFace(DatabaseFace::fromVariant(extraData));
    assignNameWidget()->setUserData(ImageModel::retrieveImageInfo(d->index), extraData);
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
    {
        return false;
    }

    return DatabaseFace::fromVariant(extraData).isUnconfirmedType();
}

void AssignNameOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    /*
        // TODO: add again when fading in
        // see bug 228810, this is a small workaround
        if (m_widget && m_widget->isVisible() && m_index.isValid() && index == m_index)
            addTagsLineEdit()->setVisibleImmediately;
    */

    d->index = index;
    updatePosition();
    updateFace();
}

void AssignNameOverlay::viewportLeaveEvent(QObject*, QEvent*)
{
    // Dont hide when hovering the pop-up of the line edit.
    // isAncestorOf does not work: different window
    QWidget* widget = qApp->widgetAt(QCursor::pos());
    QWidget* parent = assignNameWidget();

    while (widget)
    {
        if (widget == parent)
        {
            return;
        }

        widget = widget->parentWidget();
    }

    hide();
}

void AssignNameOverlay::slotAssigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    DatabaseFace face = DatabaseFace::fromVariant(faceIdentifier);
    //kDebug() << "Confirming" << face << action.shallAssignTag() << action.tagId();

    if (face.isConfirmedName() || !action.isValid())
    {
        return;
    }

    int tagId = 0;

    if (action.shallAssignTag())
    {
        tagId = action.tagId();
    }
    else if (action.shallCreateNewTag())
    {
        tagId = d->faceIface.getOrCreateTagForPerson(action.newTagName(), action.parentTagId());
    }

    if (tagId)
    {
        emit confirmFaces(affectedIndexes(d->index), tagId);
    }

    hide();
}

void AssignNameOverlay::slotRejected(const ImageInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    Q_UNUSED(faceIdentifier);
    //DatabaseFace face = DatabaseFace::fromVariant(faceIdentifier);
    emit removeFaces(affectedIndexes(d->index));
    hide();
}

void AssignNameOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(d->index);
}

void AssignNameOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}


} // namespace Digikam
