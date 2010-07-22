/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2010 Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagepreviewviewv2.moc"

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

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "contextmenuhelper.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dimgpreviewitem.h"
#include "dpopupmenu.h"
#include "imageinfo.h"
#include "metadatamanager.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"
#include "themeengine.h"

// libkface includes

#include <libkface/kface.h>
#include <libkface/database.h>
#include <libkface/faceitem.h>

// KDE includes

#include "kglobalsettings.h"

using namespace KFaceIface;

namespace Digikam
{

class ImagePreviewViewItem : public DImgPreviewItem
{
public:

    ImagePreviewViewItem(ImagePreviewViewV2 *view) : m_view(view)
    {
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        m_view->showContextMenu(m_info, event);
    }

    void setImageInfo(const ImageInfo& info)
    {
        m_info = info;
        setPath(info.filePath());
    }
    
    ImageInfo imageInfo() const
    {
        return m_info;
    }

protected:

    ImagePreviewViewV2 *m_view;
    ImageInfo m_info;
};

class ImagePreviewViewV2Priv
{
public:

    ImagePreviewViewV2Priv()
    {
        peopleTagsShown      = false;
        scale                = 1.0;
        
        item                 = 0;
        stack                = 0;
        toolBar              = 0;
        back2AlbumAction     = 0;
        prevAction           = 0;
        nextAction           = 0;
        rotLeftAction        = 0;
        rotRightAction       = 0;
        peopleToggleAction   = 0;
        
        faceIface            = new KFaceIface::Database(KFaceIface::Database::InitAll, "/home/aditya");
    }

    bool                  peopleTagsShown;
    double                scale;

    ImagePreviewViewItem* item;

    QAction*              back2AlbumAction;
    QAction*              prevAction;
    QAction*              nextAction;
    QAction*              rotLeftAction;
    QAction*              rotRightAction;
    QAction*              peopleToggleAction;
    
    QToolBar*             toolBar;

    AlbumWidgetStack*     stack;
    
    QList<FaceItem* >     faceitems;
    QList<Face>           currentFaces;

    KFaceIface::Database* faceIface;
};

ImagePreviewViewV2::ImagePreviewViewV2(AlbumWidgetStack* parent)
                : GraphicsDImgView(parent), d(new ImagePreviewViewV2Priv)
{
    d->item = new ImagePreviewViewItem(this);
    setItem(d->item);

    connect(d->item, SIGNAL(loaded()),
            this, SLOT(imageLoaded()));

    connect(d->item, SIGNAL(loadingFailed()),
            this, SLOT(imageLoadingFailed()));

    installPanIcon();

    // ------------------------------------------------------------

    d->stack            = parent;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    clearFaceItems();
    
    // ------------------------------------------------------------

    d->back2AlbumAction   = new QAction(SmallIcon("folder-image"),        i18n("Back to Album"),                    this);
    d->prevAction         = new QAction(SmallIcon("go-previous"),         i18nc("go to previous image", "Back"),    this);
    d->nextAction         = new QAction(SmallIcon("go-next"),             i18nc("go to next image", "Forward"),     this);
    d->rotLeftAction      = new QAction(SmallIcon("object-rotate-left"),  i18nc("@info:tooltip", "Rotate Left"),    this);
    d->rotRightAction     = new QAction(SmallIcon("object-rotate-right"), i18nc("@info:tooltip", "Rotate Right"),   this);
    d->peopleToggleAction = new QAction(SmallIcon("user-identity"),       i18n("Show Face Tags"), this);
    
    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->back2AlbumAction);
    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);
    d->toolBar->addAction(d->peopleToggleAction);

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
    
    connect(d->peopleToggleAction, SIGNAL(triggered()),
            this, SLOT(slotTogglePeople()));
    
    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalBack2Album()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

ImagePreviewViewV2::~ImagePreviewViewV2()
{
    delete d;
}

void ImagePreviewViewV2::reload()
{
    previewItem()->reload();
    
    updateScale();
}

void ImagePreviewViewV2::imageLoaded()
{
    d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
    d->stack->previewLoaded();
    emit signalPreviewLoaded(true);
    d->rotLeftAction->setEnabled(true);
    d->rotRightAction->setEnabled(true);
    
    updateScale();
}

void ImagePreviewViewV2::imageLoadingFailed()
{
    d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
    d->stack->previewLoaded();
    emit signalPreviewLoaded(false);
    d->rotLeftAction->setEnabled(false);
    d->rotRightAction->setEnabled(false);
}

void ImagePreviewViewV2::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    d->item->setImageInfo(info);

    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    d->item->setPreloadPaths(QStringList() << next.filePath() << previous.filePath());
}

ImageInfo ImagePreviewViewV2::getImageInfo() const
{
    return d->item->imageInfo();
}

void ImagePreviewViewV2::showContextMenu(const ImageInfo& info, QGraphicsSceneContextMenuEvent *event)
{
    if (info.isNull())
        return;

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
    // --------------------------------------------------------
    cmhelper.addAction(d->prevAction, true);
    cmhelper.addAction(d->nextAction, true);
    cmhelper.addAction(d->back2AlbumAction);
    cmhelper.addGotoMenu(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addKipiActions(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionItemDelete(this, SIGNAL(signalDeleteItem()));
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

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

void ImagePreviewViewV2::slotAssignTag(int tagID)
{
    MetadataManager::instance()->assignTag(d->item->imageInfo(), tagID);
}

void ImagePreviewViewV2::slotRemoveTag(int tagID)
{
    MetadataManager::instance()->removeTag(d->item->imageInfo(), tagID);
}

void ImagePreviewViewV2::slotAssignRating(int rating)
{
    MetadataManager::instance()->assignRating(d->item->imageInfo(), rating);
}

void ImagePreviewViewV2::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(plt);
}

void ImagePreviewViewV2::slotSetupChanged()
{
    previewItem()->setLoadFullImageSize(AlbumSettings::instance()->getPreviewLoadFullImageSize());
    previewItem()->setExifRotate(AlbumSettings::instance()->getExifRotate());
}

void ImagePreviewViewV2::slotRotateLeft()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_ccw"))
                ac->trigger();
        }
    }
}

void ImagePreviewViewV2::slotRotateRight()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_cw"))
                ac->trigger();
        }
    }
}

void ImagePreviewViewV2::slotTogglePeople()
{
    // Already shown, need to clean up
    if(d->peopleTagsShown)
    {
        d->peopleToggleAction->setText(i18n("Show face tags"));
        d->peopleTagsShown = false;
        
        clearFaceItems();
    }
    
    // Not shown, need to get faces and draw items
    else
    {
        d->peopleToggleAction->setText(i18n("Hide face tags"));
        d->peopleTagsShown = true;
        
        clearFaceItems();
        findFaces();
        drawFaceItems();
    }
    
}

void ImagePreviewViewV2::updateScale()
{
    int sceneWidth    = this->scene()->width();
    int sceneHeight   = this->scene()->height();
    int previewWidth  = d->item->image().width();
    int previewHeight = d->item->image().height();

    if (1.*sceneWidth/previewWidth < 1.*sceneHeight/previewHeight)
    {
        d->scale = 1.*sceneWidth/previewWidth;
    }
    else
    {
        d->scale = 1.*sceneHeight/previewHeight;
    }
}

void ImagePreviewViewV2::clearFaceItems()
{
    FaceItem* item=0;

    foreach(item, d->faceitems)
        item->setVisible(false);

    d->faceitems.clear();
    kDebug() << "Found : " << d->currentFaces.size() << " faces.";
}

void ImagePreviewViewV2::drawFaceItems()
{
    Face face;
    for (int i = 0; i < d->currentFaces.size(); ++i)
    {
        face = d->currentFaces[i];
        d->faceitems.append(new FaceItem(0, this->scene(), face.toRect(), d->scale));
        kDebug() << face.toRect()<<endl;
    }
}

void ImagePreviewViewV2::findFaces()
{
    d->currentFaces = d->faceIface->detectFaces(KFaceIface::Image(getImageInfo().filePath()));
}

}  // namespace Digikam