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

#include <QImage>
#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>

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
#include <khbox.h>
#include <kinputdialog.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "haariface.h"
#include "searchxml.h"
#include "searchtextbar.h"
#include "sketchwidget.h"
#include "fuzzysearchfolderview.h"
#include "fuzzysearchview.h"
#include "fuzzysearchview.moc"

namespace Digikam
{

class FuzzySearchViewPriv
{

public:

    FuzzySearchViewPriv()
    {
        sketchWidget          = 0;
        hsSelector            = 0;
        vSelector             = 0;
        penSize               = 0;
        results               = 0;
        resetButton           = 0;
        saveButton            = 0;
        nameEdit              = 0;
        searchFuzzyBar        = 0;
        fuzzySearchFolderView = 0;
    }

    QPushButton            *resetButton;
    QPushButton            *saveButton;

    QSpinBox               *penSize;
    QSpinBox               *results;

    KLineEdit              *nameEdit;

    KHueSaturationSelector *hsSelector;

    KColorValueSelector    *vSelector;

    SearchTextBar          *searchFuzzyBar;

    FuzzySearchFolderView  *fuzzySearchFolderView;

    SketchWidget           *sketchWidget;
};

FuzzySearchView::FuzzySearchView(QWidget *parent)
               : QWidget(parent)
{
    d = new FuzzySearchViewPriv;
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vlay = new QVBoxLayout(this);
    QFrame *panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout *grid = new QGridLayout(panel);

    // ---------------------------------------------------------------

    QWidget *box       = new QWidget(panel);
    QVBoxLayout *vlay2 = new QVBoxLayout(box);
    KHBox *drawingBox  = new KHBox(box);
    d->sketchWidget    = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    vlay2->addStretch(10);
    vlay2->addWidget(drawingBox, 0, Qt::AlignCenter);
    vlay2->addStretch(10);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    d->hsSelector = new KHueSaturationSelector(panel);
    d->vSelector  = new KColorValueSelector(panel);
    d->hsSelector->setMinimumSize(200, 142);
    d->vSelector->setMinimumSize(26, 142);
    d->hsSelector->setChooserMode(ChooserValue);
    d->vSelector->setChooserMode(ChooserValue);

    QString tips(i18n("<p>Set here the brush color used to draw sketch."));
    d->hsSelector->setWhatsThis(tips);
    d->vSelector->setWhatsThis(tips);

    // ---------------------------------------------------------------

    KHBox *hbox          = new KHBox(panel);
    QLabel *brushLabel   = new QLabel(i18n("Brush size:"), hbox);
    d->penSize           = new QSpinBox(hbox);
    d->penSize->setRange(1, 40);
    d->penSize->setSingleStep(1);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("<p>Set here the brush size in pixels used to draw sketch."));

    QLabel *resultsLabel = new QLabel(i18n("Results:"), hbox);
    d->results           = new QSpinBox(hbox);
    d->results->setRange(1, 50);
    d->results->setSingleStep(1);
    d->results->setValue(10);
    d->results->setWhatsThis(i18n("<p>Set here the number of items to find."));

    hbox->setStretchFactor(brushLabel, 10);
    hbox->setStretchFactor(resultsLabel, 10);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox *hbox2 = new KHBox(panel);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    d->resetButton = new QPushButton(hbox2);
    d->resetButton->setIcon(SmallIcon("document-revert"));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("<p>Use this button to clear sketch contents."));

    d->nameEdit    = new KLineEdit(hbox2);
    d->nameEdit->setClearButtonShown(true);
    d->nameEdit->setWhatsThis(i18n("<p>Enter the name of the current fuzzy search to save in the "
                                   "\"My Fuzzy Searches\" view"));

    d->saveButton  = new QPushButton(hbox2);
    d->saveButton->setIcon(SmallIcon("document-save"));
    d->saveButton->setEnabled(false);
    d->saveButton->setToolTip(i18n("Save current search to a new virtual Album"));
    d->saveButton->setWhatsThis(i18n("<p>If you press this button, current "
                                     "fuzzy search will be saved to a new search "
                                     "virtual Album using name "
                                     "set on the left side."));

    // ---------------------------------------------------------------

    grid->addWidget(box,            0, 0, 1, 3);
    grid->addWidget(d->hsSelector,  1, 0, 1, 2);
    grid->addWidget(d->vSelector,   1, 2, 1, 1);
    grid->addWidget(hbox,           2, 0, 1, 3);
    grid->addWidget(hbox2,          3, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->fuzzySearchFolderView = new FuzzySearchFolderView(this);
    d->searchFuzzyBar        = new SearchTextBar(this, "FuzzySearchViewSearchFuzzyBar");

    // ---------------------------------------------------------------

    vlay->addWidget(panel);
    vlay->addWidget(d->fuzzySearchFolderView);
    vlay->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                  QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlay->addWidget(d->searchFuzzyBar);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    // ---------------------------------------------------------------

    connect(d->fuzzySearchFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchFuzzyBar, SLOT(slotSearchResult(bool)));

    connect(d->searchFuzzyBar, SIGNAL(textChanged(const QString&)),
            d->fuzzySearchFolderView, SLOT(slotTextSearchFilterChanged(const QString&)));

    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged()));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->results, SIGNAL(valueChanged(int)),
            this, SLOT(slotDirty()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotClear()));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(const QImage&)),
            this, SLOT(slotDirty()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveSelection()));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditConditions()));

    // ---------------------------------------------------------------

    readConfig();
    slotCheckNameEditConditions();
}

FuzzySearchView::~FuzzySearchView()
{
    writeConfig();
    delete d;
}

FuzzySearchFolderView* FuzzySearchView::folderView() const
{
    return d->fuzzySearchFolderView;
}

SearchTextBar* FuzzySearchView::searchBar() const
{
    return d->searchFuzzyBar;
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
    int hue      = d->vSelector->hue();
    int sat      = d->vSelector->saturation();
    int val      = d->vSelector->value();
    d->hsSelector->setColorValue(val);
    d->hsSelector->updateContents();
    d->hsSelector->repaint();
    QColor color = QColor::fromHsv(hue, sat, val);

    d->sketchWidget->setPenColor(color);
}

void FuzzySearchView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));

    d->penSize->setValue(group.readEntry("Pen Size", 10));
    d->results->setValue(group.readEntry("Result items", 10));
    d->hsSelector->setXValue(group.readEntry("Pen Hue", 180));
    d->hsSelector->setYValue(group.readEntry("Pen Saturation", 128));
    d->vSelector->setValue(group.readEntry("Pen Value", 255));
    d->hsSelector->updateContents();
    slotHSChanged(d->hsSelector->xValue(), d->hsSelector->yValue());
    d->sketchWidget->setPenWidth(d->penSize->value());
}

void FuzzySearchView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));
    group.writeEntry("Pen Size",       d->penSize->value());
    group.writeEntry("Result items",   d->results->value());
    group.writeEntry("Pen Hue",        d->hsSelector->xValue());
    group.writeEntry("Pen Saturation", d->hsSelector->yValue());
    group.writeEntry("Pen Value",      d->vSelector->value());
    group.sync();
}

void FuzzySearchView::setActive(bool val)
{
    if (d->fuzzySearchFolderView->selectedItem()) 
    {
        d->fuzzySearchFolderView->setActive(val);
    }
    else if (val)
    {
        AlbumList sList = AlbumManager::instance()->allSAlbums();
        for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
        {
            SAlbum* salbum = (SAlbum*)(*it);
            if (salbum->title() == d->fuzzySearchFolderView->currentFuzzySearchName())
                AlbumManager::instance()->setCurrentAlbum(salbum);
        }
    }
}

void FuzzySearchView::slotSaveSelection()
{
    QString name = d->nameEdit->text();
    if (!checkName(name))
        return;

    createNewFuzzySearchAlbum(name);
}

void FuzzySearchView::slotDirty()
{
    slotCheckNameEditConditions();
    createNewFuzzySearchAlbum(FuzzySearchFolderView::currentFuzzySearchName());
}

void FuzzySearchView::createNewFuzzySearchAlbum(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (d->sketchWidget->isClear())
        return;

    // We query database here

    HaarIface haarIface;
    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "signature");         // we pass a signature
    writer.writeAttribute("numberofresults", QString::number(d->results->value()));
    writer.writeAttribute("sketchtype", "handdrawn");
    writer.writeValue(haarIface.signatureAsText(d->sketchWidget->sketchImage()));
    writer.finishField();
    writer.finishGroup();

    SAlbum* album = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(album);
}

void FuzzySearchView::slotAlbumSelected(SAlbum* salbum)
{
    slotClear();

    if (!salbum) 
        return;

    // NOTE: There is nothing to display in scketch widget. Database do not store the scketch image.

    AlbumManager::instance()->setCurrentAlbum(salbum);
}

void FuzzySearchView::slotClear()
{
    d->sketchWidget->slotClear();
    slotCheckNameEditConditions();
    AlbumManager::instance()->setCurrentAlbum(0);
}

bool FuzzySearchView::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked) 
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool FuzzySearchView::checkAlbum(const QString& name) const
{
    AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::Iterator it = list.begin() ; it != list.end() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void FuzzySearchView::slotCheckNameEditConditions()
{
    if (!d->sketchWidget->isClear())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
            d->saveButton->setEnabled(true);
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveButton->setEnabled(false);
    }
}

void FuzzySearchView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)").arg(oldName),
                                          i18n("Enter new album name:"),
                                          oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

}  // NameSpace Digikam
