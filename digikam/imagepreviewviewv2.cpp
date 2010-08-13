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
 * Copyright (C) 2010 Aditya Bhatt <adityabhatt1991 at gmail dot com>
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
#include "previewlayout.h"
#include "tagscache.h"
#include "imagetagpair.h"
#include "albummanager.h"
#include "faceiface.h"

// libkface includes

#include <libkface/kface.h>
#include <libkface/faceitem.h>

// KDE includes

#include "kglobalsettings.h"
#include <kstandarddirs.h>

using namespace KFaceIface;

namespace Digikam
{

class ImagePreviewViewItem : public DImgPreviewItem
{
public:

    ImagePreviewViewItem(ImagePreviewViewV2* view) : m_view(view)
    {
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

    ImageInfo imageInfo() const
    {
        return m_info;
    }

protected:

    ImagePreviewViewV2* m_view;
    ImageInfo           m_info;
};

class ImagePreviewViewV2Priv
{
public:

    ImagePreviewViewV2Priv()
    {
        peopleTagsShown      = false;
        fullSize             = 0;
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
        addPersonAction      = 0;

        faceIface            = new FaceIface;
    }

    bool                  peopleTagsShown;
    bool                  fullSize;
    double                scale;
    
    ImagePreviewViewItem* item;

    QAction*              back2AlbumAction;
    QAction*              prevAction;
    QAction*              nextAction;
    QAction*              rotLeftAction;
    QAction*              rotRightAction;
    QAction*              peopleToggleAction;
    QAction*              addPersonAction;
    QAction*              forgetFacesAction;
    
    QToolBar*             toolBar;

    AlbumWidgetStack*     stack;

    QList<FaceItem* >     faceitems;
    QList<Face>           currentFaces;

    FaceIface*            faceIface;
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
    
    connect(d->item, SIGNAL(loadedWithSize(bool)),
            this, SLOT(imageLoadedWithSize(bool)) );

    installPanIcon();

    // ------------------------------------------------------------

    d->stack = parent;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    clearFaceItems();

    // ------------------------------------------------------------

    d->back2AlbumAction   = new QAction(SmallIcon("folder-image"),        i18n("Back to Album"),                    this);
    d->prevAction         = new QAction(SmallIcon("go-previous"),         i18nc("go to previous image", "Back"),    this);
    d->nextAction         = new QAction(SmallIcon("go-next"),             i18nc("go to next image", "Forward"),     this);
    d->rotLeftAction      = new QAction(SmallIcon("object-rotate-left"),  i18nc("@info:tooltip", "Rotate Left"),    this);
    d->rotRightAction     = new QAction(SmallIcon("object-rotate-right"), i18nc("@info:tooltip", "Rotate Right"),   this);
    d->peopleToggleAction = new QAction(SmallIcon("user-identity"),       i18n("Show Face Tags"), this);
    d->addPersonAction    = new QAction(SmallIcon("list-add-user"),       i18n("Add a Face Tag"), this);
    d->forgetFacesAction  = new QAction(SmallIcon("list-remove-user"),    i18n("Forget all faces from this image"), this);
    
    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->back2AlbumAction);
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

    connect(d->peopleToggleAction, SIGNAL(triggered()),
            this, SLOT(slotTogglePeople()));

    connect(d->addPersonAction, SIGNAL(triggered()),
            this, SLOT(slotAddPersonTag()));
    
    connect(d->forgetFacesAction, SIGNAL(triggered()),
            this, SLOT(slotForgetFaces()));

    connect(this->layout(), SIGNAL( zoomFactorChanged(double)),
            this, SLOT(slotUpdatePersonTagScales()));

    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    /* I don't think that clicking on the view to get back to the album is a good idea anymore. When people play with
     * the face tags, it is extremely difficult to avoid clicking the background sometimes.
    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalBack2Album()));*/

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImagePreviewViewV2::~ImagePreviewViewV2()
{
    delete d;
}

void ImagePreviewViewV2::reload()
{
    previewItem()->reload();

    slotRefreshPeopleTags();
}

void ImagePreviewViewV2::imageLoaded()
{
    d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
    d->stack->previewLoaded();
    emit signalPreviewLoaded(true);
    d->rotLeftAction->setEnabled(true);
    d->rotRightAction->setEnabled(true);

    if(d->peopleTagsShown)
    {
        slotHidePeopleTags();
        if(hasBeenScanned())
            slotShowPeopleTags();
        d->peopleTagsShown = true;
    }
    else
    {
        slotHidePeopleTags();
    }
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

void ImagePreviewViewV2::showContextMenu(const ImageInfo& info, QGraphicsSceneContextMenuEvent* event)
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
    cmhelper.addAction(d->addPersonAction, true);
    cmhelper.addAction(d->forgetFacesAction, true);

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

    slotRefreshPeopleTags();
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

    slotRefreshPeopleTags();
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

    slotRefreshPeopleTags();
}

void ImagePreviewViewV2::slotTogglePeople()
{
    if(d->peopleTagsShown)
        slotHidePeopleTags();
    else
        slotShowPeopleTags();
}

void ImagePreviewViewV2::updateScale()
{
    int sceneWidth    = this->scene()->width();
    int sceneHeight   = this->scene()->height();
    int previewWidth;
    int previewHeight;
    
    if(d->fullSize)
    {
        previewWidth  = d->item->image().width();
        previewHeight = d->item->image().height();
    }
    else
    {
        DImg img(getImageInfo().filePath());
        previewWidth = img.width();
        previewHeight = img.height();
    }

    // FIXME: This way doesn't always work. Only works with Full Sized preview.
    if (1.*sceneWidth/previewWidth < 1.*sceneHeight/previewHeight)
    {
        d->scale = 1.*sceneWidth/previewWidth;
    }
    else
    {
        d->scale = 1.*sceneHeight/previewHeight;
    }
}

void ImagePreviewViewV2::slotUpdatePersonTagScales()
{
    updateScale();

    FaceItem* item = 0;
    foreach(item, d->faceitems)
    {
        item->setVisible(false);
        d->faceitems.removeAt(d->faceitems.indexOf(item));
        d->faceitems.append(new FaceItem(0, this->scene(), item->originalRect(), d->scale, item->text(), item->originalScale()));
        delete item;
    }
    
    makeFaceItemConnections();
}

void ImagePreviewViewV2::clearFaceItems()
{
    FaceItem* item = 0;

    foreach(item, d->faceitems)
        item->setVisible(false);

    d->faceitems.clear();
}

void ImagePreviewViewV2::drawFaceItems()
{
    Face face;
    
    for (int i = 0; i < d->currentFaces.size(); ++i)
    {
        face = d->currentFaces[i];
        d->faceitems.append(new FaceItem(0, this->scene(), face.toRect(), d->scale, face.name(), d->scale));
    }
    makeFaceItemConnections();
}

void ImagePreviewViewV2::findFaces()
{
    DImg dimg(d->item->image());
    DImg fullImg(getImageInfo().filePath());
    int i;
    
    if(hasBeenScanned())
    {
        kDebug()<<"Image already has been scanned.";
        d->currentFaces = d->faceIface->findFacesFromTags(dimg, d->item->imageInfo().id());
        
        for(i = 0; i < d->currentFaces.size(); ++i)
        {
            kDebug()<<d->currentFaces.at(i);
        }
        
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group("Face Tags Settings");
        if(group.readEntry("FaceSuggestion", false))
            this->trainFaces();
        return;
    }
    
    d->currentFaces = d->faceIface->findAndTagFaces(fullImg, d->item->imageInfo().id());
    
    kDebug() << "Found : " << d->currentFaces.size() << " faces.";
    
    for(i = 0; i < d->currentFaces.size(); ++i)
    {
        kDebug()<<d->currentFaces.at(i);
    }
}

void ImagePreviewViewV2::slotShowPeopleTags()
{
    updateScale();
    clearFaceItems();
    findFaces();
    drawFaceItems();
    
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group("Face Tags Settings");
        if(group.readEntry("FaceSuggestion", false))
            this->suggestFaces();
    
    d->peopleToggleAction->setText(i18n("Hide face tags"));
    d->addPersonAction->setVisible(true);
    d->peopleTagsShown = true;
}

void ImagePreviewViewV2::slotHidePeopleTags()
{
    updateScale();
    clearFaceItems();
    
    d->currentFaces.clear();
    d->peopleToggleAction->setText(i18n("Show face tags"));
    d->addPersonAction->setVisible(false);
    d->peopleTagsShown = false;
}

void ImagePreviewViewV2::slotRefreshPeopleTags()
{
    if(d->peopleTagsShown)
    {
        slotHidePeopleTags();
        slotShowPeopleTags();
    }
}


void ImagePreviewViewV2::slotAddPersonTag()
{
    int w = this->scene()->width()/2;
    int h = w;
    int x = this->scene()->width()/2 - w/2;
    int y = this->scene()->height()/2 - w/2;

    d->faceitems.append(new FaceItem(0, this->scene(), QRect(x, y, w, h), d->scale , "", d->scale));
    
    makeFaceItemConnections();
}

bool ImagePreviewViewV2::hasBeenScanned()
{
    return d->faceIface->hasBeenScanned(d->item->imageInfo().id());
}

void ImagePreviewViewV2::slotForgetFaces()
{
    clearFaceItems();
    d->currentFaces.clear();
    
    d->faceIface->forgetFaceTags(d->item->imageInfo().id());
}

void ImagePreviewViewV2::slotTagPerson ( const QString& name, const QRect& rect)
{
    for(int i = 0; i < d->currentFaces.size(); ++i)
    {
        if(d->currentFaces[i].toRect() == rect)
        {
            d->currentFaces[i].setName(name);
            break;
        }
    }
    
    d->faceIface->setName(getImageInfo().id(), rect, name);
}

void ImagePreviewViewV2::slotRemoveFaceTag ( const QString& name, const QRect& rect)
{
    for(int i = 0; i < d->currentFaces.size(); ++i)
    {
        if(d->currentFaces[i].toRect() == rect)
        {
            d->currentFaces.removeAt(i);
            d->faceitems.removeAt(i);
            break;
        }
    }
    
    d->faceIface->removeRect(getImageInfo().id(), rect, name);
}

void ImagePreviewViewV2::makeFaceItemConnections()
{
    for(int i = 0; i < d->faceitems.size(); ++i)
    {
        connect( d->faceitems[i], SIGNAL(acceptButtonClicked(QString, QRect)), 
                 this, SLOT(slotTagPerson(QString, QRect)) );
        
        connect( d->faceitems[i], SIGNAL(rejectButtonClicked(QString, QRect)),
                this, SLOT(slotRemoveFaceTag(QString, QRect)) );
    }
}

void ImagePreviewViewV2::trainFaces()
{
    QList<Face> trainList;
    foreach(Face f, d->currentFaces)
    {
        if(f.name() != "" && !d->faceIface->isFaceTrained(getImageInfo().id(), f.toRect(), f.name()))
            trainList += f;
    }
    
    kDebug()<<"Number of training faces"<<trainList.size();
    
    if(trainList.size()!=0)
    {
        d->faceIface->trainWithFaces(trainList);
        d->faceIface->markFacesAsTrained(getImageInfo().id(), trainList);
    }
}

void ImagePreviewViewV2::suggestFaces()
{
    // Assign tentative names to the face list
    QList<Face> recogList;
    foreach(Face f, d->currentFaces)
    {
        if(!d->faceIface->isFaceRecognized(getImageInfo().id(), f.toRect(), f.name()) && f.name() == "")
        {
            f.setName(d->faceIface->recognizedName(f));
            d->faceIface->markFaceAsRecognized(getImageInfo().id(), f.toRect(), f.name());
            
            // If the face wasn't recognized (too distant) don't suggest anything
            if(f.name() == "")
                continue;
            else
                recogList += f;
        }
    }
    
    kDebug()<<"Number of suggestions = "<<recogList.size();
    kDebug()<<"Number of faceitems = "<<d->faceitems.size();
    // Now find the relevant face items and suggest faces
    for(int i = 0; i < recogList.size(); ++i)
    {
        for(int j = 0; j < d->faceitems.size(); ++j)
        {
            if(recogList[i].toRect() == d->faceitems[j]->originalRect())
            {
                kDebug()<<"Suggesting a name "<<recogList[i].name();
                d->faceitems[j]->suggest(recogList[i].name());
                break;
            }
        }
    }
    
}

void ImagePreviewViewV2::imageLoadedWithSize(bool fullSize)
{
    d->fullSize = fullSize;
}


}  // namespace Digikam
