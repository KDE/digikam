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

#include <kdebug.h>
#include <klocale.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kmultitabbar.h>
#include <QTreeView>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <kactionmenu.h>

#include "tagsmanager.h"
#include "tagpropwidget.h"
#include "tagfolderview.h"

namespace Digikam
{


class TagsManager::PrivateTagMngr
{

public:

    PrivateTagMngr()
    {
        treeModel       = 0;
        tagmngrLabel    = 0;
        tagPixmap       = 0;
        digikamPixmap   = 0;
        lineEdit        = 0;
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

    ~PrivateTagMngr()
    {
        delete treeModel;
        delete tagmngrLabel;
        delete tagPixmap;
        delete digikamPixmap;
        delete lineEdit;
        delete treeWinLayout;
        delete treeWindow;
        delete mainToolbar;
        delete rightToolBar;
        delete organizeAction;
        delete syncexportAction;
        delete tagProperties;
        delete addAction;
        delete delAction;
        delete listView;
    }

    QTreeView*      treeModel;
    TagFolderView*  tagFolderView;
    QLabel *        tagmngrLabel;
    QLabel*         tagPixmap;
    QLabel*         digikamPixmap;
    QLineEdit*      lineEdit;

    QHBoxLayout*    treeWinLayout;
    KMainWindow*    treeWindow;
    KToolBar*       mainToolbar;
    KMultiTabBar*   rightToolBar;
    KActionMenu*    organizeAction;
    KActionMenu*    syncexportAction;
    KAction*        tagProperties;
    KAction*        addAction;
    KAction*        delAction;
    QListView*      listView;

    TagPropWidget*  tagPropWidget;

    TagModel*       tagModel;
};

TagsManager::TagsManager(TagModel* model)
    : KDialog(0), d(new PrivateTagMngr())
{

    /** No buttons **/
    d->tagModel = model;
    this->setButtons(0x00);

    setupUi(this);
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

     d->lineEdit = new QLineEdit();
     d->lineEdit->setText(i18n("Search..."));

     d->lineEdit->setMaximumWidth(200);

     d->digikamPixmap = new QLabel();
     QPixmap dpix (KStandardDirs::locate("data", "digikam/about/top-left-digikam.png"));
     d->digikamPixmap->setPixmap(dpix.scaled(40,40,Qt::KeepAspectRatio));
     d->digikamPixmap->setMaximumHeight(40);
     d->digikamPixmap->setScaledContents(true);


     QHBoxLayout* tempLayout = new QHBoxLayout();
     tempLayout->setAlignment(Qt::AlignLeft);
     tempLayout->addWidget(d->tagPixmap);
     tempLayout->addWidget(d->lineEdit);
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

     /** Tree Widget & Actions + Tag Properties sidebar **/

     d->treeWindow = new KMainWindow(this);
     d->mainToolbar = new KToolBar(d->treeWindow);

     d->addAction = new KAction(i18n("Add"),d->treeWindow);
     d->delAction = new KAction(i18n("Del"),d->treeWindow);

     d->organizeAction   = new KActionMenu(i18n("Organize"),this);
     d->organizeAction->setDelayed(false);
     d->organizeAction->addAction(new KAction("New Tag",this));

     d->syncexportAction = new KActionMenu(i18n("Sync & Export"),this);

     d->tagProperties = new KAction(i18n("Tag Properties"),this);

     d->mainToolbar->addAction(d->addAction);
     d->mainToolbar->addAction(d->delAction);
     d->mainToolbar->addAction(d->organizeAction);
     d->mainToolbar->addAction(d->syncexportAction);
     d->treeWindow->addToolBar(d->mainToolbar);


     d->rightToolBar = new KMultiTabBar(KMultiTabBar::Right);
     d->rightToolBar->appendTab(dpix,0,"Tag Properties");
     d->rightToolBar->setStyle(KMultiTabBar::KDEV3ICON);

     connect(d->rightToolBar->tab(0),SIGNAL(clicked()),this, SLOT(slotOpenProperties()));

     d->treeWinLayout = new QHBoxLayout(d->treeWindow);

     if(d->tagModel == 0)
     {
        d->treeModel = new QTreeView();
        d->treeWinLayout->addWidget(d->treeModel,9);
     }
     else
     {
         d->tagFolderView = new TagFolderView(this,d->tagModel);
         d->treeWinLayout->addWidget(d->tagFolderView,9);
     }

     d->tagPropWidget = new TagPropWidget(d->treeModel);
     d->tagPropWidget->setMaximumWidth(350);
     d->treeWinLayout->addWidget(d->tagPropWidget,3);
     d->tagPropWidget->hide();

     /** Tag List and Tag Manager Title **/

     QHBoxLayout* thirdLine = new QHBoxLayout();
     d->listView = new QListView(Dialog);
     d->listView->setObjectName(QString::fromUtf8("listView"));

     d->listView->setContextMenuPolicy(Qt::CustomContextMenu);
     d->listView->setMaximumWidth(300);

     QVBoxLayout* listLayout = new QVBoxLayout();
     listLayout->addWidget(d->tagmngrLabel);
     listLayout->addWidget(d->listView);


     QWidget* treeCentralW = new QWidget(d->treeWindow);
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

} // namespace Digikam
