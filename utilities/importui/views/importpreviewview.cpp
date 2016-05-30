/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-14-07
 * Description : An embedded view to show the cam item preview widget.
 *
 * Copyright (C) 2012 by Islam Wazery  <wazery at ubuntu dot com>
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

#include "importpreviewview.h"

// Qt includes

#include <QMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QToolBar>
#include <QMenu>
#include <QApplication>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimgpreviewitem.h"
#include "fileactionmngr.h"
#include "importcontextmenu.h"
#include "previewlayout.h"
#include "thememanager.h"
#include "importsettings.h"
#include "previewsettings.h"

namespace Digikam
{

class ImportPreviewViewItem : public DImgPreviewItem
{
public:

    explicit ImportPreviewViewItem(ImportPreviewView* const view)
        : m_view(view)/*, m_group(0)*/
    {
        setAcceptHoverEvents(true);
    }

/*
    void setFaceGroup(FaceGroup* group)
    {
       m_group = group;
    }
*/

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
    {
        m_view->showContextMenu(m_info, event);
    }

    void setCamItemInfo(const CamItemInfo& info)
    {
        m_info = info;

        if(!info.isNull())
        {
            setPath(info.url().toLocalFile(), true);
        }
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e)
    {
        Q_UNUSED(e) //FIXME
        //m_group->itemHoverEnterEvent(e);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
    {
        Q_UNUSED(e) //FIXME:
        //m_group->itemHoverLeaveEvent(e);
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent* e)
    {
        Q_UNUSED(e) //FIXME:
        //m_group->itemHoverMoveEvent(e);
    }

    CamItemInfo camItemInfo() const
    {
        return m_info;
    }

protected:

    ImportPreviewView*  m_view;
    //FaceGroup*        m_group;
    CamItemInfo         m_info;
};

// ---------------------------------------------------------------------

class ImportPreviewView::Private
{
public:

    Private()
    {
        //peopleTagsShown    = false;
        fullSize             = 0;
        scale                = 1.0;
        item                 = 0;
        isValid              = false;
        toolBar              = 0;
        escapePreviewAction  = 0;
        prevAction           = 0;
        nextAction           = 0;
        rotLeftAction        = 0;
        rotRightAction       = 0;
        //peopleToggleAction = 0;
        //addPersonAction    = 0;
        //faceGroup          = 0;
        mode                 = ImportPreviewView::IconViewPreview;
    }

    //bool                  peopleTagsShown;
    bool                    fullSize;
    double                  scale;
    bool                    isValid;

    ImportPreviewView::Mode mode;

    ImportPreviewViewItem*  item;

    QAction*                escapePreviewAction;
    QAction*                prevAction;
    QAction*                nextAction;
    QAction*                rotLeftAction;
    QAction*                rotRightAction;
    //QAction*              peopleToggleAction;
    //QAction*              addPersonAction;
    //QAction*              forgetFacesAction;

    QToolBar*               toolBar;

    //FaceGroup*            faceGroup;
};

ImportPreviewView::ImportPreviewView(QWidget* const parent, Mode mode)
    : GraphicsDImgView(parent), d(new Private)
{
    d->mode = mode;
    d->item = new ImportPreviewViewItem(this);
    setItem(d->item);

    //d->faceGroup = new FaceGroup(this);
    //d->faceGroup->setShowOnHover(true);

    //d->item->setFaceGroup(d->faceGroup);

    connect(d->item, SIGNAL(loaded()),
            this, SLOT(camItemLoaded()));

    connect(d->item, SIGNAL(loadingFailed()),
            this, SLOT(camItemLoadingFailed()));

    // set default zoom
    layout()->fitToWindow();

    // ------------------------------------------------------------

    installPanIcon();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    d->escapePreviewAction = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")),          i18n("Escape preview"),                 this);
    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),              i18nc("go to previous image", "Back"),  this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),                  i18nc("go to next image", "Forward"),   this);
    d->rotLeftAction       = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-left")),       i18nc("@info:tooltip", "Rotate Left"),  this);
    d->rotRightAction      = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-right")),      i18nc("@info:tooltip", "Rotate Right"), this);
    //FIXME: d->addPersonAction    = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),    i18n("Add a Face Tag"),                 this);
    //FIXME: d->forgetFacesAction  = new QAction(QIcon::fromTheme(QLatin1String("list-remove-user")), i18n("Clear all faces on this image"),  this);
    //FIXME: d->peopleToggleAction = new Qaction(QIcon::fromTheme(QLatin1String("im-user")),          i18n("Show Face Tags"),                 this);
    //FIXME: d->peopleToggleAction->setCheckable(true);

    d->toolBar = new QToolBar(this);

    if (mode == IconViewPreview)
    {
        d->toolBar->addAction(d->prevAction);
        d->toolBar->addAction(d->nextAction);
        d->toolBar->addAction(d->escapePreviewAction);
    }

    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);
    //FIXME: d->toolBar->addAction(d->peopleToggleAction);
    //FIXME: d->toolBar->addAction(d->addPersonAction);

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(toPreviousImage()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(toNextImage()));

    connect(d->escapePreviewAction, SIGNAL(triggered()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->rotLeftAction, SIGNAL(triggered()),
            this, SLOT(slotRotateLeft()));

    connect(d->rotRightAction, SIGNAL(triggered()),
            this, SLOT(slotRotateRight()));

    //FIXME: connect(d->peopleToggleAction, SIGNAL(toggled(bool)),
            //d->faceGroup, SLOT(setVisible(bool)));

    //FIXME: connect(d->addPersonAction, SIGNAL(triggered()),
            //d->faceGroup, SLOT(addFace()));

    //FIXME: connect(d->forgetFacesAction, SIGNAL(triggered()),
            //d->faceGroup, SLOT(rejectAll()));

    // ------------------------------------------------------------

    connect(this, SIGNAL(toNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(toPreviousImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(activated()),
            this, SIGNAL(signalEscapePreview()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(ImportSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImportPreviewView::~ImportPreviewView()
{
    delete d->item;
    delete d;
}

void ImportPreviewView::reload()
{
    previewItem()->reload();
}

void ImportPreviewView::camItemLoaded()
{
    emit signalPreviewLoaded(true);
    d->rotLeftAction->setEnabled(true);
    d->rotRightAction->setEnabled(true);

    //FIXME: d->faceGroup->setInfo(d->item->camItemInfo());
}

void ImportPreviewView::camItemLoadingFailed()
{
    emit signalPreviewLoaded(false);
    d->rotLeftAction->setEnabled(false);
    d->rotRightAction->setEnabled(false);
    //FIXME: d->faceGroup->setInfo(CamItemInfo());
}

void ImportPreviewView::setCamItemInfo(const CamItemInfo& info, const CamItemInfo& previous, const CamItemInfo& next)
{
    //FIXME: d->faceGroup->aboutToSetInfo(info);
    d->item->setCamItemInfo(info);

    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    QStringList previewPaths;

    if (identifyCategoryforMime(next.mime) == QLatin1String("image"))
    {
        previewPaths << next.url().toLocalFile();
    }

    if (identifyCategoryforMime(previous.mime) == QLatin1String("image"))
    {
        previewPaths << previous.url().toLocalFile();
    }

    d->item->setPreloadPaths(previewPaths);
}

QString ImportPreviewView::identifyCategoryforMime(QString mime)
{
    return mime.split(QLatin1Char('/')).at(0);
}

CamItemInfo ImportPreviewView::getCamItemInfo() const
{
    return d->item->camItemInfo();
}

bool ImportPreviewView::acceptsMouseClick(QMouseEvent* e)
{
    if (!GraphicsDImgView::acceptsMouseClick(e))
    {
        return false;
    }
    return true;

    //FIXME: return d->faceGroup->acceptsMouseClick(mapToScene(e->pos()));
}

void ImportPreviewView::enterEvent(QEvent* e)
{
    Q_UNUSED(e) //FIXME
    //FIXME: d->faceGroup->enterEvent(e);
}

void ImportPreviewView::leaveEvent(QEvent* e)
{
    Q_UNUSED(e) //FIXME
    //FIXME: d->faceGroup->leaveEvent(e);
}

void ImportPreviewView::showEvent(QShowEvent* e)
{
    GraphicsDImgView::showEvent(e);
    //FIXME: d->faceGroup->setVisible(d->peopleToggleAction->isChecked());
}

void ImportPreviewView::showContextMenu(const CamItemInfo& info, QGraphicsSceneContextMenuEvent* event)
{
    if (info.isNull())
    {
        return;
    }

    event->accept();

    QList<qlonglong> idList;
    idList << info.id;
    QList<QUrl> selectedItems;
    selectedItems << info.url();

    // --------------------------------------------------------

    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();

    // --------------------------------------------------------

    if (d->mode == IconViewPreview)
    {
        cmhelper.addAction(d->prevAction, true);
        cmhelper.addAction(d->nextAction, true);
        cmhelper.addAction(QLatin1String("importui_icon_view"));
        //cmhelper.addGotoMenu(idList);
        cmhelper.addSeparator();
    }

    // --------------------------------------------------------

    //FIXME: cmhelper.addAction(d->peopleToggleAction, true);
    //FIXME: cmhelper.addAction(d->addPersonAction, true);
    //FIXME: cmhelper.addAction(d->forgetFacesAction, true);
    //FIXME: cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addRotateMenu(idList);
    cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addAction(QLatin1String("importui_delete"));
    cmhelper.addSeparator();

    // --------------------------------------------------------

    //FIXME: cmhelper.addAssignTagsMenu(idList);
    //FIXME: cmhelper.addRemoveTagsMenu(idList);
    //FIXME: cmhelper.addSeparator();

    // --------------------------------------------------------

    cmhelper.addLabelsAction();

    // special action handling --------------------------------

    //FIXME: connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            //this, SLOT(slotAssignTag(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            //this, SIGNAL(signalPopupTagsView()));

    //FIXME: connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            //this, SLOT(slotRemoveTag(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            //this, SLOT(slotAssignPickLabel(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            //this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SIGNAL(signalAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SIGNAL(signalAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SIGNAL(signalAssignRating(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            //this, SIGNAL(signalAddToExistingQueue(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            //this, SIGNAL(signalGotoTagAndItem(int)));

    cmhelper.exec(event->screenPos());
}

/*
void ImportPreviewView::slotAssignTag(int tagID)
{
   FileActionMngr::instance()->assignTag(d->item->camItemInfo(), tagID);
}

void ImportPreviewView::slotRemoveTag(int tagID)
{
   FileActionMngr::instance()->removeTag(d->item->camItemInfo(), tagID);
}
*/

void ImportPreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), qApp->palette().color(QPalette::Base));
    setPalette(plt);
}

void ImportPreviewView::slotSetupChanged()
{
    PreviewSettings settings;
    settings.quality = ImportSettings::instance()->getPreviewLoadFullImageSize() ? PreviewSettings::HighQualityPreview
                                                                                 : PreviewSettings::FastPreview;
    previewItem()->setPreviewSettings(settings);

    d->toolBar->setVisible(ImportSettings::instance()->getPreviewShowIcons());
    setShowText(ImportSettings::instance()->getPreviewShowIcons());

    // pass auto-suggest?
}

void ImportPreviewView::slotRotateLeft()
{
/*
    ImageInfo info(d->item->camItemInfo().url().toLocalFile());

    FileActionMngr::instance()->transform(QList<ImageInfo>() << info, MetaEngineRotation::Rotate270);
*/
}

void ImportPreviewView::slotRotateRight()
{
/*
    ImageInfo info(d->item->camItemInfo().url().toLocalFile());

    FileActionMngr::instance()->transform(QList<ImageInfo>() << info, MetaEngineRotation::Rotate90);
*/
}

void ImportPreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

} // namespace Digikam
