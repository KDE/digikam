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

#include "tagsmanager.moc"

// Qt includes

#include <QQueue>
#include <QTreeView>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QDBusInterface>

// KDE includes

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

// local includes

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
        treeWinLayout    = 0;
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


    QHBoxLayout*     treeWinLayout;
    KMainWindow*     treeWindow;
    KToolBar*        mainToolbar;
    KMultiTabBar*    rightToolBar;
    KActionMenu*     organizeAction;
    KActionMenu*     syncexportAction;
    KAction*         tagProperties;
    KAction*         addAction;
    KAction*         delAction;

    TagList*         listView;
    TagPropWidget*   tagPropWidget;
    TagModel*        tagModel;
};

TagsManager::TagsManager()
    : KMainWindow(0), d(new Private())
{
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

    /** Set KMainWindow in center of the screen **/
    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());
}

TagsManager::~TagsManager()
{
    delete d->listView;
    delete d->tagMngrView;
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

void TagsManager::setupUi(KMainWindow* const Dialog)
{
     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QHBoxLayout* const mainLayout = new QHBoxLayout();

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

     d->treeWindow    = new KMainWindow(this);
     setupActions();

     d->treeWinLayout = new QHBoxLayout();

     d->treeWinLayout->addWidget(d->tagMngrView,9);

     d->tagPropWidget = new TagPropWidget(d->treeWindow);
     d->tagPropWidget->setMaximumWidth(350);
     d->treeWinLayout->addWidget(d->tagPropWidget,3);
     d->tagPropWidget->hide();

     d->listView      = new TagList(d->tagMngrView,Dialog);
     d->listView->setMaximumWidth(300);

     QWidget* const treeCentralW = new QWidget(this);
     treeCentralW->setLayout(d->treeWinLayout);
     d->treeWindow->setCentralWidget(treeCentralW);

     mainLayout->addWidget(d->listView,2);
     mainLayout->addWidget(d->treeWindow,9);
     mainLayout->addWidget(d->rightToolBar);

     QWidget* const centraW = new QWidget(this);
     centraW->setLayout(mainLayout);
     this->setCentralWidget(centraW);
}

void TagsManager::slotOpenProperties()
{
    KMultiTabBarTab* const sender = (KMultiTabBarTab*)QObject::sender();

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
    TAlbum* const parent = d->tagMngrView->currentAlbum();
    QString       title, icon;
    QKeySequence  ks;

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

    foreach(const QModelIndex& index, selected)
    {
        if(!index.isValid())
            return;

        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

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
                                                        i18n("Tags '%1' have one or more subtags. "
                                                             "Deleting them will also delete "
                                                             "the subtags. "
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

void TagsManager::slotResetTagIcon()
{
    QString errMsg;

    QList<Album*> selected = d->tagMngrView->selectedTags();
    QString icon           = QString("tag");
    QList<Album*>::iterator it;

    for(it = selected.begin(); it != selected.end(); ++it )
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
    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("This operation can take long time "
                                                         "depending on collection size.\n"
                                                         "Do you want to continue?"
                                                    ));

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
    int result = KMessageBox::warningContinueCancel(this,
                                                    i18n("This operation can take long time "
                                                         "depending on collection size.\n"
                                                         "Do you want to continue?"
                                                    ));

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
    QModelIndex root  = d->tagMngrView->model()->index(0,0);
    int iter          = 0;
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

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList(),
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->setTagsOnly(true);
    tool->start();
}

void TagsManager::slotNepomukToDb()
{
#ifdef HAVE_NEPOMUK
    QDBusInterface interface("org.kde.nepomuk.services.digikamnepomukservice",
                             "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");

    if (interface.isValid())
    {
        interface.call(QDBus::NoBlock, "triggerResync", true, false);
    }

#endif // HAVE_NEPOMUK
}

void TagsManager::slotDbToNepomuk()
{
#ifdef HAVE_NEPOMUK
    QDBusInterface interface("org.kde.nepomuk.services.digikamnepomukservice",
                             "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");

    if (interface.isValid())
    {
        interface.call(QDBus::NoBlock, "triggerResync", false, true);
    }

#endif // HAVE_NEPOMUK
}

void TagsManager::slotRemoveTagsFromImgs()
{
    QModelIndexList selList = d->tagMngrView->selectionModel()->selectedIndexes();

    foreach(const QModelIndex& index, selList)
    {
        TAlbum* const t = static_cast<TAlbum*>(d->tagMngrView->albumForIndex(index));

        AlbumPointer<TAlbum> tag(t);

        if(tag->isRoot())
            continue;

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

    QWidgetAction* const pixMapAction = new QWidgetAction(this);
    pixMapAction->setDefaultWidget(d->tagPixmap);

    QWidgetAction* const searchAction = new QWidgetAction(this);
    searchAction->setDefaultWidget(d->searchBar);

    d->mainToolbar->addAction(pixMapAction);
    d->mainToolbar->addAction(searchAction);

    d->mainToolbar->addSeparator();

    d->addAction = new KAction(KIcon("list-add"),"",d->treeWindow);

    d->delAction = new KAction(KIcon("list-remove"),"",d->treeWindow);

    /** organize group **/
    d->organizeAction      = new KActionMenu(KIcon("autocorrection"),
                                             i18n("Organize"),this);
    d->organizeAction->setDelayed(false);

    KAction* resetIcon     = new KAction(KIcon("view-refresh"),
                                         i18n("Reset tag Icon"), this);

    KAction* createTagAddr = new KAction(KIcon("tag-addressbook"),
                                         i18n("Create Tag from Address Book"),
                                         this);
    KAction* invSel        = new KAction(KIcon("tag-reset"),
                                         i18n("Invert Selection"), this);

    KAction* expandTree    = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Tag Tree"), this);

    KAction* expandSel     = new KAction(KIcon("format-indent-more"),
                                         i18n("Expand Selected Nodes"), this);
    KAction* delTagFromImg = new KAction(KIcon("tag-delete"),
                                         i18n("Remove Tag from Images"), this);

    /** Tool tips  **/
    d->addAction->setHelpText(i18n("Add new tag to current tag. "
                                  "Current tag is last clicked tag."));

    d->delAction->setHelpText(i18n("Delete selected items. "
                                  "Also work with multiple items, "
                                  "but won't delete the root tag."));

    resetIcon->setHelpText(i18n("Reset icon to selected tags. "
                               "Works with multiple selection." ));

    invSel->setHelpText(i18n("Invert selection. "
                            "Only visible items will be selected"));

    expandTree->setHelpText(i18n("Expand tag tree by one level"));

    expandSel->setHelpText(i18n("Selected items will be expanded"));

    delTagFromImg->setHelpText(i18n("Delete selected tag(s) from images. "
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
    d->syncexportAction = new KActionMenu(KIcon("server-database"),
                                          i18n("Sync &Export"),this);
    d->syncexportAction->setDelayed(false);

    KAction* const wrDbImg  = new KAction(KIcon("view-refresh"),
                                          i18n("Write Tags from Database "
                                              "to Image"), this);

    KAction* const readTags = new KAction(KIcon("tag-new"),
                                          i18n("Read Tags from Image"), this);

    KAction* const wipeAll  = new KAction(KIcon("draw-eraser"),
                                          i18n("Wipe all tags from Database "
                                              "and read from images"), this);


    wrDbImg->setHelpText(i18n("Write Tags Metadata to Image."));

    readTags->setHelpText(i18n("Read tags from Images into Database. "
                              "Existing tags won't be affected"));

    wipeAll->setHelpText(i18n("Delete all tags from database. "
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

#ifdef HAVE_NEPOMUK
    KAction* const DbToNepomuk = new KAction(KIcon("nepomuk"),
                                             i18n("Sync Database with Nepomuk"),
                                             this);

    KAction* const NepomukToDb = new KAction(KIcon("nepomuk"),
                                             i18n("Sync Nepomuk to Database"), this);
    DbToNepomuk->setHelpText(i18n("Export all tags from Database to Nepomuk. "
                                 "digiKam with Nepomuk support is required."));

    NepomukToDb->setHelpText(i18n("Import tags from Nepomuk. "
                                 "digiKam with Nepomuk support is required." ));

    connect(DbToNepomuk, SIGNAL(triggered()),
            this, SLOT(slotDbToNepomuk()));

    connect(NepomukToDb, SIGNAL(triggered()),
            this, SLOT(slotNepomukToDb()));

    d->syncexportAction->addAction(DbToNepomuk);
    d->syncexportAction->addAction(NepomukToDb);

#endif

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
    d->rightToolBar->appendTab(KIcon("tag-properties").pixmap(10,10),
                               0,i18n("Tag Properties"));
    d->rightToolBar->setStyle(KMultiTabBar::KDEV3ICON);

    connect(d->rightToolBar->tab(0),SIGNAL(clicked()),
            this, SLOT(slotOpenProperties()));
}

} // namespace Digikam
