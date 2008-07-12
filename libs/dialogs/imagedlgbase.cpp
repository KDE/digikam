/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-23
 * Description : simple plugins dialog without threadable
 *               filter interface. The dialog layout is
 *               designed to accept custom widgets in
 *               preview and settings area.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QFrame>
#include <QTimer>
#include <QSplitter>
#include <QGridLayout>

// KDE includes.

#include <kpushbutton.h>
#include <kcursor.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <ktoolinvocation.h>
#include <kvbox.h>

// Local includes.

#include "ddebug.h"
#include "sidebar.h"
#include "dimginterface.h"
#include "imagedlgbase.h"
#include "imagedlgbase.moc"

namespace Digikam
{

class ImageDlgBasePriv
{
public:

    ImageDlgBasePriv()
    {
        aboutData       = 0;
        timer           = 0;
        parent          = 0;
        mainLayout      = 0;
        hbox            = 0;
        settingsSideBar = 0;
        splitter        = 0;
    }

    bool            tryAction;

    QGridLayout    *mainLayout;

    QWidget        *parent;

    QString         name;

    QTimer         *timer;

    KHBox          *hbox;

    SidebarSplitter*splitter;

    KAboutData     *aboutData;

    Sidebar        *settingsSideBar;
};

ImageDlgBase::ImageDlgBase(QWidget* parent, QString title, QString name, 
                           bool loadFileSettings, 
                           bool tryAction)
            : KDialog(parent)
{
    setButtons(Help|Default|User1|User2|User3|Try|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setButtonText(User1, QString());
    setButtonText(User2, i18n("&Save As..."));
    setButtonText(User3, i18n("&Load..."));

    kapp->setOverrideCursor( Qt::WaitCursor );
    setCaption(DImgInterface::defaultInterface()->getImageFileName() + QString(" - ") + title);
    showButton(User1, false);

    d = new ImageDlgBasePriv;
    d->parent    = parent;
    d->name      = name;
    d->tryAction = tryAction;

    setButtonWhatsThis( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    restoreDialogSize(KGlobal::config()->group(name + QString(" Tool Dialog")));

    // -------------------------------------------------------------

    setMainWidget(new QWidget(this));
    d->mainLayout = new QGridLayout(mainWidget());

    // -------------------------------------------------------------

    d->hbox     = new KHBox(mainWidget());
    d->splitter = new SidebarSplitter(d->hbox);
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);

    d->mainLayout->addWidget(d->hbox, 1, 0, 2, 2 );
    d->mainLayout->setColumnStretch(0, 10);
    d->mainLayout->setRowStretch(2, 10);
    d->mainLayout->setMargin(0);
    d->mainLayout->setSpacing(spacingHint());

    // -------------------------------------------------------------

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotUser1()));

    connect(this, SIGNAL(user2Clicked()),
            this, SLOT(slotUser2()));

    connect(this, SIGNAL(user3Clicked()),
            this, SLOT(slotUser3()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancel()));

    connect(this, SIGNAL(tryClicked()),
            this, SLOT(slotEffect()));

    connect(this, SIGNAL(defaultClicked()),
            this, SLOT(slotDefault()));

    connect(this, SIGNAL(helpClicked()),
            this, SLOT(slotHelp()));

    // -------------------------------------------------------------

    kapp->restoreOverrideCursor();
}

ImageDlgBase::~ImageDlgBase()
{
    if (d->timer)
       delete d->timer;

    if (d->aboutData)
       delete d->aboutData;

    delete d->settingsSideBar;
    delete d;
}

void ImageDlgBase::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->name + QString(" Tool Dialog"));
    d->splitter->restoreState(group);

    readUserSettings();
}

void ImageDlgBase::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->name + QString(" Tool Dialog"));
    d->splitter->saveState(group);
    saveDialogSize(group);
    config->sync();
}

void ImageDlgBase::closeEvent(QCloseEvent *e)
{
    writeSettings();
    e->accept();
}

void ImageDlgBase::slotCancel()
{
    writeSettings();
    done(Cancel);
}

void ImageDlgBase::slotOk()
{
    writeSettings();
    writeUserSettings();
    finalRendering();
}

void ImageDlgBase::slotDefault()
{
    resetValues();
    slotEffect();
}

void ImageDlgBase::slotHelp()
{
    // If setAboutData() is called by plugin, well DigikamImagePlugins help is launched, 
    // else digiKam help. In this case, setHelp() method must be used to set anchor and handbook name.

    if (d->aboutData)
        KToolInvocation::invokeHelp(d->name, "digikam");
}

void ImageDlgBase::setAboutData(KAboutData *about)
{
    disconnect(this, SIGNAL(helpClicked()),
               this, SLOT(slotHelp()));

    d->aboutData            = about;
    KPushButton *helpButton = button( Help );
    KHelpMenu* helpMenu     = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeAction(helpMenu->menu()->actions().first());
    QAction *handbook       = new QAction(i18n("digiKam Handbook"), this);
    connect(handbook, SIGNAL(triggered(bool)),
            this, SLOT(slotHelp()));
    helpMenu->menu()->insertAction(helpMenu->menu()->actions().first(), handbook);
    helpButton->setDelayedMenu( helpMenu->menu() );
}

void ImageDlgBase::setPreviewAreaWidget(QWidget *w)
{
    w->setParent(d->splitter);
    d->splitter->setStretchFactor(0, 10);      // set widget default size to max.
}

void ImageDlgBase::setUserAreaWidget(QWidget *w)
{
    QString sbName(d->name + QString(" Image Plugin Sidebar"));
    d->settingsSideBar = new Sidebar(d->hbox, d->splitter, KMultiTabBar::Right);
    d->settingsSideBar->setObjectName(sbName.toAscii());
    d->settingsSideBar->appendTab(w, SmallIcon("configure"), i18n("Settings"));
    d->settingsSideBar->loadViewState();

    readSettings();
}

void ImageDlgBase::slotTimer()
{
    if (d->timer)
    {
       d->timer->stop();
       delete d->timer;
    }

    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

}  // NameSpace Digikam
