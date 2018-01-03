/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "imagepreviewview.h"

// Qt includes

#include <QApplication>
#include <QGraphicsSceneContextMenuEvent>
#include <QMouseEvent>
#include <QToolBar>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMenu>
#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "imagepreviewviewitem.h"
#include "applicationsettings.h"
#include "contextmenuhelper.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dimgpreviewitem.h"
#include "imageinfo.h"
#include "fileactionmngr.h"
#include "metadatasettings.h"
#include "regionframeitem.h"
#include "tagspopupmenu.h"
#include "thememanager.h"
#include "previewlayout.h"
#include "previewsettings.h"
#include "tagscache.h"
#include "imagetagpair.h"
#include "albummanager.h"
#include "facegroup.h"

namespace Digikam
{

class ImagePreviewView::Private
{
public:

    Private()
    {
        fullSize            = 0;
        scale               = 1.0;
        item                = 0;
        isValid             = false;
        rotationLock        = false;
        toolBar             = 0;
        prevAction          = 0;
        nextAction          = 0;
        rotLeftAction       = 0;
        rotRightAction      = 0;
        mode                = ImagePreviewView::IconViewPreview;
        faceGroup           = 0;
        peopleToggleAction  = 0;
        addPersonAction     = 0;
        forgetFacesAction   = 0;
        fullscreenAction    = 0;
        currAlbum           = 0;
    }

    bool                   fullSize;
    double                 scale;
    bool                   isValid;
    bool                   rotationLock;

    ImagePreviewView::Mode mode;

    ImagePreviewViewItem*  item;

    QAction*               prevAction;
    QAction*               nextAction;
    QAction*               rotLeftAction;
    QAction*               rotRightAction;

    QToolBar*              toolBar;

    FaceGroup*             faceGroup;
    QAction*               peopleToggleAction;
    QAction*               addPersonAction;
    QAction*               forgetFacesAction;

    QAction*               fullscreenAction;

    Album*                 currAlbum;
};

ImagePreviewView::ImagePreviewView(QWidget* const parent, Mode mode, Album* currAlbum)
    : GraphicsDImgView(parent),
      d(new Private())
{
    d->mode      = mode;
    d->item      = new ImagePreviewViewItem();
    d->currAlbum = currAlbum;
    setItem(d->item);

    d->faceGroup = new FaceGroup(this);
    d->faceGroup->setShowOnHover(true);
    d->item->setFaceGroup(d->faceGroup);

    connect(d->item, SIGNAL(loaded()),
            this, SLOT(imageLoaded()));

    connect(d->item, SIGNAL(loadingFailed()),
            this, SLOT(imageLoadingFailed()));

    connect(d->item, SIGNAL(imageChanged()),
            this, SLOT(slotUpdateFaces()));

    connect(d->item, SIGNAL(showContextMenu(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(slotShowContextMenu(QGraphicsSceneContextMenuEvent*)));

    // set default zoom
    layout()->fitToWindow();

    // ------------------------------------------------------------

    installPanIcon();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),         i18nc("go to previous image", "Back"),  this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),             i18nc("go to next image", "Forward"),   this);
    d->rotLeftAction       = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-left")),  i18nc("@info:tooltip", "Rotate Left"),  this);
    d->rotRightAction      = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-right")), i18nc("@info:tooltip", "Rotate Right"), this);

    d->addPersonAction     = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),       i18n("Add a Face Tag"),                 this);
    d->forgetFacesAction   = new QAction(QIcon::fromTheme(QLatin1String("list-remove-user")),    i18n("Clear all faces on this image"),  this);
    d->peopleToggleAction  = new QAction(QIcon::fromTheme(QLatin1String("im-user")),             i18n("Show Face Tags"),                 this);
    d->peopleToggleAction->setCheckable(true);

    d->fullscreenAction    = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Show Fullscreen"), this);
    d->toolBar             = new QToolBar(this);

    if (mode == IconViewPreview)
    {
        d->toolBar->addAction(d->prevAction);
        d->toolBar->addAction(d->nextAction);
    }

    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);

    d->toolBar->addAction(d->peopleToggleAction);
    d->toolBar->addAction(d->addPersonAction);
    d->toolBar->addAction(d->fullscreenAction);

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(toPreviousImage()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(toNextImage()));

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

    connect(d->fullscreenAction, SIGNAL(triggered()),
            this, SIGNAL(signalSlideShowCurrent()));

    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalEscapePreview()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
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

    QStringList previewPaths;

    if (next.category() == DatabaseItem::Image)
    {
        previewPaths << next.filePath();
    }

    if (previous.category() == DatabaseItem::Image)
    {
        previewPaths << previous.filePath();
    }

    d->item->setPreloadPaths(previewPaths);
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

void ImagePreviewView::showEvent(QShowEvent* e)
{
    GraphicsDImgView::showEvent(e);
    d->faceGroup->setVisible(d->peopleToggleAction->isChecked());
}

void ImagePreviewView::slotShowContextMenu(QGraphicsSceneContextMenuEvent* event)
{
    ImageInfo info = d->item->imageInfo();

    if (info.isNull())
    {
        return;
    }

    event->accept();

    QList<qlonglong> idList;
    idList << info.id();

    // --------------------------------------------------------

    QMenu popmenu(this);
    ContextMenuHelper cmHelper(&popmenu);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    if (d->mode == IconViewPreview)
    {
        cmHelper.addAction(d->prevAction, true);
        cmHelper.addAction(d->nextAction, true);
        cmHelper.addSeparator();
    }

    // --------------------------------------------------------

    cmHelper.addAction(d->peopleToggleAction, true);
    cmHelper.addAction(d->addPersonAction,    true);
    cmHelper.addAction(d->forgetFacesAction,  true);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addOpenAndNavigateActions(idList);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_find_similar"));

    if (d->mode == IconViewPreview)
    {
        cmHelper.addStandardActionLightTable();
    }

    cmHelper.addQueueManagerMenu();
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_rotate"));
    cmHelper.addStandardActionItemDelete(this, SLOT(slotDeleteItem()));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    if (d->mode == IconViewPreview && d->currAlbum)
    {
        cmHelper.addStandardActionThumbnail(idList, d->currAlbum);
    }
    cmHelper.addAssignTagsMenu(idList);
    cmHelper.addRemoveTagsMenu(idList);
    cmHelper.addLabelsAction();

    // special action handling --------------------------------

    connect(&cmHelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmHelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmHelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmHelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmHelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(signalGotoTagAndItem(int)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ImageInfo)),
            this, SIGNAL(signalGotoAlbumAndItem(ImageInfo)));

    connect(&cmHelper, SIGNAL(signalGotoDate(ImageInfo)),
            this, SIGNAL(signalGotoDateAndItem(ImageInfo)));

    cmHelper.exec(event->screenPos());
}

void ImagePreviewView::slotAssignTag(int tagID)
{
    FileActionMngr::instance()->assignTag(d->item->imageInfo(), tagID);
}

void ImagePreviewView::slotRemoveTag(int tagID)
{
    FileActionMngr::instance()->removeTag(d->item->imageInfo(), tagID);
}

void ImagePreviewView::slotAssignPickLabel(int pickId)
{
    FileActionMngr::instance()->assignPickLabel(d->item->imageInfo(), pickId);
}

void ImagePreviewView::slotAssignColorLabel(int colorId)
{
    FileActionMngr::instance()->assignColorLabel(d->item->imageInfo(), colorId);
}

void ImagePreviewView::slotAssignRating(int rating)
{
    FileActionMngr::instance()->assignRating(d->item->imageInfo(), rating);
}

void ImagePreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), qApp->palette().color(QPalette::Base));
    setPalette(plt);
}

void ImagePreviewView::slotSetupChanged()
{
    previewItem()->setPreviewSettings(ApplicationSettings::instance()->getPreviewSettings());

    d->toolBar->setVisible(ApplicationSettings::instance()->getPreviewShowIcons());
    setShowText(ApplicationSettings::instance()->getPreviewShowIcons());

    // pass auto-suggest?
}

void ImagePreviewView::slotRotateLeft()
{
    if(d->rotationLock)
        return;

    d->rotationLock = true;

    /**
     * Setting lock won't allow mouse hover events in ImagePreviewViewItem class
     */
    d->item->setAcceptHoverEvents(false);

    /**
     * aboutToSetInfo will delete all face tags from FaceGroup storage
     */
    d->faceGroup->aboutToSetInfo(ImageInfo());

    FileActionMngr::instance()->transform(QList<ImageInfo>() << d->item->imageInfo(), MetaEngineRotation::Rotate270);
}

void ImagePreviewView::slotRotateRight()
{
    if(d->rotationLock)
        return;

    d->rotationLock = true;

    /**
     * Setting lock won't allow mouse hover events in ImagePreviewViewItem class
     */
    d->item->setAcceptHoverEvents(false);

    /**
     * aboutToSetInfo will delete all face tags from FaceGroup storage
     */
    d->faceGroup->aboutToSetInfo(ImageInfo());

    FileActionMngr::instance()->transform(QList<ImageInfo>() << d->item->imageInfo(), MetaEngineRotation::Rotate90);
}

void ImagePreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

void Digikam::ImagePreviewView::slotUpdateFaces()
{
    //d->faceGroup->aboutToSetInfo(ImageInfo());
    d->faceGroup->aboutToSetInfoAfterRotate(ImageInfo());
    d->item->setAcceptHoverEvents(true);

    /**
     * Release rotation lock after rotation
     */
    d->rotationLock = false;
}

void ImagePreviewView::dragMoveEvent(QDragMoveEvent* e)
{
    if (DTagListDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImagePreviewView::dragEnterEvent(QDragEnterEvent* e)
{
  if (DTagListDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImagePreviewView::dropEvent(QDropEvent* e)
{
    if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        QMenu popMenu(this);
        QAction* const assignToThisAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")), i18n("Assign Tags to &This Item"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("&Cancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice             = popMenu.exec(this->mapToGlobal(e->pos()));

        if(choice == assignToThisAction)
        {
            FileActionMngr::instance()->assignTags(d->item->imageInfo(),tagIDs);
        }
    }

    e->accept();
    return;
}

void ImagePreviewView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        d->faceGroup->addFace();
    }

    GraphicsDImgView::mousePressEvent(e);
}

}  // namespace Digikam
