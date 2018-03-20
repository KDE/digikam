/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2010-10-14
* Description : overlay for assigning names to faces
*
* Copyright (C) 2010      by Aditya Bhatt <caulier dot gilles at gmail dot com>
* Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
* Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
* Copyright (C) 2008      by Peter Penz <peter.penz@gmx.at>
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

#include "assignnameoverlay.h"

// Qt includes

#include <QApplication>
#include <QPushButton>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "assignnamewidget.h"
#include "facetagsiface.h"
#include "facepipeline.h"
#include "facetags.h"
#include "imagedelegate.h"
#include "imagemodel.h"
#include "imagecategorizedview.h"
#include "taggingaction.h"
#include "tagscache.h"
#include "searchutilities.h"
#include "applicationsettings.h"

namespace Digikam
{

class AssignNameOverlay::Private
{
public:

    Private()
        : tagModel(AbstractAlbumModel::IgnoreRootAlbum)
    {
        assignNameWidget = 0;
    }

    bool isChildWidget(QWidget* widget, QWidget* const parent) const
    {
        if (!parent)
        {
            return false;
        }

        // isAncestorOf may not work if widgets are located in different windows
        while (widget)
        {
            if (widget == parent)
            {
                return true;
            }

            widget = widget->parentWidget();
        }

        return false;
    }

public:

    TagModel                  tagModel;
    CheckableAlbumFilterModel filterModel;
    TagPropertiesFilterModel  filteredModel;

    AssignNameWidget*         assignNameWidget;
    QPersistentModelIndex     index;
};

AssignNameOverlay::AssignNameOverlay(QObject* const parent)
    : PersistentWidgetDelegateOverlay(parent),
      d(new Private)
{
    d->filteredModel.setSourceAlbumModel(&d->tagModel);
    d->filterModel.setSourceFilterModel(&d->filteredModel);
    // Restrict the tag properties filter model to people if configured.
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        if (settings->showOnlyPersonTagsInPeopleSidebar())
        {
            d->filteredModel.listOnlyTagsWithProperty(TagPropertyName::person());
        }
    }
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
    DVBox* const vbox    = new DVBox(parentWidget());
    QWidget* const space = new QWidget(vbox);
    d->assignNameWidget  = new AssignNameWidget(vbox);
    d->assignNameWidget->setMode(AssignNameWidget::UnconfirmedEditMode);
    d->assignNameWidget->setVisualStyle(AssignNameWidget::TranslucentThemedFrameless);
    d->assignNameWidget->setTagEntryWidgetMode(AssignNameWidget::AddTagsLineEditMode);
    d->assignNameWidget->setLayoutMode(AssignNameWidget::Compact);
    d->assignNameWidget->setModel(&d->tagModel, &d->filteredModel, &d->filterModel);
    d->assignNameWidget->lineEdit()->installEventFilter(this);

    vbox->setStretchFactor(space, 4);

    //new StyleSheetDebugger(d->assignNameWidget);

    return vbox;
}

void AssignNameOverlay::setActive(bool active)
{
    PersistentWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ImageInfo,QVariant)),
                this, SLOT(slotAssigned(TaggingAction,ImageInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(rejected(ImageInfo,QVariant)),
                this, SLOT(slotRejected(ImageInfo,QVariant)));

        connect(assignNameWidget(), SIGNAL(selected(TaggingAction,ImageInfo,QVariant)),
                this, SLOT(enterPersistentMode()));

        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ImageInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(rejected(ImageInfo,QVariant)),
                this, SLOT(leavePersistentMode()));

        connect(assignNameWidget(), SIGNAL(assigned(TaggingAction,ImageInfo,QVariant)),
                this, SLOT(storeFocus()));

        connect(assignNameWidget(), SIGNAL(rejected(ImageInfo,QVariant)),
                this, SLOT(storeFocus()));

/*
        if (view()->model())
            connect(view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
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
    PersistentWidgetDelegateOverlay::hide();
}

void AssignNameOverlay::updatePosition()
{
    if (!index().isValid())
    {
        return;
    }

    // See bug #365667.
    // Use information view below pixmap.
    // Depending of icon-view item options enabled in setup, the free space to use can be different.
    // We can continue to show the widget behind bottom of thumbnail view.

    QRect rect = delegate()->imageInformationRect();
    rect.setTop(delegate()->pixmapRect().top());

    if (rect.width() < m_widget->minimumSizeHint().width())
    {
        int offset = (m_widget->minimumSizeHint().width() - rect.width()) / 2;
        rect.adjust(-offset, 0, offset, 0);
    }

    QRect visualRect = m_view->visualRect(index());
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width(), rect.height());
    m_widget->move(rect.topLeft());
}

void AssignNameOverlay::updateFace()
{
    if (!index().isValid() || !assignNameWidget())
    {
        return;
    }

    QVariant extraData = index().data(ImageModel::ExtraDataRole);
    assignNameWidget()->setCurrentFace(FaceTagsIface::fromVariant(extraData));
    assignNameWidget()->setUserData(ImageModel::retrieveImageInfo(index()), extraData);
}

/*
void AssignNameOverlay::slotDataChanged(const QModelIndex& / *topLeft* /, const QModelIndex& / *bottomRight* /)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(index()))
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

    return FaceTagsIface::fromVariant(extraData).isUnconfirmedType();
}

void AssignNameOverlay::showOnIndex(const QModelIndex& index)
{
    PersistentWidgetDelegateOverlay::showOnIndex(index);

/*
    // TODO: add again when fading in
    // see bug 228810, this is a small workaround
    if (m_widget && m_widget->isVisible() && index().isValid() && index == index())
        addTagsLineEdit()->setVisibleImmediately;
*/

    updatePosition();
    updateFace();
}

void AssignNameOverlay::viewportLeaveEvent(QObject* o, QEvent* e)
{
    if (isPersistent() && m_widget->isVisible())
    {
        return;
    }

    // Do not hide when hovering the pop-up of the line edit.
    if (d->isChildWidget(qApp->widgetAt(QCursor::pos()), assignNameWidget()))
    {
        return;
    }

    PersistentWidgetDelegateOverlay::viewportLeaveEvent(o, e);
}

void AssignNameOverlay::slotAssigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    FaceTagsIface face = FaceTagsIface::fromVariant(faceIdentifier);

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Confirming" << face << action.shallAssignTag() << action.tagId();

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
        tagId = FaceTags::getOrCreateTagForPerson(action.newTagName(), -1);
    }

    if (tagId)
    {
        emit confirmFaces(affectedIndexes(index()), tagId);
    }

    hide();
}

void AssignNameOverlay::slotRejected(const ImageInfo& info, const QVariant& faceIdentifier)
{
    Q_UNUSED(info);
    Q_UNUSED(faceIdentifier);
    emit removeFaces(affectedIndexes(index()));
    hide();
}

void AssignNameOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(index());
}

void AssignNameOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

void AssignNameOverlay::setFocusOnWidget()
{
    if (assignNameWidget()->lineEdit())
    {
        assignNameWidget()->lineEdit()->selectAll();
        assignNameWidget()->lineEdit()->setFocus();
    }
}

bool AssignNameOverlay::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type())
    {
        case QEvent::MouseButtonPress:
        {
            enterPersistentMode();
            break;
        }
        case QEvent::FocusOut:
        {
            if (!d->isChildWidget(QApplication::focusWidget(), assignNameWidget()))
            {
                leavePersistentMode();
            }
            break;
        }
        default:
            break;
    }

    return PersistentWidgetDelegateOverlay::eventFilter(o, e);
}

} // namespace Digikam
