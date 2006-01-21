/* ============================================================
 * File  : imagedlgbase.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable
 *               filter interface. The dialog layout is
 *               designed to accept custom widgets in
 *               preview and settings area.
 *
 * Copyright 2005-2006 by Gilles Caulier
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Local includes.

#include "imagedlgbase.h"

namespace Digikam
{

class ImageDlgBasePriv
{
public:

    ImageDlgBasePriv()
    {
        aboutData  = 0;
        timer      = 0;
        parent     = 0;
        mainLayout = 0;
    }

    bool            tryAction;

    QGridLayout    *mainLayout;
    
    QWidget        *parent;
    
    QString         name;

    QTimer         *timer;

    KAboutData     *aboutData;
};

ImageDlgBase::ImageDlgBase(QWidget* parent, QString title, QString name, 
                                 bool loadFileSettings, bool tryAction, QFrame* bannerFrame)
            : KDialogBase(Plain, title, Help|Default|User2|User3|Try|Ok|Cancel, Ok,
                          parent, 0, true, true,
                          QString::null,
                          i18n("&Save As..."),
                          i18n("&Load..."))
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

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

    resize(configDialogSize(name + QString::QString(" Tool Dialog")));

    // -------------------------------------------------------------

    d->mainLayout = new QGridLayout( plainPage(), 2, 1 , marginHint(), spacingHint());
    if (bannerFrame)
    {
        bannerFrame->reparent( plainPage(), QPoint::QPoint(0,0) );
        d->mainLayout->addMultiCellWidget(bannerFrame, 0, 0, 0, 1);
    }

    // -------------------------------------------------------------

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

    delete d;
}

void ImageDlgBase::closeEvent(QCloseEvent *e)
{
    saveDialogSize(d->name + QString::QString(" Tool Dialog"));
    e->accept();
}

void ImageDlgBase::slotCancel()
{
    saveDialogSize(d->name + QString::QString(" Tool Dialog"));
    done(Cancel);
}

void ImageDlgBase::slotOk()
{
    saveDialogSize(d->name + QString::QString(" Tool Dialog"));
    finalRendering();
}

void ImageDlgBase::slotHelp()
{
    // If setAboutData() is called by plugin, well DigikamImagePlugins help is lauch, 
    // else digiKam help. In this case, setHelp() method must be used to set anchor and handbook name.

    if (d->aboutData)
        KApplication::kApplication()->invokeHelp(d->name, "digikamimageplugins");
    else
        KDialogBase::slotHelp();
}

void ImageDlgBase::setAboutData(KAboutData *about)
{
    d->aboutData = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ImageDlgBase::setPreviewAreaWidget(QWidget *w)
{
    d->mainLayout->addMultiCellWidget(w, 1, 2, 0, 0);
}

void ImageDlgBase::setUserAreaWidget(QWidget *w)
{
    d->mainLayout->addMultiCellWidget(w, 1, 2, 1, 1);
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

#include "imagedlgbase.moc"
