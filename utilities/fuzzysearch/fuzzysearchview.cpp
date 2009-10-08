/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "fuzzysearchview.h"
#include "fuzzysearchview.moc"

// Qt includes

#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTime>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcolorvalueselector.h>
#include <kconfig.h>
#include <kdialog.h>
#include <khbox.h>
#include <khuesaturationselect.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <kvbox.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "ddragobjects.h"
#include "imageinfo.h"
#include "haariface.h"
#include "searchxml.h"
#include "searchtextbar.h"
#include "sketchwidget.h"
#include "imagelister.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "findduplicatesview.h"
#include "fuzzysearchfolderview.h"

namespace Digikam
{

class FuzzySearchViewPriv
{

public:

    enum FuzzySearchTab
    {
        SIMILARS=0,
        SKETCH,
        DUPLICATES
    };

    FuzzySearchViewPriv()
    {
        sketchWidget          = 0;
        hsSelector            = 0;
        vSelector             = 0;
        penSize               = 0;
        resultsSketch         = 0;
        levelImage            = 0;
        resetButton           = 0;
        saveBtnSketch         = 0;
        undoBtnSketch         = 0;
        redoBtnSketch         = 0;
        saveBtnImage          = 0;
        nameEditSketch        = 0;
        nameEditImage         = 0;
        searchFuzzyBar        = 0;
        fuzzySearchFolderView = 0;
        tabWidget             = 0;
        thumbLoadThread       = 0;
        imageSAlbum           = 0;
        sketchSAlbum          = 0;
        folderView            = 0;
        timerSketch           = 0;
        timerImage            = 0;
        findDuplicatesPanel   = 0;
        active                = false;
        fingerprintsChecked   = false;
    }

    bool                    active;
    bool                    fingerprintsChecked;

    QColor                  selColor;

    QToolButton            *resetButton;
    QToolButton            *saveBtnSketch;
    QToolButton            *undoBtnSketch;
    QToolButton            *redoBtnSketch;
    QToolButton            *saveBtnImage;

    QSpinBox               *penSize;
    QSpinBox               *resultsSketch;
    QSpinBox               *levelImage;

    QLabel                 *imageWidget;

    QTimer                 *timerSketch;
    QTimer                 *timerImage;

    KVBox                  *folderView;

    KLineEdit              *nameEditSketch;
    KLineEdit              *nameEditImage;

    KTabWidget             *tabWidget;

    KHueSaturationSelector *hsSelector;

    KColorValueSelector    *vSelector;

    KSqueezedTextLabel     *labelFile;
    KSqueezedTextLabel     *labelFolder;

    ImageInfo               imageInfo;

    SearchTextBar          *searchFuzzyBar;

    FuzzySearchFolderView  *fuzzySearchFolderView;

    SketchWidget           *sketchWidget;

    ThumbnailLoadThread    *thumbLoadThread;

    FindDuplicatesView     *findDuplicatesPanel;

    AlbumPointer<SAlbum>    imageSAlbum;
    AlbumPointer<SAlbum>    sketchSAlbum;
};

FuzzySearchView::FuzzySearchView(QWidget *parent)
               : QScrollArea(parent), d(new FuzzySearchViewPriv)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);
    viewport()->setAutoFillBackground(false);
    viewport()->setAcceptDrops(true);

    QVBoxLayout *vlay    = new QVBoxLayout(panel);
    d->tabWidget         = new KTabWidget(panel);

    // ---------------------------------------------------------------
    // Find Similar Images Panel

    QWidget *imagePanel = new QWidget(panel);
    QGridLayout *grid   = new QGridLayout(imagePanel);
    QWidget *box2       = new QWidget(imagePanel);
    QVBoxLayout *vlay3  = new QVBoxLayout(box2);
    KHBox *imageBox     = new KHBox(box2);
    d->imageWidget      = new QLabel(imageBox);
    d->imageWidget->setFixedSize(256, 256);
    d->imageWidget->setText(i18n("<p>Drag & drop an image here<br/>to perform similar<br/>items search.</p>"
                                 "<p>You can also use the context menu<br/> when browsing through your images.</p>"));
    d->imageWidget->setAlignment(Qt::AlignCenter);
    imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    imageBox->setLineWidth(1);

    vlay3->addStretch(10);
    vlay3->addWidget(imageBox, 0, Qt::AlignCenter);
    vlay3->addStretch(10);
    vlay3->setMargin(0);
    vlay3->setSpacing(0);

    // ---------------------------------------------------------------

    QLabel *file   = new QLabel(i18n("<b>File</b>:"), imagePanel);
    d->labelFile   = new KSqueezedTextLabel(0, imagePanel);
    QLabel *folder = new QLabel(i18n("<b>Folder</b>:"), imagePanel);
    d->labelFolder = new KSqueezedTextLabel(0, imagePanel);
    int hgt        = fontMetrics().height()-2;
    file->setMaximumHeight(hgt);
    folder->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);

    // ---------------------------------------------------------------

    KHBox *hbox3          = new KHBox(imagePanel);
    QLabel *resultsLabel2 = new QLabel(i18n("Threshold:"), hbox3);
    d->levelImage         = new QSpinBox(hbox3);
    d->levelImage->setSuffix(QChar('%'));
    d->levelImage->setRange(1, 100);
    d->levelImage->setSingleStep(1);
    d->levelImage->setValue(90);
    d->levelImage->setWhatsThis(i18n("Select here the approximate threshold "
                                     "value, as a percentage. "
                                     "This value is used by the algorithm to distinguish two "
                                     "similar images. The default value is 90."));

    hbox3->setStretchFactor(resultsLabel2, 10);
    hbox3->setMargin(0);
    hbox3->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox *hbox4     = new KHBox(imagePanel);
    hbox4->setMargin(0);
    hbox4->setSpacing(KDialog::spacingHint());

    d->nameEditImage = new KLineEdit(hbox4);
    d->nameEditImage->setClearButtonShown(true);
    d->nameEditImage->setWhatsThis(i18n("Enter the name of the current similar image search to save in the "
                                        "\"My Fuzzy Searches\" view."));

    d->saveBtnImage  = new QToolButton(hbox4);
    d->saveBtnImage->setIcon(SmallIcon("document-save"));
    d->saveBtnImage->setEnabled(false);
    d->saveBtnImage->setToolTip(i18n("Save current similar image search to a new virtual Album"));
    d->saveBtnImage->setWhatsThis(i18n("If you press this button, the current "
                                       "similar image search will be saved to a new search "
                                       "virtual album using name "
                                       "set on the left side."));

    // ---------------------------------------------------------------

    grid->addWidget(box2,           0, 0, 1, 3);
    grid->addWidget(file,           1, 0, 1, 1);
    grid->addWidget(d->labelFile,   1, 1, 1, 1);
    grid->addWidget(folder,         2, 0, 1, 1);
    grid->addWidget(d->labelFolder, 2, 1, 1, 1);
    grid->addWidget(hbox3,          3, 0, 1, 3);
    grid->addWidget(hbox4,          4, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------
    // Find by Sketch Panel

    QWidget *sketchPanel = new QWidget(panel);
    QGridLayout *grid2   = new QGridLayout(sketchPanel);
    QWidget *box         = new QWidget(sketchPanel);
    QVBoxLayout *vlay2   = new QVBoxLayout(box);
    KHBox *drawingBox    = new KHBox(box);
    d->sketchWidget      = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    vlay2->addStretch(10);
    vlay2->addWidget(drawingBox, 0, Qt::AlignCenter);
    vlay2->addStretch(10);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    QString tips(i18n("Set here the brush color used to draw sketch."));

    d->hsSelector = new KHueSaturationSelector(sketchPanel);
    d->hsSelector->setMinimumSize(200, 96);
    d->hsSelector->setChooserMode(ChooserValue);
    d->hsSelector->setColorValue(255);
    d->hsSelector->setWhatsThis(tips);

    d->vSelector  = new KColorValueSelector(sketchPanel);
    d->vSelector->setMinimumSize(26, 96);
    d->vSelector->setChooserMode(ChooserValue);
    d->vSelector->setIndent(false);
    d->vSelector->setWhatsThis(tips);

    // ---------------------------------------------------------------

    KHBox *hbox        = new KHBox(sketchPanel);

    d->undoBtnSketch   = new QToolButton(hbox);
    d->undoBtnSketch->setAutoRepeat(true);
    d->undoBtnSketch->setIcon(SmallIcon("edit-undo"));
    d->undoBtnSketch->setToolTip(i18n("Undo last draw on sketch"));
    d->undoBtnSketch->setWhatsThis(i18n("Use this button to undo last drawing action on sketch."));
    d->undoBtnSketch->setEnabled(false);

    d->redoBtnSketch   = new QToolButton(hbox);
    d->redoBtnSketch->setAutoRepeat(true);
    d->redoBtnSketch->setIcon(SmallIcon("edit-redo"));
    d->redoBtnSketch->setToolTip(i18n("Redo last draw on sketch"));
    d->redoBtnSketch->setWhatsThis(i18n("Use this button to redo last drawing action on sketch."));
    d->redoBtnSketch->setEnabled(false);

    QLabel *brushLabel = new QLabel(i18n("Pen:"), hbox);
    d->penSize         = new QSpinBox(hbox);
    d->penSize->setRange(1, 40);
    d->penSize->setSingleStep(1);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("Set here the brush size in pixels used to draw sketch."));

    QLabel *resultsLabel = new QLabel(i18n("Items:"), hbox);
    d->resultsSketch     = new QSpinBox(hbox);
    d->resultsSketch->setRange(1, 50);
    d->resultsSketch->setSingleStep(1);
    d->resultsSketch->setValue(10);
    d->resultsSketch->setWhatsThis(i18n("Set here the number of items to find using sketch."));

    hbox->setStretchFactor(brushLabel, 10);
    hbox->setStretchFactor(resultsLabel, 10);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox *hbox2      = new KHBox(sketchPanel);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    d->resetButton    = new QToolButton(hbox2);
    d->resetButton->setIcon(SmallIcon("document-revert"));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("Use this button to clear sketch contents."));

    d->nameEditSketch = new KLineEdit(hbox2);
    d->nameEditSketch->setClearButtonShown(true);
    d->nameEditSketch->setWhatsThis(i18n("Enter the name of the current sketch search to save in the "
                                         "\"My Fuzzy Searches\" view."));

    d->saveBtnSketch  = new QToolButton(hbox2);
    d->saveBtnSketch->setIcon(SmallIcon("document-save"));
    d->saveBtnSketch->setEnabled(false);
    d->saveBtnSketch->setToolTip(i18n("Save current sketch search to a new virtual Album"));
    d->saveBtnSketch->setWhatsThis(i18n("If you press this button, the current sketch "
                                        "fuzzy search will be saved to a new search "
                                        "virtual album using the name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    grid2->addWidget(box,            0, 0, 1, 3);
    grid2->addWidget(d->hsSelector,  1, 0, 1, 2);
    grid2->addWidget(d->vSelector,   1, 2, 1, 1);
    grid2->addWidget(hbox,           2, 0, 1, 3);
    grid2->addWidget(hbox2,          3, 0, 1, 3);
    grid2->setRowStretch(5, 10);
    grid2->setColumnStretch(1, 10);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------
    // Find Duplicates Panel

    d->findDuplicatesPanel = new FindDuplicatesView(panel);

    d->tabWidget->insertTab(FuzzySearchViewPriv::SIMILARS,   imagePanel,             i18n("Image"));
    d->tabWidget->insertTab(FuzzySearchViewPriv::SKETCH,     sketchPanel,            i18n("Sketch"));
    d->tabWidget->insertTab(FuzzySearchViewPriv::DUPLICATES, d->findDuplicatesPanel, i18n("Duplicates"));

    // ---------------------------------------------------------------

    d->folderView            = new KVBox(panel);
    d->fuzzySearchFolderView = new FuzzySearchFolderView(d->folderView);
    d->searchFuzzyBar        = new SearchTextBar(d->folderView, "FuzzySearchViewSearchFuzzyBar");
    d->folderView->setSpacing(KDialog::spacingHint());
    d->folderView->setMargin(0);

    // ---------------------------------------------------------------

    vlay->addWidget(d->tabWidget);
    vlay->addWidget(d->folderView);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    readConfig();

    // ---------------------------------------------------------------

    connect(d->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchFuzzyBar, SLOT(slotSearchResult(bool)));

    connect(d->searchFuzzyBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->fuzzySearchFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged(int)));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->resultsSketch, SIGNAL(valueChanged(int)),
            this, SLOT(slotDirtySketch()));

    connect(d->levelImage, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelImageChanged()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotClearSketch()));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(const QImage&)),
            this, SLOT(slotDirtySketch()));

    connect(d->sketchWidget, SIGNAL(signalUndoRedoStateChanged(bool, bool)),
            this, SLOT(slotUndoRedoStateChanged(bool, bool)));

    connect(d->undoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotUndo()));

    connect(d->redoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotRedo()));

    connect(d->saveBtnSketch, SIGNAL(clicked()),
            this, SLOT(slotSaveSketchSAlbum()));

    connect(d->saveBtnImage, SIGNAL(clicked()),
            this, SLOT(slotSaveImageSAlbum()));

    connect(d->nameEditSketch, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditSketchConditions()));

    connect(d->nameEditSketch, SIGNAL(returnPressed(const QString&)),
            d->saveBtnSketch, SLOT(animateClick()));

    connect(d->nameEditImage, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditImageConditions()));

    connect(d->nameEditImage, SIGNAL(returnPressed(const QString&)),
            d->saveBtnImage, SLOT(animateClick()));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    connect(d->findDuplicatesPanel, SIGNAL(signalUpdateFingerPrints()),
            this, SIGNAL(signalUpdateFingerPrints()));

    // ---------------------------------------------------------------

    slotCheckNameEditSketchConditions();
    slotCheckNameEditImageConditions();
}

FuzzySearchView::~FuzzySearchView()
{
    writeConfig();
    delete d->timerSketch;
    delete d->timerImage;
    delete d;
}

// Common methods ----------------------------------------------------------------------

void FuzzySearchView::newDuplicatesSearch(Album* album)
{
    if (album)
    {
        if (album->type() == Album::PHYSICAL)
            d->findDuplicatesPanel->slotSetSelectedAlbum(album);
        else if (album->type() == Album::TAG)
            d->findDuplicatesPanel->slotSetSelectedTag(album);
    }
    d->tabWidget->setCurrentIndex(FuzzySearchViewPriv::DUPLICATES);
}

void FuzzySearchView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));

    d->tabWidget->setCurrentIndex(group.readEntry("FuzzySearch Tab",
                                  (int)FuzzySearchViewPriv::SKETCH));
    d->penSize->setValue(group.readEntry("Pen Sketch Size", 10));
    d->resultsSketch->setValue(group.readEntry("Result Sketch items", 10));
    d->hsSelector->setHue(group.readEntry("Pen Sketch Hue", 180));
    d->hsSelector->setSaturation(group.readEntry("Pen Sketch Saturation", 128));
    d->vSelector->setValue(group.readEntry("Pen Sketch Value", 255));
    d->levelImage->setValue(group.readEntry("Similars Threshold", 90));
    d->hsSelector->updateContents();

    QColor col;
    col.setHsv(d->hsSelector->hue(),
               d->hsSelector->saturation(),
               d->vSelector->value());
    setColor(col);

    d->sketchWidget->setPenWidth(d->penSize->value());
}

void FuzzySearchView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));
    group.writeEntry("FuzzySearch Tab",        d->tabWidget->currentIndex());
    group.writeEntry("Pen Sketch Size",        d->penSize->value());
    group.writeEntry("Result Sketch items",    d->resultsSketch->value());
    group.writeEntry("Pen Sketch Hue",         d->hsSelector->hue());
    group.writeEntry("Pen Sketch Saturation",  d->hsSelector->saturation());
    group.writeEntry("Pen Sketch Value",       d->vSelector->value());
    group.writeEntry("Similars Threshold",     d->levelImage->value());
    group.sync();
}

FuzzySearchFolderView* FuzzySearchView::folderView() const
{
    return d->fuzzySearchFolderView;
}

SearchTextBar* FuzzySearchView::searchBar() const
{
    return d->searchFuzzyBar;
}

void FuzzySearchView::setActive(bool val)
{
    d->active = val;

    // at first occasion, warn if no fingerprints are available
    if (val && !d->fingerprintsChecked && isVisible())
    {
        if (!DatabaseAccess().db()->hasHaarFingerprints())
        {
            QString msg = i18n("Image fingerprints have not yet been generated for your collection. "
                               "The Fuzzy Search Tools will not be operational "
                               "without pre-generated fingerprints.\n"
                               "Do you want to build fingerprints now?\n"
                               "Note: This process can take a while. You can run it "
                               "any time later using 'Tools/Rebuild all Fingerprints'");
            int result = KMessageBox::questionYesNo(this, msg, i18n("No Fingerprints"));

            if (result == KMessageBox::Yes)
            {
                emit signalUpdateFingerPrints();
            }
        }
        d->fingerprintsChecked = true;
    }

    int tab = d->tabWidget->currentIndex();

    switch(tab)
    {
        case FuzzySearchViewPriv::SIMILARS:
        {
            if (d->fuzzySearchFolderView->selectedItem())
                d->fuzzySearchFolderView->setActive(val);
            break;
        }
        case FuzzySearchViewPriv::SKETCH:
        {
            if (d->fuzzySearchFolderView->selectedItem())
                d->fuzzySearchFolderView->setActive(val);
            break;
        }
        default:  // DUPLICATES
        {
            break;
        }
    }

    if (val)
        slotTabChanged(tab);
}

void FuzzySearchView::slotTabChanged(int tab)
{
    switch(tab)
    {
        case FuzzySearchViewPriv::SIMILARS:
        {
            AlbumManager::instance()->setCurrentAlbum(d->imageSAlbum);
            d->folderView->setVisible(true);
            break;
        }
        case FuzzySearchViewPriv::SKETCH:
        {
            AlbumManager::instance()->setCurrentAlbum(d->sketchSAlbum);
            d->folderView->setVisible(true);
            break;
        }
        default:  // DUPLICATES
        {
            AlbumManager::instance()->setCurrentAlbum(d->findDuplicatesPanel->currentFindDuplicatesAlbum());
            d->folderView->setVisible(false);
            break;
        }
    }
}

void FuzzySearchView::slotAlbumSelected(SAlbum* salbum)
{
    if (!salbum || !salbum->isHaarSearch())
        return;

    if (!d->active)
        return;

    AlbumManager::instance()->setCurrentAlbum(salbum);

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type             = reader.attributes().value("type");
    QStringRef numResultsString = reader.attributes().value("numberofresults");
    QStringRef thresholdString  = reader.attributes().value("threshold");
    QStringRef sketchTypeString = reader.attributes().value("sketchtype");

    if (type == "imageid")
    {
        setCurrentImage(reader.valueToLongLong());
        d->imageSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::SIMILARS);
    }
    else if (type == "signature")
    {
        d->sketchSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::SKETCH);
        if (reader.readToStartOfElement("SketchImage"))
            d->sketchWidget->setSketchImageFromXML(reader);
    }
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

void FuzzySearchView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    if (salbum->title() == FuzzySearchFolderView::currentFuzzySketchSearchName() ||
        salbum->title() == FuzzySearchFolderView::currentFuzzyImageSearchName())
        return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)",oldName),
                                         i18n("Enter new album name:"),
                                         oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

// Sketch Searches methods -----------------------------------------------------------------------

void FuzzySearchView::slotHSChanged(int h, int s)
{
    QColor color;

    int val = d->selColor.value();

    color.setHsv(h, s, val);
    setColor(color);
}

void FuzzySearchView::slotVChanged(int v)
{
    QColor color;

    int hue = d->selColor.hue();
    int sat = d->selColor.saturation();

    color.setHsv(hue, sat, v);
    setColor(color);
}

void FuzzySearchView::setColor(QColor c)
{
    if (c.isValid())
    {
        d->selColor = c;

        // set values
        d->hsSelector->setValues(c.hue(), c.saturation());
        d->vSelector->setValue(c.value());

        // set colors
        d->hsSelector->blockSignals(true);
        d->hsSelector->setHue(c.hue());
        d->hsSelector->setSaturation(c.saturation());
        d->hsSelector->setColorValue(c.value());
        d->hsSelector->updateContents();
        d->hsSelector->blockSignals(false);
        d->hsSelector->repaint();

        d->vSelector->blockSignals(true);
        d->vSelector->setHue(c.hue());
        d->vSelector->setSaturation(c.saturation());
        d->vSelector->setColorValue(c.value());
        d->vSelector->updateContents();
        d->vSelector->blockSignals(false);
        d->vSelector->repaint();

        d->sketchWidget->setPenColor(c);
    }
}

void FuzzySearchView::slotUndoRedoStateChanged(bool hasUndo, bool hasRedo)
{
    d->undoBtnSketch->setEnabled(hasUndo);
    d->redoBtnSketch->setEnabled(hasRedo);
}

void FuzzySearchView::slotSaveSketchSAlbum()
{
    QString name = d->nameEditSketch->text();
    if (!checkName(name))
        return;

    createNewFuzzySearchAlbumFromSketch(name);
}

void FuzzySearchView::slotDirtySketch()
{
    if (d->timerSketch)
    {
       d->timerSketch->stop();
       delete d->timerSketch;
    }

    d->timerSketch = new QTimer( this );
    connect( d->timerSketch, SIGNAL(timeout()),
             this, SLOT(slotTimerSketchDone()) );
    d->timerSketch->setSingleShot(true);
    d->timerSketch->start(500);
}

void FuzzySearchView::slotTimerSketchDone()
{
    slotCheckNameEditSketchConditions();
    createNewFuzzySearchAlbumFromSketch(FuzzySearchFolderView::currentFuzzySketchSearchName());
}

void FuzzySearchView::createNewFuzzySearchAlbumFromSketch(const QString& name)
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
    writer.writeAttribute("numberofresults", QString::number(d->resultsSketch->value()));
    writer.writeAttribute("sketchtype", "handdrawn");
    writer.writeValue(haarIface.signatureAsText(d->sketchWidget->sketchImage()));
    d->sketchWidget->sketchImageToXML(writer);
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->sketchSAlbum = salbum;
}

void FuzzySearchView::slotClearSketch()
{
    d->sketchWidget->slotClear();
    slotCheckNameEditSketchConditions();
    AlbumManager::instance()->setCurrentAlbum(0);
}

void FuzzySearchView::slotCheckNameEditSketchConditions()
{
    if (!d->sketchWidget->isClear())
    {
        d->nameEditSketch->setEnabled(true);

        if (!d->nameEditSketch->text().isEmpty())
            d->saveBtnSketch->setEnabled(true);
    }
    else
    {
        d->nameEditSketch->setEnabled(false);
        d->saveBtnSketch->setEnabled(false);
    }
}

// Similars Searches methods ----------------------------------------------------------------------

void FuzzySearchView::dragEnterEvent(QDragEnterEvent *e)
{
    if (DItemDrag::canDecode(e->mimeData()))
        e->acceptProposedAction();
}

void FuzzySearchView::dropEvent(QDropEvent *e)
{
    if (DItemDrag::canDecode(e->mimeData()))
    {
        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (imageIDs.isEmpty())
            return;

        setCurrentImage(imageIDs.first());
        slotCheckNameEditImageConditions();
        createNewFuzzySearchAlbumFromImage(FuzzySearchFolderView::currentFuzzyImageSearchName());
        d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::SIMILARS);

        e->acceptProposedAction();
    }
}

void FuzzySearchView::slotLevelImageChanged()
{
    if (d->timerImage)
    {
       d->timerImage->stop();
       delete d->timerImage;
    }

    d->timerImage = new QTimer( this );
    connect( d->timerImage, SIGNAL(timeout()),
             this, SLOT(slotTimerImageDone()) );
    d->timerImage->setSingleShot(true);
    d->timerImage->start(500);
}

void FuzzySearchView::slotTimerImageDone()
{
    if (!d->imageInfo.isNull())
        setImageInfo(d->imageInfo);
}

void FuzzySearchView::setCurrentImage(qlonglong imageid)
{
    setCurrentImage(ImageInfo(imageid));
}

void FuzzySearchView::setCurrentImage(const ImageInfo& info)
{
    d->imageInfo = info;
    d->labelFile->setText(d->imageInfo.name());
    d->labelFolder->setText(d->imageInfo.fileUrl().directory());
    d->thumbLoadThread->find(d->imageInfo.fileUrl().toLocalFile());
}

void FuzzySearchView::setImageInfo(const ImageInfo& info)
{
    setCurrentImage(info);
    slotCheckNameEditImageConditions();
    createNewFuzzySearchAlbumFromImage(FuzzySearchFolderView::currentFuzzyImageSearchName());
    d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::SIMILARS);
}

void FuzzySearchView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    if (!d->imageInfo.isNull() && KUrl(desc.filePath) == d->imageInfo.fileUrl())
        d->imageWidget->setPixmap(pix.scaled(256, 256, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
}

void FuzzySearchView::createNewFuzzySearchAlbumFromImage(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (d->imageInfo.isNull())
        return;

    // We query database here

    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "imageid");
    writer.writeAttribute("threshold", QString::number(d->levelImage->value()/100.0));
    writer.writeAttribute("sketchtype", "scanned");
    writer.writeValue(d->imageInfo.id());
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->imageSAlbum = salbum;
}

void FuzzySearchView::slotCheckNameEditImageConditions()
{
    if (!d->imageInfo.isNull())
    {
        d->nameEditImage->setEnabled(true);

        if (!d->nameEditImage->text().isEmpty())
            d->saveBtnImage->setEnabled(true);
    }
    else
    {
        d->nameEditImage->setEnabled(false);
        d->saveBtnImage->setEnabled(false);
    }
}

void FuzzySearchView::slotSaveImageSAlbum()
{
    QString name = d->nameEditImage->text();
    if (!checkName(name))
        return;

    createNewFuzzySearchAlbumFromImage(name);
}

}  // namespace Digikam
