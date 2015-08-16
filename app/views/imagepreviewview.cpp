/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepreviewview.moc"

// Qt includes

#include <QCursor>
#include <QGraphicsSceneContextMenuEvent>
#include <QMouseEvent>
#include <QToolBar>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

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


// Local includes

#include "config-digikam.h"
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

#ifdef HAVE_KFACE
#include "facegroup.h"
#endif /* HAVE_KFACE */

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
#ifdef HAVE_KFACE
        faceGroup           = 0;
        peopleToggleAction  = 0;
        addPersonAction     = 0;
        forgetFacesAction   = 0;
#endif /* HAVE_KFACE */
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

#ifdef HAVE_KFACE
    FaceGroup*             faceGroup;
    KToggleAction*         peopleToggleAction;
    QAction*               addPersonAction;
    QAction*               forgetFacesAction;
#endif /* HAVE_KFACE */
};

ImagePreviewView::ImagePreviewView(QWidget* const parent, Mode mode)
    : GraphicsDImgView(parent), d(new Private)
{
    d->mode      = mode;
    d->item      = new ImagePreviewViewItem();
    setItem(d->item);

#ifdef HAVE_KFACE
    d->faceGroup = new FaceGroup(this);
    d->faceGroup->setShowOnHover(true);

    d->item->setFaceGroup(d->faceGroup);
#endif /* HAVE_KFACE */

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

    d->prevAction          = new QAction(SmallIcon("go-previous"),         i18nc("go to previous image", "Back"),  this);
    d->nextAction          = new QAction(SmallIcon("go-next"),             i18nc("go to next image", "Forward"),   this);
    d->rotLeftAction       = new QAction(SmallIcon("object-rotate-left"),  i18nc("@info:tooltip", "Rotate Left"),  this);
    d->rotRightAction      = new QAction(SmallIcon("object-rotate-right"), i18nc("@info:tooltip", "Rotate Right"), this);

#ifdef HAVE_KFACE
    d->addPersonAction     = new QAction(SmallIcon("list-add-user"),       i18n("Add a Face Tag"),                 this);
    d->forgetFacesAction   = new QAction(SmallIcon("list-remove-user"),    i18n("Clear all faces on this image"),  this);
    d->peopleToggleAction  = new KToggleAction(i18n("Show Face Tags"),                                             this);
    d->peopleToggleAction->setIcon(SmallIcon("user-identity"));
#endif /* HAVE_KFACE */

    d->toolBar             = new QToolBar(this);

    if (mode == IconViewPreview)
    {
        d->toolBar->addAction(d->prevAction);
        d->toolBar->addAction(d->nextAction);
    }

    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);

#ifdef HAVE_KFACE
    d->toolBar->addAction(d->peopleToggleAction);
    d->toolBar->addAction(d->addPersonAction);
#endif /* HAVE_KFACE */

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(toPreviousImage()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(toNextImage()));

    connect(d->rotLeftAction, SIGNAL(triggered()),
            this, SLOT(slotRotateLeft()));

    connect(d->rotRightAction, SIGNAL(triggered()),
            this, SLOT(slotRotateRight()));

#ifdef HAVE_KFACE
    connect(d->peopleToggleAction, SIGNAL(toggled(bool)),
            d->faceGroup, SLOT(setVisible(bool)));

    connect(d->addPersonAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(addFace()));

    connect(d->forgetFacesAction, SIGNAL(triggered()),
            d->faceGroup, SLOT(rejectAll()));
#endif /* HAVE_KFACE */

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

#ifdef HAVE_KFACE
    d->faceGroup->setInfo(d->item->imageInfo());
#endif /* HAVE_KFACE */
}

void ImagePreviewView::imageLoadingFailed()
{
    emit signalPreviewLoaded(false);
    d->rotLeftAction->setEnabled(false);
    d->rotRightAction->setEnabled(false);

#ifdef HAVE_KFACE
    d->faceGroup->setInfo(ImageInfo());
#endif /* HAVE_KFACE */
}

void ImagePreviewView::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
#ifdef HAVE_KFACE
    d->faceGroup->aboutToSetInfo(info);
#endif /* HAVE_KFACE */

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

#ifdef HAVE_KFACE
    return d->faceGroup->acceptsMouseClick(mapToScene(e->pos()));
#else
    return false;
#endif /* HAVE_KFACE */
}

void ImagePreviewView::enterEvent(QEvent* e)
{
#ifdef HAVE_KFACE
    d->faceGroup->enterEvent(e);
#else
    Q_UNUSED(e);
#endif /* HAVE_KFACE */
}

void ImagePreviewView::leaveEvent(QEvent* e)
{
#ifdef HAVE_KFACE
    d->faceGroup->leaveEvent(e);
#else
    Q_UNUSED(e);
#endif /* HAVE_KFACE */
}

void ImagePreviewView::showEvent(QShowEvent* e)
{
    GraphicsDImgView::showEvent(e);
#ifdef HAVE_KFACE
    d->faceGroup->setVisible(d->peopleToggleAction->isChecked());
#endif /* HAVE_KFACE */
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
    KUrl::List selectedItems;
    selectedItems << info.fileUrl();

    // --------------------------------------------------------

    KMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("full_screen");
    cmhelper.addAction("options_show_menubar");
    cmhelper.addSeparator();

    // --------------------------------------------------------

    if (d->mode == IconViewPreview)
    {
        cmhelper.addAction(d->prevAction, true);
        cmhelper.addAction(d->nextAction, true);
        cmhelper.addGotoMenu(idList);
        cmhelper.addSeparator();
    }

    // --------------------------------------------------------

#ifdef HAVE_KFACE
    cmhelper.addAction(d->peopleToggleAction, true);
    cmhelper.addAction(d->addPersonAction,    true);
    cmhelper.addAction(d->forgetFacesAction,  true);
    cmhelper.addSeparator();
#endif /* HAVE_KFACE */

    // --------------------------------------------------------

    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addAction("image_rotate");
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction("image_find_similar");

    if (d->mode == IconViewPreview)
    {
        cmhelper.addStandardActionLightTable();
    }

    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addStandardActionItemDelete(this, SLOT(slotDeleteItem()));
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    cmhelper.addSeparator();

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

    connect(&cmhelper, SIGNAL(signalGotoAlbum(ImageInfo)),
            this, SIGNAL(signalGotoAlbumAndItem(ImageInfo)));

    connect(&cmhelper, SIGNAL(signalGotoDate(ImageInfo)),
            this, SIGNAL(signalGotoDateAndItem(ImageInfo)));

    cmhelper.exec(event->screenPos());
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
    plt.setColor(backgroundRole(), kapp->palette().color(QPalette::Base));
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

#ifdef HAVE_KFACE
    /**
     * aboutToSetInfo will delete all face tags from FaceGroup storage
     */
    d->faceGroup->aboutToSetInfo(ImageInfo());
#endif /* HAVE_KFACE */

    FileActionMngr::instance()->transform(QList<ImageInfo>() << d->item->imageInfo(), KExiv2Iface::RotationMatrix::Rotate270);
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

#ifdef HAVE_KFACE
    /**
     * aboutToSetInfo will delete all face tags from FaceGroup storage
     */
    d->faceGroup->aboutToSetInfo(ImageInfo());
#endif /* HAVE_KFACE */

    FileActionMngr::instance()->transform(QList<ImageInfo>() << d->item->imageInfo(), KExiv2Iface::RotationMatrix::Rotate90);
}

void ImagePreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

void Digikam::ImagePreviewView::slotUpdateFaces()
{
#ifdef HAVE_KFACE
    d->faceGroup->aboutToSetInfo(ImageInfo());
#endif /* HAVE_KFACE */

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

        KMenu popMenu(this);
        QAction* const assignToThisAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &This Item"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));
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
#ifdef HAVE_KFACE
    if (e->button() == Qt::LeftButton && QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        d->faceGroup->addFace();
    }
#endif /* HAVE_KFACE */
    GraphicsDImgView::mousePressEvent(e);
}

}  // namespace Digikam
