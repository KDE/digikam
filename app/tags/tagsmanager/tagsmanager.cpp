/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * Copyright (C) 2014 by Michael G. Hansen <mike at mghansen dot de>
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

#include "tagsmanager.h"

// Qt includes

#include <QQueue>
#include <QTreeView>
#include <QLabel>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QSplitter>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <QAction>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kmultitabbar.h>
#include <kactionmenu.h>
#include <kmessagebox.h>

// local includes

#include "digikam_debug.h"
#include "config-digikam.h"
#include "tagpropwidget.h"
#include "tagmngrtreeview.h"
#include "taglist.h"
#include "tagfolderview.h"
#include "ddragobjects.h"
#include "searchtextbar.h"
#include "tageditdlg.h"
#include "albumdb.h"
#include "dlogoaction.h"
#include "metadatasynchronizer.h"
#include "fileactionmngr.h"
#include "metadatasettings.h"

namespace Digikam
{

QPointer<TagsManager> TagsManager::internalPtr = QPointer<TagsManager>();

class TagsManager::Private
{

public:

    Private()
    {
        tagPixmap        = 0;
        searchBar        = 0;
        splitter         = 0;
        treeWindow       = 0;
        mainToolbar      = 0;
        rightToolBar     = 0;
        organizeAction   = 0;
        syncexportAction = 0;
        tagProperties    = 0;
        addAction        = 0;
        delAction        = 0;
        listView         = 0;
        tagPropWidget    = 0;
        tagMngrView      = 0;
        tagModel         = 0;
    }

    TagMngrTreeView* tagMngrView;
    QLabel*          tagPixmap;
    SearchTextBar*   searchBar;


    QSplitter*       splitter;
    KMainWindow*     treeWindow;
    KToolBar*        mainToolbar;
    KMultiTabBar*    rightToolBar;
    KActionMenu*     organizeAction;
    KActionMenu*     syncexportAction;
    QAction*         tagProperties;
    QAction*         addAction;
    QAction*         delAction;
    /** Options unavailable for root tag **/
    QList<QAction*>  rootDisabledOptions;

    TagList*         listView;
    TagPropWidget*   tagPropWidget;
    TagModel*        tagModel;

};

TagsManager::TagsManager()
    : KMainWindow(0), StateSavingObject(this), d(new Private())
{
    setObjectName("Tags Manager");
    d->tagModel = new TagModel(AbstractAlbumModel::IncludeRootAlbum, this);;
    d->tagModel->setCheckable(false);
    setupUi(this);

    /*----------------------------Connects---------------------------*/

    connect(d->tagMngrView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));

    connect(d->addAction, SIGNAL(triggered()),
            this, SLOT(slotAddAction()));

    connect(d->delAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteAction()));

    d->tagMngrView->setCurrentIndex(d->tagMngrView->model()->index(0,0));

    StateSavingObject::loadState();

    /** Set KMainWindow in center of the screen **/
    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());
}

TagsManager::~TagsManager()
{
    StateSavingObject::saveState();
    delete d->listView;
    delete d->tagMngrView;
    delete d->tagModel;
    delete d;
}

TagsManager* TagsManager::instance()
{
    if (TagsManager::internalPtr.isNull())
    {
        TagsManager::internalPtr = new TagsManager();
    }

    return TagsManager::internalPtr;
}

void TagsManager::setupUi(KMainWindow* const Dialog)
{
     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QHBoxLayout* const mainLayout = new QHBoxLayout();

     d->tagPixmap = new QLabel();
     d->tagPixmap->setText("Tag Pixmap");
     d->tagPixmap->setMaximumWidth(40);
     d->tagPixmap->setPixmap(QIcon::fromTheme("tag").pixmap(30,30));

     d->tagMngrView = new TagMngrTreeView(this,d->tagModel);
     d->tagMngrView->setConfigGroup(getConfigGroup());

     d->searchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");
     d->searchBar->setHighlightOnResult(true);
     d->searchBar->setModel(d->tagModel,
                            AbstractAlbumModel::AlbumIdRole,
                            AbstractAlbumModel::AlbumTitleRole);
     d->searchBar->setMaximumWidth(200);
     d->searchBar->setFilterModel(d->tagMngrView->albumFilterModel());

     /** Tree Widget & Actions + Tag Properties sidebar **/

     d->treeWindow    = new KMainWindow(this);
     setupActions();

     d->splitter = new QSplitter();

     d->listView      = new TagList(d->tagMngrView,Dialog);

     d->splitter->addWidget(d->listView);
     d->splitter->addWidget(d->tagMngrView);

     d->tagPropWidget = new TagPropWidget(d->treeWindow);
     d->splitter->addWidget(d->tagPropWidget);
     d->tagPropWidget->hide();

     d->splitter->setStretchFactor(0,0);
     d->splitter->setStretchFactor(1,1);
     d->splitter->setStretchFactor(2,0);
     d->treeWindow->setCentralWidget(d->splitter);

     mainLayout->addWidget(d->treeWindow);
     mainLayout->addWidget(d->rightToolBar);

     QWidget* const centraW = new QWidget(this);
     centraW->setLayout(mainLayout);
     this->setCentralWidget(centraW);
}

void TagsManager::slotOpenProperties()
{
    KMultiTabBarTab* const sender = (KMultiTabBarTab*)QObject::sender();

    if (sender->isChecked())
    {
        d->tagPropWidget->show();
    }
    else
    {
        d->tagPropWidget->hide();
    }
}

void TagsManager::slotSelectionChanged()
{
    QList<Album*> selectedTags = d->tagMngrView->selectedTags();

    if(selectedTags.isEmpty() || (selectedTags.size() == 1 && selectedTags.at(0)->isRoot()))
    {
        enableRootTagActions(false);
        d->listView->enableAddButton(false);
    }
    else
    {
        enableRootTagActions(true);
        d->listView->enableAddButton(true);
    }

    d->tagPropWidget->slotSelectionChanged(selectedTags);
}

void TagsManager::slotItemChanged()
{
}

void TagsManager::slotAddAction()
{
    TAlbum* const parent = d->tagMngrView->currentAlbum();
    QString       title, icon;
    QKeySequence  ks;

    if (!TagEditDlg::tagCreate(qApp->activeWindow(), parent, title, icon, ks))
    {
        return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, ks, errMap);
    TagEditDlg::showtagsListCreationError(qApp->activeWindow(), errMap);
}

namespace
{

QString JoinTagNamesToList(const QStringList& stringList)
{
    const QString joinedStringList = stringList.join(QString("', '"));
    return QChar('\'') + joinedStringList + QChar('\'');
}

} // namespace

void TagsManager::slotDeleteAction()
{
    const QModelIndexList selected = d->tagMngrView->selectionModel()->selectedIndexes();

    QStringList tagNames;
    QStringList tagsWithChildren;
    QStringList tagsWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    foreach(const QModelIndex& index, selected)
    {
        if (!index.isValid())
        {
            return;
        }

        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));
        if (!t || t->isRoot())
        {
            return;
        }

        AlbumPointer<TAlbum> tag(t);
        tagNames.append(tag->title());

        // find number of subtags
        int children = 0;
        AlbumIterator iter(tag);

        while (iter.current())
        {
            ++children;
            ++iter;
        }

        if (children)
        {
            tagsWithChildren.append(tag->title());
        }

        QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());
        if (!assignedItems.isEmpty())
        {
            tagsWithImages.append(tag->title());
        }

        /**
         * Tags must be deleted from children to parents, if we don't want
         * to step on invalid index. Use QMultiMap to order them by distance
         * to root tag
         */
        Album* parent = t;
        int depth = 0;

        while (!parent->isRoot())
        {
            parent = parent->parent();
            depth++;
        }

        sortedTags.insert(depth, tag);
    }

    // ask for deletion of children
    if (!tagsWithChildren.isEmpty())
    {
        const int result = KMessageBox::warningContinueCancel(
                this,
                i18ncp(
                        "%2 is a comma separated list of tags to be deleted.",
                        "Tag %2 has one or more subtags. "
                        "Deleting it will also delete "
                        "the subtags. "
                        "Do you want to continue?",
                        "Tags %2 have one or more subtags. "
                        "Deleting them will also delete "
                        "the subtags. "
                        "Do you want to continue?",
                        tagsWithChildren.count(),
                        JoinTagNamesToList(tagsWithChildren)
                    )
            );

        if (result != KMessageBox::Continue)
        {
            return;
        }
    }

    QString message;

    if (!tagsWithImages.isEmpty())
    {
        message = i18ncp(
                "%2 is a comma separated list of tags to be deleted.",
                "Tag %2 is assigned to one or more items. "
                "Do you want to delete it?",
                "Tags %2 are assigned to one or more items. "
                "Do you want to delete them?",
                tagsWithImages.count(),
                JoinTagNamesToList(tagsWithImages)
            );
    }
    else
    {
        message = i18ncp(
                "%2 is a comma separated list of tags to be deleted.",
                "Delete tag %2?",
                "Delete tags %2?",
                tagNames.count(),
                JoinTagNamesToList(tagNames)
            );
    }

    const int result = KMessageBox::warningContinueCancel(
            0,
            message,
            i18np("Delete tag", "Delete tags", tagNames.count()),
            KGuiItem(i18n("Delete"),
            "edit-delete")
        );

    if (result == KMessageBox::Continue)
    {
        QMultiMap<int, TAlbum*>::iterator it;

        /**
         * QMultimap doesn't provide reverse iterator, -1 is required
         * because end() points after the last element
         */
        for (it = sortedTags.end()-1; it != sortedTags.begin()-1; --it)
        {
            QString errMsg;

            if (!AlbumManager::instance()->deleteTAlbum(it.value(), errMsg))
            {
                KMessageBox::error(0, errMsg);
            }
        }
    }
}

void TagsManager::slotResetTagIcon()
{
    QString errMsg;

    const QList<Album*> selected = d->tagMngrView->selectedTags();
    const QString icon = QString("tag");

    for (QList<Album*>::const_iterator it = selected.constBegin(); it != selected.constEnd(); ++it )
    {
        TAlbum* const tag = dynamic_cast<TAlbum*>(*it);

        if (tag)
        {
            if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
            {
                KMessageBox::error(0, errMsg);
            }
        }
    }
}

void TagsManager::slotCreateTagAddr()
{
}

void TagsManager::slotInvertSel()
{
    QModelIndex root                 = d->tagMngrView->model()->index(0,0);
    QItemSelectionModel* const model = d->tagMngrView->selectionModel();
    QModelIndexList selected         = model->selectedIndexes();

    QQueue<QModelIndex> greyNodes;
    bool currentSet = false;

    greyNodes.append(root);

    model->clearSelection();

    while(!greyNodes.isEmpty())
    {
        QModelIndex current = greyNodes.dequeue();

        if(!(current.isValid()))
        {
            continue;
        }

        int it            = 0;
        QModelIndex child = current.child(it++,0);

        while(child.isValid())
        {
            if(!selected.contains(child))
            {
                if(!currentSet)
                {
                    /**
                     * Must set a new current item when inverting selection
                     * it should be done only once
                     */
                    d->tagMngrView->setCurrentIndex(child);
                    currentSet = true;
                }

                model->select(child, model->Select);
            }

            if(d->tagMngrView->isExpanded(child))
            {
                greyNodes.enqueue(child);
            }

            child = current.child(it++,0);
        }
    }
}

void TagsManager::slotWriteToImg()
{
    int result = KMessageBox::warningContinueCancel(
            this,
            i18n(
                    "digiKam will clean up tag metadata before setting "
                    "tags from database.<br> You may <b>lose tags</b> if you did not "
                    "read tags before (by calling Read Tags from Image).<br> "
                    "Do you want to continue?"
                )
        );

    if (result != KMessageBox::Continue)
    {
        return;
    }

    result = KMessageBox::warningContinueCancel(
            this,
            i18n(
                    "This operation can take long time "
                    "depending on collection size.\n"
                    "Do you want to continue?"
                )
        );

    if (result != KMessageBox::Continue)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList(),
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotReadFromImg()
{
    int result = KMessageBox::warningContinueCancel(
            this,
            i18n(
                    "This operation can take long time "
                    "depending on collection size.\n"
                    "Do you want to continue?"
                )
        );

    if (result != KMessageBox::Continue)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList(),
                                                                MetadataSynchronizer::ReadFromFileToDatabase);

    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotWipeAll()
{
    const int result = KMessageBox::warningContinueCancel(
            this,
            i18n(
                    "This operation will wipe all tags from database only.\n"
                    "To apply changes to files, "
                    "you must choose write metadata to file later.\n"
                    "Do you want to continue?"
                )
        );

    if (result != KMessageBox::Continue)
    {
        return;
    }

    /** Disable writing tags to images **/
    MetadataSettings* metaSettings = MetadataSettings::instance();
    MetadataSettingsContainer backUpContainer = metaSettings->settings();
    MetadataSettingsContainer newContainer = backUpContainer;
    bool settingsChanged = false;

    if (backUpContainer.saveTags == true || backUpContainer.saveFaceTags == true)
    {
        settingsChanged = true;
        newContainer.saveTags = false;
        newContainer.saveFaceTags = false;
        metaSettings->setSettings(newContainer);
    }

    AlbumPointerList<TAlbum> tagList;
    const QModelIndex root  = d->tagMngrView->model()->index(0,0);
    int iter          = 0;
    QModelIndex child = root.child(iter++, 0);

    while (child.isValid())
    {
        tagList <<  AlbumPointer<TAlbum>(d->tagMngrView->albumForIndex(child));
        child = root.child(iter++, 0);
    }

    AlbumPointerList<TAlbum>::iterator it;

    for (it = tagList.begin(); it != tagList.end(); ++it)
    {
        QString errMsg;

        if (!AlbumManager::instance()->deleteTAlbum(*it, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    /** Restore settings after tag deletion **/
    if (settingsChanged)
    {
        metaSettings->setSettings(backUpContainer);
    }
}

void TagsManager::slotRemoveTagsFromImgs()
{
    const QModelIndexList selList = d->tagMngrView->selectionModel()->selectedIndexes();

    const int result = KMessageBox::warningContinueCancel(
            this,
            i18np(
                    "Do you really want to remove the selected tag from all images?",
                    "Do you really want to remove the selected tags from all images?",
                    selList.count()
                )
        );

    if (result != KMessageBox::Continue)
    {
        return;
    }

    foreach (const QModelIndex& index, selList)
    {
        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

        AlbumPointer<TAlbum> tag(t);

        if (tag->isRoot())
        {
            continue;
        }

        QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());
        ImageInfoList imgList(assignedItems);
        FileActionMngr::instance()->removeTag(imgList, tag->id());
    }
}

void TagsManager::closeEvent(QCloseEvent* event)
{
    d->listView->saveSettings();
    KMainWindow::closeEvent(event);
}

void TagsManager::setupActions()
{
    d->mainToolbar = new KToolBar(d->treeWindow, true);
    d->mainToolbar->layout()->setContentsMargins(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin),
                                                 QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin),
                                                 QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin),
                                                 QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));

    QWidgetAction* const pixMapAction = new QWidgetAction(this);
    pixMapAction->setDefaultWidget(d->tagPixmap);

    QWidgetAction* const searchAction = new QWidgetAction(this);
    searchAction->setDefaultWidget(d->searchBar);

    d->mainToolbar->addAction(pixMapAction);
    d->mainToolbar->addAction(searchAction);

    d->mainToolbar->addSeparator();

    d->addAction = new QAction(QIcon::fromTheme("list-add"),"",d->treeWindow);

    d->delAction = new QAction(QIcon::fromTheme("list-remove"),"",d->treeWindow);

    /** organize group **/
    d->organizeAction      = new KActionMenu(QIcon::fromTheme("autocorrection"),
                                             i18nc("@title:menu", "Organize"),this);
    d->organizeAction->setDelayed(false);

    QAction* const resetIcon     = new QAction(QIcon::fromTheme("view-refresh"),
                                         i18n("Reset tag Icon"), this);

    QAction* const createTagAddr = new QAction(QIcon::fromTheme("tag-addressbook"),
                                         i18n("Create Tag from Address Book"),
                                         this);
    QAction* const invSel        = new QAction(QIcon::fromTheme("tag-reset"),
                                         i18n("Invert Selection"), this);

    QAction* const expandTree    = new QAction(QIcon::fromTheme("format-indent-more"),
                                         i18n("Expand Tag Tree"), this);

    QAction* const expandSel     = new QAction(QIcon::fromTheme("format-indent-more"),
                                         i18n("Expand Selected Nodes"), this);
    QAction* const delTagFromImg = new QAction(QIcon::fromTheme("tag-delete"),
                                         i18n("Remove Tag from Images"), this);

    /** Tool tips  **/
    setHelpText(d->addAction, i18n("Add new tag to current tag. "
                                   "Current tag is last clicked tag."));

    setHelpText(d->delAction, i18n("Delete selected items. "
                                  "Also work with multiple items, "
                                  "but won't delete the root tag."));

    setHelpText(resetIcon, i18n("Reset icon to selected tags. "
                               "Works with multiple selection." ));

    setHelpText(invSel, i18n("Invert selection. "
                            "Only visible items will be selected"));

    setHelpText(expandTree, i18n("Expand tag tree by one level"));

    setHelpText(expandSel, i18n("Selected items will be expanded"));

    setHelpText(delTagFromImg, i18n("Delete selected tag(s) from images. "
                                    "Works with multiple selection."));

    connect(resetIcon, SIGNAL(triggered()),
            this, SLOT(slotResetTagIcon()));

    connect(createTagAddr, SIGNAL(triggered()),
            this, SLOT(slotCreateTagAddr()));

    connect(invSel, SIGNAL(triggered()),
            this, SLOT(slotInvertSel()));

    connect(expandTree, SIGNAL(triggered()),
            d->tagMngrView, SLOT(slotExpandTree()));

    connect(expandSel, SIGNAL(triggered()),
            d->tagMngrView, SLOT(slotExpandSelected()));

    connect(delTagFromImg, SIGNAL(triggered()),
            this, SLOT(slotRemoveTagsFromImgs()));

    d->organizeAction->addAction(resetIcon);
    d->organizeAction->addAction(createTagAddr);
    d->organizeAction->addAction(invSel);
    d->organizeAction->addAction(expandTree);
    d->organizeAction->addAction(expandSel);
    d->organizeAction->addAction(delTagFromImg);

    /** Sync & Export Group **/
    d->syncexportAction = new KActionMenu(QIcon::fromTheme("server-database"),
                                          i18n("Sync &Export"),this);
    d->syncexportAction->setDelayed(false);

    QAction* const wrDbImg  = new QAction(QIcon::fromTheme("view-refresh"),
                                          i18n("Write Tags from Database "
                                              "to Image"), this);

    QAction* const readTags = new QAction(QIcon::fromTheme("tag-new"),
                                          i18n("Read Tags from Image"), this);

    QAction* const wipeAll  = new QAction(QIcon::fromTheme("draw-eraser"),
                                          i18n("Wipe all tags from Database only"), this);


    setHelpText(wrDbImg, i18n("Write Tags Metadata to Image."));

    setHelpText(readTags, i18n("Read tags from Images into Database. "
                              "Existing tags won't be affected"));

    setHelpText(wipeAll, i18n("Delete all tags from database only. Will not sync with files. "
                             "Proceed with caution."));


    connect(wrDbImg, SIGNAL(triggered()),
            this, SLOT(slotWriteToImg()));

    connect(readTags, SIGNAL(triggered()),
            this, SLOT(slotReadFromImg()));

    connect(wipeAll, SIGNAL(triggered()),
            this, SLOT(slotWipeAll()));

    d->syncexportAction->addAction(wrDbImg);
    d->syncexportAction->addAction(readTags);
    d->syncexportAction->addAction(wipeAll);

    d->mainToolbar->addAction(d->addAction);
    d->mainToolbar->addAction(d->delAction);
    d->mainToolbar->addAction(d->organizeAction);
    d->mainToolbar->addAction(d->syncexportAction);
    d->mainToolbar->addAction(new DLogoAction(this));
    this->addToolBar(d->mainToolbar);

    /**
     * Right Toolbar with vertical properties button
     */
    d->rightToolBar = new KMultiTabBar(KMultiTabBar::Right);
    d->rightToolBar->appendTab(QIcon::fromTheme("tag-properties").pixmap(10,10),
                               0,i18n("Tag Properties"));
    d->rightToolBar->setStyle(KMultiTabBar::KDEV3ICON);

    connect(d->rightToolBar->tab(0),SIGNAL(clicked()),
            this, SLOT(slotOpenProperties()));

    d->rootDisabledOptions.append(d->delAction);
    d->rootDisabledOptions.append(resetIcon);
    d->rootDisabledOptions.append(delTagFromImg);
}

// helper based on KAction::setHelpText
void TagsManager::setHelpText(QAction *action, const QString &text)
{
    action->setStatusTip(text);
    action->setToolTip(text);
    if(action->whatsThis().isEmpty()) {
        action->setWhatsThis(text);
    }
}


void TagsManager::enableRootTagActions(bool value)
{
    Q_FOREACH(QAction* const action, d->rootDisabledOptions)
    {
        if(value)
            action->setEnabled(true);
        else
            action->setEnabled(false);
    }
}

void TagsManager::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    d->tagMngrView->doLoadState();
    group.sync();
}

void TagsManager::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    d->tagMngrView->doSaveState();
    group.sync();
}

} // namespace Digikam
