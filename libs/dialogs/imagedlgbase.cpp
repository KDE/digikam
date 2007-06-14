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
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <q3groupbox.h>
#include <qlabel.h>
#include <q3whatsthis.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>
#include <q3frame.h>
#include <qtimer.h>
#include <qsplitter.h>
#include <q3hbox.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3GridLayout>

// KDE includes.

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

    Q3GridLayout    *mainLayout;
    
    QWidget        *parent;
    
    QString         name;

    QTimer         *timer;

    Q3HBox          *hbox;

    QSplitter      *splitter;

    KAboutData     *aboutData;

    Sidebar        *settingsSideBar;
};

ImageDlgBase::ImageDlgBase(QWidget* parent, QString title, QString name, 
                           bool loadFileSettings, bool tryAction, Q3Frame* bannerFrame)
            : KDialogBase(Plain, 0, Help|Default|User1|User2|User3|Try|Ok|Cancel, Ok,
                          parent, 0, true, true,
                          QString(),
                          i18n("&Save As..."),
                          i18n("&Load..."))
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    setCaption(DImgInterface::defaultInterface()->getImageFileName() + QString(" - ") + title);
    showButton(User1, false);

    d = new ImageDlgBasePriv;
    d->parent    = parent;
    d->name      = name;
    d->tryAction = tryAction;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    resize(configDialogSize(name + QString(" Tool Dialog")));

    // -------------------------------------------------------------

    d->mainLayout = new Q3GridLayout( plainPage(), 2, 1);
    if (bannerFrame)
    {
        bannerFrame->reparent( plainPage(), QPoint(0, 0) );
        d->mainLayout->addMultiCellWidget(bannerFrame, 0, 0, 0, 1);
    }

    // -------------------------------------------------------------

    d->hbox     = new Q3HBox(plainPage());
    d->splitter = new QSplitter(d->hbox);
    d->splitter->setFrameStyle( Q3Frame::NoFrame );
    d->splitter->setFrameShadow( Q3Frame::Plain );
    d->splitter->setFrameShape( Q3Frame::NoFrame );    
    d->splitter->setOpaqueResize(false);
    
    d->mainLayout->addMultiCellWidget(d->hbox, 1, 2, 0, 1);
    d->mainLayout->setColStretch(0, 10);
    d->mainLayout->setRowStretch(2, 10);

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

void ImageDlgBase::readSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup(d->name + QString(" Tool Dialog"));
    if(config->hasKey("SplitterSizes"))
        d->splitter->setSizes(config->readIntListEntry("SplitterSizes"));

    readUserSettings();
}

void ImageDlgBase::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup(d->name + QString(" Tool Dialog"));
    config->writeEntry("SplitterSizes", d->splitter->sizes());
    config->sync();
    saveDialogSize(d->name + QString(" Tool Dialog"));
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
    else
        KDialogBase::slotHelp();
}

void ImageDlgBase::setAboutData(KAboutData *about)
{
    d->aboutData = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("digiKam Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ImageDlgBase::setPreviewAreaWidget(QWidget *w)
{
    w->reparent( d->splitter, QPoint(0, 0) );
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred,
                              QSizePolicy::Expanding,
                              2, 1);
    w->setSizePolicy(rightSzPolicy);
}

void ImageDlgBase::setUserAreaWidget(QWidget *w)
{
    QString sbName(d->name + QString(" Image Plugin Sidebar"));
    d->settingsSideBar = new Sidebar(d->hbox, sbName.ascii(), Sidebar::Right);
    d->settingsSideBar->setSplitter(d->splitter);
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
    d->timer->start(500, true);
}

}  // NameSpace Digikam

