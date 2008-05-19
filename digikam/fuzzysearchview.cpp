/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QFrame>
#include <QLayout>
#include <QPushButton>

// KDE include.

#include <khbox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <khuesaturationselect.h>
#include <kcolorvalueselector.h>
#include <knuminput.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "sketchwidget.h"
#include "fuzzysearchview.h"
#include "fuzzysearchview.moc"

namespace Digikam
{

class FuzzySearchViewPriv
{

public:

    FuzzySearchViewPriv()
    {
        sketchWidget = 0;
        hsSelector   = 0;
        vSelector    = 0;
        penSize      = 0;
        clearButton  = 0;
    }

    QPushButton            *clearButton;

    KIntNumInput           *penSize;

    KHueSaturationSelector *hsSelector;

    KColorValueSelector    *vSelector;

    SketchWidget           *sketchWidget;
};

FuzzySearchView::FuzzySearchView(QWidget *parent)
               : QFrame(parent)
{
    d = new FuzzySearchViewPriv;
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QGridLayout *grid = new QGridLayout(this);

    // ---------------------------------------------------------------

    d->sketchWidget = new SketchWidget(this);
    d->hsSelector   = new KHueSaturationSelector(this);
    d->vSelector    = new KColorValueSelector(this);
    d->hsSelector->setMinimumSize(200, 142);
    d->vSelector->setMinimumSize(26, 142);

    // ---------------------------------------------------------------

    d->penSize = new KIntNumInput(this);
    d->penSize->setRange(1, 40, 1);
    d->penSize->setSliderEnabled(true);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("<p>Set here the brush size in pixels used to draw sketch."));

    // ---------------------------------------------------------------

    d->clearButton = new QPushButton(i18n("Clear"), this);
    d->clearButton->setWhatsThis(i18n("<p>Use this button to clear sketch contents."));

    // ---------------------------------------------------------------

    grid->addWidget(d->sketchWidget, 0, 0, 1, 2);
    grid->addWidget(d->hsSelector,   1, 0, 1, 1);
    grid->addWidget(d->vSelector,    1, 1, 1, 1);
    grid->addWidget(d->penSize,      2, 0, 1, 2);
    grid->addWidget(d->clearButton,  3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setColumnStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged()));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->clearButton, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotClear()));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(const QImage&)),
            this, SLOT(slotSketchChanged(const QImage&)));
}

FuzzySearchView::~FuzzySearchView()
{
    writeConfig();
    delete d;
}

void FuzzySearchView::slotHSChanged(int h, int s)
{
    d->vSelector->blockSignals(true);
    d->vSelector->setHue(h);
    d->vSelector->setSaturation(s);
    d->vSelector->updateContents();
    d->vSelector->repaint();
    d->vSelector->blockSignals(false);
    slotVChanged();
}

void FuzzySearchView::slotVChanged()
{
    int hue      = d->hsSelector->xValue();
    int sat      = d->hsSelector->yValue();
    int val      = d->vSelector->value();
    QColor color = QColor::fromHsv(hue, sat, val);

    d->sketchWidget->setPenColor(color);
}

void FuzzySearchView::slotSketchChanged(const QImage& /*img*/)
{
    // TODO: query database here !
}

void FuzzySearchView::readConfig()
{
/*    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("TimeLine SideBar"));

    d->timeUnitCB->setCurrentIndex(group.readEntry("Histogram TimeUnit", (int)TimeLineWidget::Month));
    slotTimeUnitChanged(d->timeUnitCB->currentIndex());

    int id = group.readEntry("Histogram Scale", (int)TimeLineWidget::LinScale);
    if ( d->scaleBG->button( id ) )
       d->scaleBG->button( id )->setChecked(true);
    slotScaleChanged(d->scaleBG->checkedId());

    QDateTime now = QDateTime::currentDateTime();
    d->timeLineWidget->setCursorDateTime(group.readEntry("Cursor Position", now));
    d->timeLineWidget->setCurrentIndex(d->timeLineWidget->indexForCursorDateTime());*/
}

void FuzzySearchView::writeConfig()
{
/*
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("TimeLine SideBar"));
    group.writeEntry("Histogram TimeUnit", d->timeUnitCB->currentIndex());
    group.writeEntry("Histogram Scale", d->scaleBG->checkedId());
    group.writeEntry("Cursor Position", d->timeLineWidget->cursorDateTime());
    group.sync();*/
}

void FuzzySearchView::setActive(bool val)
{
/*    if (d->timeLineFolderView->selectedItem()) 
    {
        d->timeLineFolderView->setActive(val);
    }
    else if (val)
    {
        int totalCount = 0;
        DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);
        if (list.isEmpty())
        {
            AlbumManager::instance()->setCurrentAlbum(0);
        }
        else
        {
            AlbumList sList = AlbumManager::instance()->allSAlbums();
            for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
            {
                SAlbum* salbum = (SAlbum*)(*it);
                if (salbum->title() == d->timeLineFolderView->currentTimeLineSearchName())
                    AlbumManager::instance()->setCurrentAlbum(salbum);
            }
        }
    }*/
}

}  // NameSpace Digikam
