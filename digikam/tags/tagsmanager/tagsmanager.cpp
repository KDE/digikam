/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Manager main class
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

/** Qt includes **/
#include <QtAlgorithms>
#include <QQueue>
#include <QTreeView>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>

/** KDE includes **/
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kmultitabbar.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kmessagebox.h>

/** local includes **/
#include "tagsmanager.h"
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

namespace Digikam
{

QPointer<TagsManager> TagsManager::internalPtr = QPointer<TagsManager>();

class TagsManager::PrivateTagMngr
{

public:

    PrivateTagMngr()
    {
        tagPixmap       = 0;
        searchBar       = 0;
        treeWinLayout   = 0;
        treeWindow      = 0;
        mainToolbar     = 0;
        rightToolBar    = 0;
        organizeAction  = 0;
        syncexportAction = 0;
        tagProperties   = 0;
        addAction       = 0;
        delAction       = 0;
        listView        = 0;
        tagPropWidget   = 0;
        tagMngrView     = 0;
    }

    TagMngrTreeView*  tagMngrView;
    QLabel*         tagPixmap;
    SearchTextBar*  searchBar;


    QHBoxLayout*    treeWinLayout;
    KMainWindow*    treeWindow;
    KToolBar*       mainToolbar;
    KMultiTabBar*   rightToolBar;
    KActionMenu*    organizeAction;
    KActionMenu*    syncexportAction;
    KAction*        tagProperties;
    KAction*        addAction;
    KAction*        delAction;

    TagList*        listView;
    TagPropWidget*  tagPropWidget;
    TagModel*       tagModel;
};

TagsManager::TagsManager()
    : KMainWindow(0), d(new PrivateTagMngr())
{

    /** No buttons **/
    d->tagModel = new TagModel(AbstractAlbumModel::IncludeRootAlbum, this);;
    d->tagModel->setCheckable(false);

    setupUi(this);

    /*----------------------------Connects---------------------------*/

    connect(d->tagMngrView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));

    connect(d->addAction, SIGNAL(triggered()), this, SLOT(slotAddAction()));

    connect(d->delAction, SIGNAL(triggered()), this, SLOT(slotDeleteAction()));

}

TagsManager::~TagsManager()
{
    delete d->tagModel;
    delete d;
}

TagsManager* TagsManager::instance()
{
    if(TagsManager::internalPtr.isNull())
    {
        TagsManager::internalPtr = new TagsManager();
    }
    return TagsManager::internalPtr;
}

void TagsManager::setupUi(KMainWindow *Dialog)
{

     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QHBoxLayout* mainLayout = new QHBoxLayout();

     d->tagPixmap = new QLabel();
     d->tagPixmap->setText("Tag Pixmap");
     d->tagPixmap->setMaximumWidth(40);
     d->tagPixmap->setPixmap(KIcon("tag").pixmap(30,30));

     d->tagMngrView = new TagMngrTreeView(this,d->tagModel);

     d->searchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");
     d->searchBar->setHighlightOnResult(true);
     d->searchBar->setModel(d->tagModel,
                            AbstractAlbumModel::AlbumIdRole,
                            AbstractAlbumModel::AlbumTitleRole);
     d->searchBar->setMaximumWidth(200);
     d->searchBar->setFilterModel(d->tagMngrView->albumFilterModel());



     /** Tree Widget & Actions + Tag Properties sidebar **/

     d->treeWindow = new KMainWindow(this);
     setupActions();

     d->treeWinLayout = new QHBoxLayout();

     d->treeWinLayout->addWidget(d->tagMngrView,9);

     d->tagPropWidget = new TagPropWidget(d->treeWindow);
     d->tagPropWidget->setMaximumWidth(350);
     d->treeWinLayout->addWidget(d->tagPropWidget,3);
     d->tagPropWidget->hide();

     d->listView = new TagList(d->tagMngrView,Dialog);
     d->listView->setMaximumWidth(300);

     QWidget* treeCentralW = new QWidget(this);
     treeCentralW->setLayout(d->treeWinLayout);
     d->treeWindow->setCentralWidget(treeCentralW);

     mainLayout->addWidget(d->listView,2);
     mainLayout->addWidget(d->treeWindow,9);
     mainLayout->addWidget(d->rightToolBar);

     QWidget* centraW = new QWidget(this);
     centraW->setLayout(mainLayout);
     this->setCentralWidget(centraW);

}

void TagsManager::slotOpenProperties()
{
    KMultiTabBarTab* sender = (KMultiTabBarTab*)QObject::sender();
    if(sender->isChecked())
        d->tagPropWidget->show();
    else
        d->tagPropWidget->hide();

}

void TagsManager::slotSelectionChanged()
{
    d->tagPropWidget->slotSelectionChanged(d->tagMngrView->selectedTags());
}

void TagsManager::slotItemChanged()
{

}

void TagsManager::slotAddAction()
{

    TAlbum* parent = d->tagMngrView->currentAlbum();
    QString      title, icon;
    QKeySequence ks;

    if (!TagEditDlg::tagCreate(kapp->activeWindow(), parent, title, icon, ks))
    {
        return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, ks, errMap);
    TagEditDlg::showtagsListCreationError(kapp->activeWindow(), errMap);
}

void TagsManager::slotDeleteAction()
{

    QModelIndexList selected = d->tagMngrView->selectionModel()->selectedIndexes();

    QString tagWithChildrens;
    QString tagWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    foreach(QModelIndex index, selected)
    {
        if(!index.isValid())
            return;

        TAlbum* t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

        int deph = 0;

        if (!t || t->isRoot())
        {
            return;
        }

        AlbumPointer<TAlbum> tag(t);

        // find number of subtags
        int children = 0;
        AlbumIterator iter(tag);

        while (iter.current())
        {
            ++children;
            ++iter;
        }

        if(children)
            tagWithChildrens.append(tag->title() + QString(" "));

        QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());

        if(!assignedItems.isEmpty())
            tagWithImages.append(tag->title() + QString(" "));

        /**
         * Tags must be deleted from children to parents, if we don't want
         * to step on invalid index. Use QMultiMap to order them by distance
         * to root tag
         */
        Album* parent = t;

        while(!parent->isRoot())
        {
            parent = parent->parent();
            deph++;
        }

        sortedTags.insert(deph,tag);
    }
            // ask for deletion of children
        if (!tagWithChildrens.isEmpty())
        {
            int result = KMessageBox::warningContinueCancel(this,
                                                            i18n("Tags '%1' has one or more subtags. "
                                                                "Deleting this will also delete "
                                                                "the subtag."
                                                                "Do you want to continue?",
                                                                tagWithChildrens));

            if (result != KMessageBox::Continue)
            {
                return;
            }
        }

        QString message;

        if (!tagWithImages.isEmpty())
        {
            message = i18n("Tags '%1' are assigned to one or more items. "
                            "Do you want to continue?",
                            tagWithImages);
        }
        else
        {
            message = i18n("Delete '%1' tag(s)?", tagWithImages);
        }

        int result = KMessageBox::warningContinueCancel(0, message,
                                                        i18n("Delete Tag"),
                                                        KGuiItem(i18n("Delete"),
                                                                "edit-delete"));

        if (result == KMessageBox::Continue)
        {
            QMultiMap<int, TAlbum*>::iterator it;
            /**
             * QMultimap doesn't provide reverse iterator, -1 is required
             * because end() points after the last element
             */
            for(it = sortedTags.end()-1; it != sortedTags.begin()-1; --it)
            {
                QString errMsg;
                if (!AlbumManager::instance()->deleteTAlbum(it.value(), errMsg))
                {
                    KMessageBox::error(0, errMsg);
                }
            }
        }
}

void TagsManager::setupActions()
{
    d->mainToolbar = new KToolBar(d->treeWindow);

    QHBoxLayout* tempLayout = new QHBoxLayout();
    tempLayout->addWidget(d->tagPixmap);
    tempLayout->addWidget(d->searchBar);

    QWidget* searchWidget = new QWidget(this);
    searchWidget->setLayout(tempLayout);

    QWidgetAction* searchAction = new QWidgetAction(this);
    searchAction->setDefaultWidget(searchWidget);

    d->mainToolbar->addAction(searchAction);

    d->mainToolbar->addSeparator();

    d->addAction = new KAction(KIcon("list-add"),i18n(""),d->treeWindow);

    d->delAction = new KAction(KIcon("list-remove"),i18n(""),d->treeWindow);

    /** organize group **/
    d->organizeAction   = new KActionMenu(KIcon("autocorrection"),
                                          i18n("Organize"),this);
    d->organizeAction->setDelayed(false);

    KAction* resetIcon     = new KAction(KIcon("view-refresh"),
                                         i18n("Reset tag Icon"), this);

    KAction* createTagAddr = new KAction(KIcon("tag-addressbook"),
                                         i18n("Create Tag from Addess Book"),
                                         this);
    KAction* invSel        = new KAction(KIcon(),
                                         i18n("Invert Selection"), this);

    KAction* expandTree    = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Tag Tree"), this);

    KAction* expandSel     = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Selected Nodes"), this);

    /** Tool tips  **/
    d->addAction->setHelpText(i18n("Add new tag to current tag. "
                                  "Current tag is last clicked tag"));

    d->delAction->setHelpText(i18n("Delete selected items. "
                                  "Also work with multiple items, "
                                  "but won't delete the root tag"));

    resetIcon->setHelpText(i18n("Reset icon to selected tags. "
                               "Works with multiple selection" ));

    invSel->setHelpText(i18n("Invert selection. "
                            "Only visible items will be selected"));

    expandTree->setHelpText(i18n("Expand tag tree by one level"));

    expandSel->setHelpText(i18n("Selected items will be expanded"));

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

    d->organizeAction->addAction(resetIcon);
    d->organizeAction->addAction(createTagAddr);
    d->organizeAction->addAction(invSel);
    d->organizeAction->addAction(expandTree);
    d->organizeAction->addAction(expandSel);

    /** Sync & Export Group **/
    d->syncexportAction = new KActionMenu(KIcon("server-database"),
                                          i18n("Sync &Export"),this);
    d->syncexportAction->setDelayed(false);

    KAction* wrDbImg        = new KAction(KIcon("view-refresh"),
                                          i18n("Write Tags from Database "
                                              "to Image"), this);

    KAction* readTags       = new KAction(KIcon("tag-new"),
                                          i18n("Read Tags from Image"), this);

    KAction* wipeAll        = new KAction(KIcon("draw-eraser"),
                                          i18n("Wipe all tags from Database "
                                              "and read from images"), this);

    KAction* DbToNepomuk    = new KAction(KIcon("nepomuk"),
                                          i18n("Sync Database with Nepomuk"),
                                          this);

    KAction* NepomukToDb    = new KAction(KIcon("nepomuk"),
                                          i18n("Sync Nepomuk to Database"), this);


    wrDbImg->setHelpText(i18n("Write Tags Metadata to Image."));

    readTags->setHelpText(i18n("Read tags from Images into Database"
                              "Existing tags won't be affected"));

    wipeAll->setHelpText(i18n("Delete all tags from database. "
                             "Proceed with caution."));

    DbToNepomuk->setHelpText(i18n("Export all tags from Database to Nepomuk. "
                                 "Digikam with nepomuk support is required "));

    NepomukToDb->setHelpText(i18n("Import tags from Nepomuk."
                                 "Digikam with nepomuk support is required" ));

    connect(wrDbImg, SIGNAL(triggered()),
            this, SLOT(slotWriteToImg()));

    connect(readTags, SIGNAL(triggered()),
            this, SLOT(slotReadFromImg()));

    connect(wipeAll, SIGNAL(triggered()),
            this, SLOT(slotWipeAll()));

    connect(DbToNepomuk, SIGNAL(triggered()),
            this, SLOT(slotNepomukToDb()));

    connect(NepomukToDb, SIGNAL(triggered()),
            this, SLOT(slotNepomukToDb()));

    d->syncexportAction->addAction(wrDbImg);
    d->syncexportAction->addAction(readTags);
    d->syncexportAction->addAction(wipeAll);
    d->syncexportAction->addAction(DbToNepomuk);
    d->syncexportAction->addAction(NepomukToDb);

    /**
     * For testing only
     */
    KAction* forkTags = new KAction(KIcon(),"Create a very big tag tree)",this);

    connect(forkTags, SIGNAL(triggered()), this,
            SLOT(slotForkTags()));

    d->mainToolbar->addAction(d->addAction);
    d->mainToolbar->addAction(d->delAction);
    d->mainToolbar->addAction(d->organizeAction);
    d->mainToolbar->addAction(d->syncexportAction);
    d->mainToolbar->addAction(forkTags);
    d->mainToolbar->addAction(new DLogoAction(this));
    this->addToolBar(d->mainToolbar);

    /**
     * Right Toolbar with vertical properties button
     */
    d->rightToolBar = new KMultiTabBar(KMultiTabBar::Right);
    d->rightToolBar->appendTab(KIcon("tag-properties").pixmap(10,10),
                               0,"Tag Properties");
    d->rightToolBar->setStyle(KMultiTabBar::KDEV3ICON);

    connect(d->rightToolBar->tab(0),SIGNAL(clicked()),
            this, SLOT(slotOpenProperties()));
}

void TagsManager::slotResetTagIcon()
{
    QString errMsg;

    QList<Album*> selected = d->tagMngrView->selectedTags();
    QString icon = QString("tag");
    QList<Album*>::iterator it;

    for(it = selected.begin(); it != selected.end(); ++it )
    {
        TAlbum* tag = dynamic_cast<TAlbum*>(*it);
        if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }
}

void TagsManager::slotCreateTagAddr()
{

}

void TagsManager::slotInvertSel()
{
    QModelIndex root = d->tagMngrView->model()->index(0,0);
    QItemSelectionModel* model = d->tagMngrView->selectionModel();
    QModelIndexList selected = model->selectedIndexes();

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
        int it = 0;
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
    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("This operation can take long time "
                                                         "depending on collection size.\n"
                                                         "Do you want to continue?"
                                                    ));

    if (result != KMessageBox::Continue)
    {
        return;
    }
    MetadataSynchronizer* tool = new MetadataSynchronizer(AlbumList(),
                                                          MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotReadFromImg()
{
    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("This operation can take long time "
                                                         "depending on collection size.\n"
                                                         "Do you want to continue?"
                                                    ));

    if (result != KMessageBox::Continue)
    {
        return;
    }
    MetadataSynchronizer* tool = new MetadataSynchronizer(AlbumList(),
                                                          MetadataSynchronizer::ReadFromFileToDatabase);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotWipeAll()
{
    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("This operation will wipe all tags "
                                                         "from database and will write changes "
                                                         "to image metadata.\n"
                                                         "Do you want to continue?"
                                                    ));

    if (result != KMessageBox::Continue)
    {
        return;
    }
    result = KMessageBox::warningContinueCancel(this,
                                                i18n("This operation can take long time "
                                                     "depending on collection size.\n"
                                                     "Do you want to continue?"
                                                    ));

    if (result != KMessageBox::Continue)
    {
        return;
    }

    AlbumPointerList<TAlbum> tagList;
    QModelIndex root = d->tagMngrView->model()->index(0,0);
    int iter = 0;
    QModelIndex child = root.child(iter++,0);

    while(child.isValid())
    {
        tagList <<  AlbumPointer<TAlbum>(d->tagMngrView->albumForIndex(child));
        child = root.child(iter++,0);
    }

    AlbumPointerList<TAlbum>::iterator it;
    for(it = tagList.begin(); it != tagList.end(); ++it)
    {
        QString errMsg;
        if (!AlbumManager::instance()->deleteTAlbum(*it, errMsg))
        {
            KMessageBox::error(0, errMsg);
        }
    }

    /** Write all changes to file **/

    MetadataSynchronizer* tool = new MetadataSynchronizer(AlbumList(),
                                                          MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setTagsOnly(true);
    tool->start();

}

void TagsManager::slotNepomukToDb()
{

}

void TagsManager::slotDbToNepomuk()
{

}

void TagsManager::slotForkTags()
{
    int numTags = 10;
    TAlbum* parent = d->tagMngrView->currentAlbum();

    if(!parent)
        return;

    QMap<QString, QString> errMap;

    for(int it =0; it< numTags; it++)
    {
        QString      title, icon;
        QKeySequence ks;
        icon = parent->icon();
        title = parent->title() + QString::number(it);
        AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, ks,
                                                   errMap);
        for(int jt = 0 ; jt < numTags; jt++)
        {
            QString childname = title + "H" + QString::number(jt);
            AlbumList jList = TagEditDlg::createTAlbum((TAlbum*)tList.first(),
                                                       childname,
                                                       icon, ks, errMap);
        }
    }
}

} // namespace Digikam
