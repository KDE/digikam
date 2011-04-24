/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "imagepreviewview.moc"

// Qt includes

#include <QCursor>
#include <QGraphicsSceneContextMenuEvent>
#include <QToolBar>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <ktoggleaction.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dimgpreviewitem.h"
#include "dpopupmenu.h"
#include "facegroup.h"
#include "imageinfo.h"
#include "metadatamanager.h"
#include "metadatasettings.h"
#include "regionframeitem.h"
#include "tagspopupmenu.h"
#include "thememanager.h"
#include "previewlayout.h"
#include "tagscache.h"
#include "imagetagpair.h"
#include "albummanager.h"
#include "faceiface.h"

namespace Digikam
{

class ImagePreviewViewItem : public DImgPreviewItem
{
public:

    ImagePreviewViewItem(ImagePreviewView* view)
        : m_view(view), m_group(0)
    {
        setAcceptHoverEvents(true);
    }

    void setFaceGroup(FaceGroup* group)
    {
        m_group = group;
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
    {
        m_view->showContextMenu(m_info, event);
    }

    void setImageInfo(const ImageInfo& info)
    {
        m_info = info;
        setPath(info.filePath());
    }

    void hoverEnterEvent (QGraphicsSceneHoverEvent* e)
    {
        m_group->itemHoverEnterEvent(e);
    }

    void hoverLeaveEvent (QGraphicsSceneHoverEvent* e)
    {
        m_group->itemHoverLeaveEvent(e);
    }

    void hoverMoveEvent (QGraphicsSceneHoverEvent* e)
    {
        m_group->itemHoverMoveEvent(e);
    }

    ImageInfo imageInfo() const
    {
        return m_info;
    }

protected:

    ImagePreviewView* m_view;
    FaceGroup*        m_group;
    ImageInfo         m_info;
};

// ---------------------------------------------------------------------

class ImagePreviewView::ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        peopleTagsShown    = false;
        fullSize           = 0;
        scale              = 1.0;
        item               = 0;
        isValid            = false;
        toolBar            = 0;
        back2AlbumAction   = 0;
        prevAction         = 0;
        nextAction         = 0;
        rotLeftAction      = 0;
        rotRightAction     = 0;
        peopleToggleAction = 0;
        addPersonAction    = 0;
        faceGroup          = 0;
        mode               = ImagePreviewView::IconViewPreview;
    }

    bool                   peopleTagsShown;
    bool                   fullSize;
    double                 scale;
    bool                   isValid;

    ImagePreviewView::Mode mode;

    ImagePreviewViewItem*  item;

    QAction*               back2AlbumAction;
    QAction*               prevAction;
    QAction*               nextAction;
    QAction*               rotLeftAction;
    QAction*               rotRightAction;
    KToggleAction*         peopleToggleAction;
    QAction*               addPersonAction;
    QAction*               forgetFacesAction;

    QToolBar*              toolBar;

    FaceGroup*             faceGroup;
};

ImagePreviewView::ImagePreviewView(QWidget* parent, Mode mode)
    : GraphicsDImgView(parent), d(new ImagePreviewViewPriv)
{
    d->mode = mode;
    d->item = new ImagePreviewViewItem(this);
    setItem(d->item);

    d->faceGroup = new FaceGroup(this);
    d->faceGroup->setShowOnHover(true);

    d->item->setFaceGroup(d->faceGroup);

    connect(d->item, SIGNAL(loaded()),
            this, SLOT(imageLoaded()));

    connect(d->item, SIGNAL(loadingFailed()),
            this, SLOT(imageLoadingFailed()));

    // set default zoom
    layout()->fitToWindow();

    // ------------------------------------------------------------

    installPanIcon();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    d->back2AlbumAction   = new QAction(SmallIcon("folder-image"),        i18n("Back to Album"),                  this);
    d->prevAction         = new QAction(SmallIcon("go-previous"),         i18nc("go to previous image", "Back"),  this);
    d->nextAction         = new QAction(SmallIcon("go-next"),             i18nc("go to next image", "Forward"),   this);
    d->rotLeftAction      = new QAction(SmallIcon("object-rotate-left"),  i18nc("@info:tooltip", "Rotate Left"),  this);
    d->rotRightAction     = new QAction(SmallIcon("object-rotate-right"), i18nc("@info:tooltip", "Rotate Right"), this);
    d->addPersonAction    = new QAction(SmallIcon("list-add-user"),       i18n("Add a Face Tag"),                 this);
    d->forgetFacesAction  = new QAction(SmallIcon("list-remove-user"),    i18n("Clear all faces on this image"),  this);
    d->peopleToggleAction = new KToggleAction(i18n("Show Face Tags"),                                             this);
    d->peopleToggleAction->setIcon(SmallIcon("user-identity"));

    d->toolBar = new QToolBar(this);

    if (mode == IconViewPreview)
    {
        d->toolBar->addAction(d->prevAction);
        d->toolBar->addAction(d->nextAction);
        d->toolBar->addAction(d->back2AlbumAction);
    }
    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);
    d->toolBar->addAction(d->peopleToggleAction);
    d->toolBar->addAction(d->addPersonAction);

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(toPreviousImage()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(toNextImage()));

    connect(d->back2AlbumAction, SIGNAL(triggered()),
            this, SIGNAL(signalBack2Album()));

    connect(d->rotLeftAction, SIGNAL(triggered()),
            this, SLOT(slotRotateLeft()));

    connect(d->rotRightAction, SIGNAL(triggered()),
            this, SLOT(slotRotateRight()));

    connect(d->peopleToggleAction, SIGNAL(toggled(bool)),
            d->faceGroup, SLOT(setVisible(bool)));

    connect(d->addPersonAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(addFace()));

    connect(d->forgetFacesAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(rejectAll()));

    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalBack2Album()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImagePreviewView::~ImagePreviewView()
{
    delete d->item;
    delete d;
}

void ImagePreviewView::reload()
{
    previewItem()->reload();
}

void ImagePreviewView::imageLoaded()
{
    emit signalPreviewLoaded(true);
    d->rotLeftAction->setEnabled(true);
    d->rotRightAction->setEnabled(true);

    d->faceGroup->setInfo(d->item->imageInfo());
}

void ImagePreviewView::imageLoadingFailed()
{
    emit signalPreviewLoaded(false);
    d->rotLeftAction->setEnabled(false);
    d->rotRightAction->setEnabled(false);
    d->faceGroup->setInfo(ImageInfo());
}

void ImagePreviewView::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    d->faceGroup->aboutToSetInfo(info);
    d->item->setImageInfo(info);

    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    d->item->setPreloadPaths(QStringList() << next.filePath() << previous.filePath());
}

ImageInfo ImagePreviewView::getImageInfo() const
{
    return d->item->imageInfo();
}

bool ImagePreviewView::acceptsMouseClick(QMouseEvent* e)
{
    if (!GraphicsDImgView::acceptsMouseClick(e))
    {
        return false;
    }

    return d->faceGroup->acceptsMouseClick(mapToScene(e->pos()));
}

void ImagePreviewView::enterEvent(QEvent* e)
{
    d->faceGroup->enterEvent(e);
}

void ImagePreviewView::leaveEvent(QEvent* e)
{
    d->faceGroup->leaveEvent(e);
}

void ImagePreviewView::showContextMenu(const ImageInfo& info, QGraphicsSceneContextMenuEvent* event)
{
    if (info.isNull())
    {
        return;
    }

    event->accept();

    QList<qlonglong> idList;
    idList << info.id();
    KUrl::List selectedItems;
    selectedItems << info.fileUrl();

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(d->peopleToggleAction, true);
    popmenu.addSeparator();
    cmhelper.addAction(d->addPersonAction, true);
    cmhelper.addAction(d->forgetFacesAction, true);

    // --------------------------------------------------------

    if (d->mode == IconViewPreview)
    {
        cmhelper.addAction(d->prevAction, true);
        cmhelper.addAction(d->nextAction, true);
        cmhelper.addAction(d->back2AlbumAction);
        cmhelper.addGotoMenu(idList);
        popmenu.addSeparator();
    }

    // --------------------------------------------------------

    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addKipiActions(idList);
    popmenu.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction("image_find_similar");
    cmhelper.addStandardActionLightTable();
    cmhelper.addQueueManagerMenu();
    popmenu.addSeparator();

    // --------------------------------------------------------

    cmhelper.addStandardActionItemDelete(this, SLOT(slotDeleteItem()));
    popmenu.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    popmenu.addSeparator();

    // --------------------------------------------------------

    cmhelper.addLabelsAction();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(signalGotoTagAndItem(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(const ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(const ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(const ImageInfo&)));

    cmhelper.exec(event->screenPos());
}

void ImagePreviewView::slotAssignTag(int tagID)
{
    MetadataManager::instance()->assignTag(d->item->imageInfo(), tagID);
}

void ImagePreviewView::slotRemoveTag(int tagID)
{
    MetadataManager::instance()->removeTag(d->item->imageInfo(), tagID);
}

void ImagePreviewView::slotAssignPickLabel(int pickId)
{
    MetadataManager::instance()->assignPickLabel(d->item->imageInfo(), pickId);
}

void ImagePreviewView::slotAssignColorLabel(int colorId)
{
    MetadataManager::instance()->assignColorLabel(d->item->imageInfo(), colorId);
}

void ImagePreviewView::slotAssignRating(int rating)
{
    MetadataManager::instance()->assignRating(d->item->imageInfo(), rating);
}

void ImagePreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), kapp->palette().color(QPalette::Base));
    setPalette(plt);
}

void ImagePreviewView::slotSetupChanged()
{
    previewItem()->setLoadFullImageSize(AlbumSettings::instance()->getPreviewLoadFullImageSize());
    previewItem()->setExifRotate(MetadataSettings::instance()->settings().exifRotate);

    // pass auto-suggest?
}

void ImagePreviewView::slotRotateLeft()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());

    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_ccw"))
            {
                ac->trigger();
            }
        }
    }
}

void ImagePreviewView::slotRotateRight()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());

    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_cw"))
            {
                ac->trigger();
            }
        }
    }
}

}  // namespace Digikam
