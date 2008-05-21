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

#include <QLabel>
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
#include <khbox.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "haariface.h"
#include "searchxml.h"
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
        results      = 0;
        clearButton  = 0;
    }

    QPushButton            *clearButton;

    KIntNumInput           *penSize;
    KIntNumInput           *results;

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
        
    QWidget *box      = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(box);
    KHBox *drawingBox = new KHBox(box);
    d->sketchWidget   = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    vlay->addStretch(10);
    vlay->addWidget(drawingBox, 0, Qt::AlignCenter);
    vlay->addStretch(10);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    // ---------------------------------------------------------------

    d->hsSelector = new KHueSaturationSelector(this);
    d->vSelector  = new KColorValueSelector(this);
    d->hsSelector->setMinimumSize(200, 142);
    d->vSelector->setMinimumSize(26, 142);

    QString tips(i18n("<p>Set here the brush color used to draw sketch."));
    d->hsSelector->setWhatsThis(tips);
    d->vSelector->setWhatsThis(tips);

    // ---------------------------------------------------------------

    KHBox *hbox        = new KHBox(this);
    QLabel *brushLabel = new QLabel(i18n("Brush size:"), hbox);
    d->penSize         = new KIntNumInput(hbox);
    d->penSize->setRange(1, 40, 1);
    d->penSize->setSliderEnabled(false);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("<p>Set here the brush size in pixels used to draw sketch."));
    hbox->setStretchFactor(brushLabel, 10);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->clearButton = new QPushButton(i18n("Clear"), this);
    d->clearButton->setWhatsThis(i18n("<p>Use this button to clear sketch contents."));

    // ---------------------------------------------------------------

    KHBox *hbox2         = new KHBox(this);
    QLabel *resultsLabel = new QLabel(i18n("Results:"), hbox2);
    d->results           = new KIntNumInput(hbox2);
    d->results->setRange(1, 50, 1);
    d->results->setSliderEnabled(false);
    d->results->setValue(10);
    d->results->setWhatsThis(i18n("<p>Set here the number of items to find."));
    hbox2->setStretchFactor(resultsLabel, 10);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    grid->addWidget(box,            0, 0, 1, 3);
    grid->addWidget(d->hsSelector,  1, 0, 1, 2);
    grid->addWidget(d->vSelector,   1, 2, 1, 1);
    grid->addWidget(hbox,           2, 0, 1, 3);
    grid->addWidget(d->clearButton, 3, 0, 1, 3);
    grid->addWidget(hbox2,          4, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged()));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->results, SIGNAL(valueChanged(int)),
            this, SLOT(slotResultsChanged()));

    connect(d->clearButton, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotClear()));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(const QImage&)),
            this, SLOT(slotSketchChanged(const QImage&)));

    // ---------------------------------------------------------------

    slotVChanged();
    d->sketchWidget->setPenWidth(d->penSize->value());
}

FuzzySearchView::~FuzzySearchView()
{
    writeConfig();
    delete d;
}

QString FuzzySearchView::currentHaarSearchName()
{
    return QString("_Current_Haar_Search_");
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

void FuzzySearchView::slotResultsChanged()
{
    slotSketchChanged(d->sketchWidget->sketchImage());
}

void FuzzySearchView::slotSketchChanged(const QImage& img)
{
    // We query database here

    HaarIface haarIface;
    //QList<qlonglong> list = haarIface.bestMatchesForImage(img, d->results->value(), HaarIface::HanddrawnSketch);
    //DDebug() << "Sketch Fuzzy Search Results: " << list << endl;

    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "signature"); // we pass a signature
    writer.writeAttribute("numberofresults", QString::number(d->results->value()));
    writer.writeAttribute("sketchtype", "handdrawn");
    writer.writeValue(haarIface.signatureAsText(img));
    writer.finishField();
    writer.finishGroup();

    SAlbum* album = AlbumManager::instance()->createSAlbum(currentHaarSearchName(), DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(album);
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
