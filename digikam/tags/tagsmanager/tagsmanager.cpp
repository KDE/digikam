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
#include "taglist.h"

#include "tagfolderview.h"
#include "ddragobjects.h"
#include "searchtextbar.h"
#include "tageditdlg.h"
#include "albumdb.h"

namespace Digikam
{


class TagsManager::PrivateTagMngr
{

public:

    PrivateTagMngr()
    {
        tagmngrLabel    = 0;
        tagPixmap       = 0;
        digikamPixmap   = 0;
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
    }

    TagFolderView*  tagFolderView;
    QLabel*         tagmngrLabel;
    QLabel*         tagPixmap;
    QLabel*         digikamPixmap;
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
    TagList*      listView;

    TagPropWidget*  tagPropWidget;

    TagModel*       tagModel;
};

TagsManager::TagsManager(TagModel* model)
    : KDialog(0), d(new PrivateTagMngr())
{

    /** No buttons **/
    d->tagModel = model;
    d->tagModel->setCheckable(false);
    this->setButtons(0x00);

    setupUi(this);

    /*----------------------------Connects---------------------------*/

    connect(d->tagFolderView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalSelectionChanged(TAlbum*)),
            d->tagPropWidget, SLOT(slotSelectionChanged(TAlbum*)));

    connect(d->addAction, SIGNAL(triggered()), this, SLOT(slotAddAction()));

    connect(d->delAction, SIGNAL(triggered()), this, SLOT(slotDeleteAction()));

}

TagsManager::~TagsManager()
{
    delete d;
}

void TagsManager::setupUi(KDialog *Dialog)
{

     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QVBoxLayout* mainLayout = new QVBoxLayout();

     /** Tag Pixmap, search and digikam.org logo **/
     QHBoxLayout* firstLine = new QHBoxLayout();

     d->tagPixmap = new QLabel();
     d->tagPixmap->setText("Tag Pixmap");
     d->tagPixmap->setMaximumWidth(40);
     d->tagPixmap->setPixmap(KIcon("tag").pixmap(30,30));

     d->digikamPixmap = new QLabel();
     QPixmap dpix (KStandardDirs::locate("data", "digikam/about/top-left-digikam.png"));
     d->digikamPixmap->setPixmap(dpix.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
     d->digikamPixmap->setMaximumHeight(40);
     d->digikamPixmap->setScaledContents(true);

     d->tagFolderView = new TagFolderView(this,d->tagModel);

     d->searchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");
     d->searchBar->setHighlightOnResult(true);
     d->searchBar->setModel(d->tagModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
     d->searchBar->setMaximumWidth(200);
     d->searchBar->setFilterModel(d->tagFolderView->albumFilterModel());

     QHBoxLayout* tempLayout = new QHBoxLayout();
     tempLayout->setAlignment(Qt::AlignLeft);
     tempLayout->addWidget(d->tagPixmap);
     tempLayout->addWidget(d->searchBar);
     firstLine->addLayout(tempLayout);
     firstLine->addWidget(d->digikamPixmap);

     d->tagmngrLabel = new QLabel(Dialog);
     d->tagmngrLabel->setObjectName(QString::fromUtf8("label"));
     d->tagmngrLabel->setText(i18n("Tags Manager"));
     d->tagmngrLabel->setAlignment(Qt::AlignCenter);
     QFont font2;
     font2.setPointSize(12);
     font2.setBold(true);
     font2.setWeight(75);
     d->tagmngrLabel->setFont(font2);
     d->tagmngrLabel->setAutoFillBackground(true);
     d->tagmngrLabel->setMinimumHeight(30);

     /** Tree Widget & Actions + Tag Properties sidebar **/

     d->treeWindow = new KMainWindow(this);
     setupActions();

     d->treeWinLayout = new QHBoxLayout();

     d->treeWinLayout->addWidget(d->tagFolderView,9);

     d->tagPropWidget = new TagPropWidget(d->treeWindow);
     d->tagPropWidget->setMaximumWidth(350);
     d->treeWinLayout->addWidget(d->tagPropWidget,3);
     d->tagPropWidget->hide();

     /** Tag List and Tag Manager Title **/

     QHBoxLayout* thirdLine = new QHBoxLayout();
     d->listView = new TagList(d->tagFolderView,Dialog);
     d->listView->setMaximumWidth(300);

     QVBoxLayout* listLayout = new QVBoxLayout();
     listLayout->addWidget(d->tagmngrLabel);
     listLayout->addWidget(d->listView);


     QWidget* treeCentralW = new QWidget(this);
     treeCentralW->setLayout(d->treeWinLayout);

     d->treeWindow->setCentralWidget(treeCentralW);

     thirdLine->addLayout(listLayout,2);
     thirdLine->addWidget(d->treeWindow,9);
     thirdLine->addWidget(d->rightToolBar);

     mainLayout->addLayout(firstLine);
     mainLayout->addLayout(thirdLine);

     this->mainWidget()->setLayout(mainLayout);

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
    TAlbum* currentAl = d->tagFolderView->currentAlbum();
    /** When deleting a tag, current Album is not valid **/
    if(currentAl)
        emit signalSelectionChanged(currentAl);
}

void TagsManager::slotItemChanged()
{

}

void TagsManager::slotAddAction()
{
    QModelIndexList selected = d->tagFolderView->selectionModel()->selectedIndexes();

    if(selected.isEmpty())
        return;
    TAlbum* parent = static_cast<TAlbum*>(d->tagFolderView->albumForIndex(selected.at(0)));
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

    QModelIndexList selected = d->tagFolderView->selectionModel()->selectedIndexes();

    QString tagWithChildrens;
    QString tagWithImages;
    QMultiMap<int, TAlbum*> sortedTags;

    foreach(QModelIndex index, selected)
    {
        if(!index.isValid())
            return;

        TAlbum* t = static_cast<TAlbum*>(d->tagFolderView->albumForIndex(index));

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

    d->addAction = new KAction(KIcon("list-add"),i18n(""),d->treeWindow);

    d->delAction = new KAction(KIcon("list-remove"),i18n(""),d->treeWindow);

    /** organize group **/
    d->organizeAction   = new KActionMenu(KIcon("autocorrection"),
                                           i18n("Organize"),this);
    d->organizeAction->setDelayed(false);

    KAction* resetIcon     = new KAction(KIcon("view-refresh"),
                                          i18n("Reset tag Icon"), this);
    KAction* createTagAddr = new KAction(KIcon("tag-addressbook"),
                                          i18n("Create Tag from Addess Book"), this);
    KAction* invSel        = new KAction(KIcon(),
                                          i18n("Invert Selection"), this);
    KAction* expandTree    = new KAction(KIcon("format-indent-more"),
                                          i18n("Expand Tag Tree"), this);
    KAction* expandSel     = new KAction(KIcon("format-indent-more"),
                                          i18n("Expand Selected Nodes"), this);

    connect(resetIcon, SIGNAL(triggered()),
            this, SLOT(slotResetTagIcon()));

    connect(createTagAddr, SIGNAL(triggered()),
            this, SLOT(slotCreateTagAddr()));

    connect(invSel, SIGNAL(triggered()),
            this, SLOT(slotInvertSel()));

    connect(expandTree, SIGNAL(triggered()),
            this, SLOT(slotExpandTree()));

    connect(expandSel, SIGNAL(triggered()),
            this, SLOT(slotExpandSelected()));

    d->organizeAction->addAction(resetIcon);
    d->organizeAction->addAction(createTagAddr);
    d->organizeAction->addAction(invSel);
    d->organizeAction->addAction(expandTree);
    d->organizeAction->addAction(expandSel);

    /** Sync & Export Group **/
    d->syncexportAction = new KActionMenu(KIcon("server-database"),i18n("Sync &Export"),this);
    d->syncexportAction->setDelayed(false);

    KAction* wrDbImg       = new KAction(KIcon("view-refresh"),
                                         i18n("Write Tags from Database to Image"), this);
    KAction* readTags      = new KAction(KIcon("tag-new"),
                                         i18n("Read Tags from Image"), this);
    KAction* wipeAll       = new KAction(KIcon("draw-eraser"),
                                         i18n("Wipe all tags from Database and read from images"), this);
    KAction* exportToKipi  = new KAction(KIcon("kipi"),
                                         i18n("Export to kipi"), this);
    KAction* syncNepomuk   = new KAction(KIcon("nepomuk"),
                                         i18n("Sync Database with Nepomuk"), this);

    connect(wrDbImg, SIGNAL(triggered()),
            this, SLOT(slotWriteToImg()));

    connect(readTags, SIGNAL(triggered()),
            this, SLOT(slotReadFromImg()));

    connect(wipeAll, SIGNAL(triggered()),
            this, SLOT(slotWipeAll()));

    connect(exportToKipi, SIGNAL(triggered()),
            this, SLOT(slotExportKipi()));

    connect(syncNepomuk, SIGNAL(triggered()),
            this, SLOT(slotSyncNepomuk()));

    d->syncexportAction->addAction(wrDbImg);
    d->syncexportAction->addAction(readTags);
    d->syncexportAction->addAction(wipeAll);
    d->syncexportAction->addAction(exportToKipi);
    d->syncexportAction->addAction(syncNepomuk);

    d->mainToolbar->addAction(d->addAction);
    d->mainToolbar->addAction(d->delAction);
    d->mainToolbar->addAction(d->organizeAction);
    d->mainToolbar->addAction(d->syncexportAction);
    d->treeWindow->addToolBar(d->mainToolbar);


    /**
     * Right Toolbar with vertical properties button
     */
    d->rightToolBar = new KMultiTabBar(KMultiTabBar::Right);
    d->rightToolBar->appendTab(KIcon("tag-properties").pixmap(10,10),0,"Tag Properties");
    d->rightToolBar->setStyle(KMultiTabBar::KDEV3ICON);

    connect(d->rightToolBar->tab(0),SIGNAL(clicked()),this, SLOT(slotOpenProperties()));
}

void TagsManager::slotResetTagIcon()
{

}

void TagsManager::slotCreateTagAddr()
{

}

void TagsManager::slotInvertSel()
{

}

void TagsManager::slotExpandTree()
{

}

void TagsManager::slotExpandSelected()
{

}

void TagsManager::slotWriteToImg()
{

}

void TagsManager::slotReadFromImg()
{

}

void TagsManager::slotWipeAll()
{

}

void TagsManager::slotExportKipi()
{

}

void TagsManager::slotSyncNepomuk()
{

}

} // namespace Digikam
