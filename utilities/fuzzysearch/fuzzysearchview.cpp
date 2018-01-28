/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QEvent>
#include <QDragEnterEvent>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "coredbalbuminfo.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumselectors.h"
#include "coredbaccess.h"
#include "ddragobjects.h"
#include "editablesearchtreeview.h"
#include "findduplicatesview.h"
#include "haariface.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "searchmodificationhelper.h"
#include "searchtextbar.h"
#include "coredbsearchxml.h"
#include "sketchwidget.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dhuesaturationselect.h"
#include "dcolorvalueselector.h"
#include "dexpanderbox.h"
#include "applicationsettings.h"
#include "drangebox.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

class FuzzySearchView::Private
{

public:

    enum FuzzySearchTab
    {
        DUPLICATES = 0,
        SIMILARS,
        SKETCH
    };

public:

    Private() :
        // initially be active to update sketch panel when the search list is restored
        active(false),
        fingerprintsChecked(false),
        resetButton(0),
        saveBtnSketch(0),
        undoBtnSketch(0),
        redoBtnSketch(0),
        saveBtnImage(0),
        penSize(0),
        resultsSketch(0),
        similarityRange(0),
        imageWidget(0),
        timerSketch(0),
        timerImage(0),
        folderView(0),
        nameEditSketch(0),
        nameEditImage(0),
        tabWidget(0),
        hsSelector(0),
        vSelector(0),
        labelFile(0),
        labelFolder(0),
        searchFuzzyBar(0),
        searchTreeView(0),
        sketchWidget(0),
        thumbLoadThread(0),
        findDuplicatesPanel(0),
        imageSAlbum(0),
        sketchSAlbum(0),
        fuzzySearchAlbumSelectors(0),
        sketchSearchAlbumSelectors(0),
        searchModel(0),
        searchModificationHelper(0),
        settings(0)
    {
    }

    static const QString      configTabEntry;
    static const QString      configPenSketchSizeEntry;
    static const QString      configResultSketchItemsEntry;
    static const QString      configPenSketchHueEntry;
    static const QString      configPenSketchSaturationEntry;
    static const QString      configPenSkethValueEntry;
    static const QString      configSimilarsThresholdEntry;
    static const QString      configSimilarsMaxThresholdEntry;

    bool                      active;
    bool                      fingerprintsChecked;

    QColor                    selColor;

    QToolButton*              resetButton;
    QToolButton*              saveBtnSketch;
    QToolButton*              undoBtnSketch;
    QToolButton*              redoBtnSketch;
    QToolButton*              saveBtnImage;

    QSpinBox*                 penSize;
    QSpinBox*                 resultsSketch;

    DIntRangeBox*             similarityRange;

    QLabel*                   imageWidget;

    QTimer*                   timerSketch;
    QTimer*                   timerImage;

    DVBox*                    folderView;

    QLineEdit*                nameEditSketch;
    QLineEdit*                nameEditImage;

    QTabWidget*               tabWidget;

    DHueSaturationSelector*   hsSelector;

    DColorValueSelector*      vSelector;

    DAdjustableLabel*         labelFile;
    DAdjustableLabel*         labelFolder;

    ImageInfo                 imageInfo;
    QUrl                      imageUrl;

    SearchTextBar*            searchFuzzyBar;

    EditableSearchTreeView*   searchTreeView;

    SketchWidget*             sketchWidget;

    ThumbnailLoadThread*      thumbLoadThread;

    FindDuplicatesView*       findDuplicatesPanel;

    AlbumPointer<SAlbum>      imageSAlbum;
    AlbumPointer<SAlbum>      sketchSAlbum;

    AlbumSelectors*           fuzzySearchAlbumSelectors;
    AlbumSelectors*           sketchSearchAlbumSelectors;

    SearchModel*              searchModel;
    SearchModificationHelper* searchModificationHelper;

    ApplicationSettings*      settings;
};

const QString FuzzySearchView::Private::configTabEntry(QLatin1String("FuzzySearch Tab"));
const QString FuzzySearchView::Private::configPenSketchSizeEntry(QLatin1String("Pen Sketch Size"));
const QString FuzzySearchView::Private::configResultSketchItemsEntry(QLatin1String("Result Sketch items"));
const QString FuzzySearchView::Private::configPenSketchHueEntry(QLatin1String("Pen Sketch Hue"));
const QString FuzzySearchView::Private::configPenSketchSaturationEntry(QLatin1String("Pen Sketch Saturation"));
const QString FuzzySearchView::Private::configPenSkethValueEntry(QLatin1String("Pen Sketch Value"));
const QString FuzzySearchView::Private::configSimilarsThresholdEntry(QLatin1String("Similars Threshold"));
const QString FuzzySearchView::Private::configSimilarsMaxThresholdEntry(QLatin1String("Similars Maximum Threshold"));

// --------------------------------------------------------

FuzzySearchView::FuzzySearchView(SearchModel* const searchModel,
                                 SearchModificationHelper* const searchModificationHelper,
                                 QWidget* const parent)
    : QScrollArea(parent),
      StateSavingObject(this),
      d(new Private)
{
    d->settings                 = ApplicationSettings::instance();

    const int spacing           = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->thumbLoadThread          = ThumbnailLoadThread::defaultThread();
    d->searchModel              = searchModel;
    d->searchModificationHelper = searchModificationHelper;

    setWidgetResizable(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // ---------------------------------------------------------------

    QWidget* const imagePanel  = setupFindSimilarPanel();
    QWidget* const sketchPanel = setupSketchPanel();
    d->findDuplicatesPanel     = new FindDuplicatesView();

    d->tabWidget               = new QTabWidget();
    d->tabWidget->insertTab(Private::DUPLICATES, d->findDuplicatesPanel, i18n("Duplicates"));
    d->tabWidget->insertTab(Private::SIMILARS,   imagePanel,             i18n("Image"));
    d->tabWidget->insertTab(Private::SKETCH,     sketchPanel,            i18n("Sketch"));

    // ---------------------------------------------------------------

    d->folderView     = new DVBox();
    d->searchTreeView = new EditableSearchTreeView(d->folderView, searchModel, searchModificationHelper);
    d->searchTreeView->filteredModel()->listHaarSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);

    d->searchFuzzyBar = new SearchTextBar(d->folderView, QLatin1String("FuzzySearchViewSearchFuzzyBar"));
    d->searchFuzzyBar->setModel(d->searchTreeView->filteredModel(),
                                AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchFuzzyBar->setFilterModel(d->searchTreeView->albumFilterModel());
    d->folderView->setContentsMargins(QMargins());
    d->folderView->setSpacing(spacing);

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout();
    mainLayout->addWidget(d->tabWidget);
    mainLayout->addWidget(d->folderView);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(0);
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);

    // ---------------------------------------------------------------

    d->timerSketch = new QTimer(this);
    d->timerSketch->setSingleShot(true);
    d->timerSketch->setInterval(500);

    d->timerImage  = new QTimer(this);
    d->timerImage->setSingleShot(true);
    d->timerImage->setInterval(500);

    // ---------------------------------------------------------------

    setupConnections();

    // ---------------------------------------------------------------

    slotCheckNameEditSketchConditions();
    slotCheckNameEditImageConditions();
}

QWidget* FuzzySearchView::setupFindSimilarPanel() const
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DHBox* const imageBox = new DHBox();
    d->imageWidget        = new QLabel(imageBox);
    d->imageWidget->setFixedSize(256, 256);
    d->imageWidget->setText(i18n("<p>Drag & drop an image here<br/>to perform similar<br/>items search.</p>"
                                 "<p>You can also use the context menu<br/> when browsing through your images.</p>"));
    d->imageWidget->setAlignment(Qt::AlignCenter);
    imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    imageBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QLabel* const file   = new QLabel(i18n("<b>File</b>:"));
    d->labelFile         = new DAdjustableLabel(0);
    QLabel* const folder = new QLabel(i18n("<b>Folder</b>:"));
    d->labelFolder       = new DAdjustableLabel(0);
    int hgt              = fontMetrics().height() - 2;
    file->setMaximumHeight(hgt);
    folder->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);

    // ---------------------------------------------------------------

    d->fuzzySearchAlbumSelectors = new AlbumSelectors(i18nc("@label", "Search in albums:"),
                                                      QLatin1String("Fuzzy Search View"),
                                                      0, AlbumSelectors::AlbumType::PhysAlbum);

    // ---------------------------------------------------------------

    QLabel* const resultsLabel = new QLabel(i18n("Similarity range:"));
    d->similarityRange = new DIntRangeBox();
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (d->settings)
    {
        d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
        d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                        d->settings->getDuplicatesSearchLastMaxSimilarity());
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(90, 100);
    }

    d->similarityRange->setWhatsThis(i18n("Select here the approximate similarity interval "
                                          "as a percentage. "));

    // ---------------------------------------------------------------

    DHBox* const saveBox = new DHBox();
    saveBox->setContentsMargins(QMargins());
    saveBox->setSpacing(spacing);

    d->nameEditImage = new QLineEdit(saveBox);
    d->nameEditImage->setClearButtonEnabled(true);
    d->nameEditImage->setWhatsThis(i18n("Enter the name of the current similar image search to save in the "
                                        "\"Similarity Searches\" view."));

    d->saveBtnImage  = new QToolButton(saveBox);
    d->saveBtnImage->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveBtnImage->setEnabled(false);
    d->saveBtnImage->setToolTip(i18n("Save current similar image search to a new virtual Album"));
    d->saveBtnImage->setWhatsThis(i18n("If you press this button, the current "
                                       "similar image search will be saved to a new search "
                                       "virtual album using name "
                                       "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget();
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(imageBox,                      0, 0, 1, 6);
    mainLayout->addWidget(file,                          1, 0, 1, 1);
    mainLayout->addWidget(d->labelFile,                  1, 1, 1, 5);
    mainLayout->addWidget(folder,                        2, 0, 1, 1);
    mainLayout->addWidget(d->labelFolder,                2, 1, 1, 5);
    mainLayout->addWidget(d->fuzzySearchAlbumSelectors,  3, 0, 1, -1);
    mainLayout->addWidget(resultsLabel,                  4, 0, 1, 1);
    mainLayout->addWidget(d->similarityRange,            4, 2, 1, 1);
    mainLayout->addWidget(saveBox,                       5, 0, 1, 6);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

QWidget* FuzzySearchView::setupSketchPanel() const
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DHBox* const drawingBox = new DHBox();
    d->sketchWidget         = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QString tooltip(i18n("Set here the brush color used to draw sketch."));

    d->hsSelector = new DHueSaturationSelector();
    d->hsSelector->setMinimumSize(200, 96);
    d->hsSelector->setChooserMode(ChooserValue);
    d->hsSelector->setColorValue(255);
    d->hsSelector->setWhatsThis(tooltip);

    d->vSelector  = new DColorValueSelector();
    d->vSelector->setMinimumSize(26, 96);
    d->vSelector->setChooserMode(ChooserValue);
    d->vSelector->setIndent(false);
    d->vSelector->setWhatsThis(tooltip);

    // ---------------------------------------------------------------

    d->undoBtnSketch   = new QToolButton();
    d->undoBtnSketch->setAutoRepeat(true);
    d->undoBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    d->undoBtnSketch->setToolTip(i18n("Undo last draw on sketch"));
    d->undoBtnSketch->setWhatsThis(i18n("Use this button to undo last drawing action on sketch."));
    d->undoBtnSketch->setEnabled(false);

    d->redoBtnSketch   = new QToolButton();
    d->redoBtnSketch->setAutoRepeat(true);
    d->redoBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("edit-redo")));
    d->redoBtnSketch->setToolTip(i18n("Redo last draw on sketch"));
    d->redoBtnSketch->setWhatsThis(i18n("Use this button to redo last drawing action on sketch."));
    d->redoBtnSketch->setEnabled(false);

    QLabel* const brushLabel = new QLabel(i18n("Pen:"));
    d->penSize               = new QSpinBox();
    d->penSize->setRange(1, 64);
    d->penSize->setSingleStep(1);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("Set here the brush size in pixels used to draw sketch."));

    QLabel* const resultsLabel = new QLabel(i18n("Items:"));
    d->resultsSketch           = new QSpinBox();
    d->resultsSketch->setRange(1, 50);
    d->resultsSketch->setSingleStep(1);
    d->resultsSketch->setValue(10);
    d->resultsSketch->setWhatsThis(i18n("Set here the number of items to find using sketch."));

    QGridLayout* const settingsLayout = new QGridLayout();
    settingsLayout->addWidget(d->undoBtnSketch, 0, 0);
    settingsLayout->addWidget(d->redoBtnSketch, 0, 1);
    settingsLayout->addWidget(brushLabel,       0, 2);
    settingsLayout->addWidget(d->penSize,       0, 3);
    settingsLayout->addWidget(resultsLabel,     0, 5);
    settingsLayout->addWidget(d->resultsSketch, 0, 6);
    settingsLayout->setColumnStretch(4, 10);
    settingsLayout->setContentsMargins(QMargins());
    settingsLayout->setSpacing(spacing);

    // ---------------------------------------------------------------

    d->sketchSearchAlbumSelectors = new AlbumSelectors(i18nc("@label",
                                                       "Search in albums:"),
                                                       QLatin1String("Sketch Search View"),
                                                       0, AlbumSelectors::AlbumType::PhysAlbum);

    // ---------------------------------------------------------------

    DHBox* const saveBox = new DHBox();
    saveBox->setContentsMargins(QMargins());
    saveBox->setSpacing(spacing);

    d->resetButton = new QToolButton(saveBox);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("Use this button to clear sketch contents."));

    d->nameEditSketch = new QLineEdit(saveBox);
    d->nameEditSketch->setClearButtonEnabled(true);
    d->nameEditSketch->setWhatsThis(i18n("Enter the name of the current sketch search to save in the "
                                         "\"Similarity Searches\" view."));

    d->saveBtnSketch = new QToolButton(saveBox);
    d->saveBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveBtnSketch->setEnabled(false);
    d->saveBtnSketch->setToolTip(i18n("Save current sketch search to a new virtual Album"));
    d->saveBtnSketch->setWhatsThis(i18n("If you press this button, the current sketch "
                                        "fuzzy search will be saved to a new search "
                                        "virtual album using the name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(drawingBox,                    0, 0, 1, 3);
    mainLayout->addWidget(d->hsSelector,                 1, 0, 1, 2);
    mainLayout->addWidget(d->vSelector,                  1, 2, 1, 1);
    mainLayout->addLayout(settingsLayout,                2, 0, 1, 3);
    mainLayout->addWidget(d->sketchSearchAlbumSelectors, 3, 0, 1, 3);
    mainLayout->addWidget(saveBox,                       4, 0, 1, 3);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

void FuzzySearchView::setupConnections()
{
    connect(d->settings, SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(d->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    connect(d->searchTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->hsSelector, SIGNAL(valueChanged(int,int)),
            this, SLOT(slotHSChanged(int,int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged(int)));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->resultsSketch, SIGNAL(valueChanged(int)),
            this, SLOT(slotDirtySketch()));

    connect(d->sketchSearchAlbumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotDirtySketch()));

    connect(d->fuzzySearchAlbumSelectors, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotFuzzyAlbumsChanged()));

    connect(d->similarityRange, SIGNAL(minChanged(int)),
            this, SLOT(slotMinLevelImageChanged(int)));

    connect(d->similarityRange, SIGNAL(maxChanged(int)),
            this, SLOT(slotMaxLevelImageChanged(int)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotClearSketch()));

    connect(d->sketchWidget, SIGNAL(signalPenSizeChanged(int)),
            d->penSize, SLOT(setValue(int)));

    connect(d->sketchWidget, SIGNAL(signalPenColorChanged(QColor)),
            this, SLOT(slotPenColorChanged(QColor)));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(QImage)),
            this, SLOT(slotDirtySketch()));

    connect(d->sketchWidget, SIGNAL(signalUndoRedoStateChanged(bool,bool)),
            this, SLOT(slotUndoRedoStateChanged(bool,bool)));

    connect(d->undoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotUndo()));

    connect(d->redoBtnSketch, SIGNAL(clicked()),
            d->sketchWidget, SLOT(slotRedo()));

    connect(d->saveBtnSketch, SIGNAL(clicked()),
            this, SLOT(slotSaveSketchSAlbum()));

    connect(d->saveBtnImage, SIGNAL(clicked()),
            this, SLOT(slotSaveImageSAlbum()));

    connect(d->nameEditSketch, SIGNAL(textChanged(QString)),
            this, SLOT(slotCheckNameEditSketchConditions()));

    connect(d->nameEditSketch, SIGNAL(returnPressed()),
            d->saveBtnSketch, SLOT(animateClick()));

    connect(d->nameEditImage, SIGNAL(textChanged(QString)),
            this, SLOT(slotCheckNameEditImageConditions()));

    connect(d->nameEditImage, SIGNAL(returnPressed()),
            d->saveBtnImage, SLOT(animateClick()));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(d->timerSketch, SIGNAL(timeout()),
             this, SLOT(slotTimerSketchDone()));

    connect(d->timerImage, SIGNAL(timeout()),
            this, SLOT(slotTimerImageDone()));

}

FuzzySearchView::~FuzzySearchView()
{
    delete d->timerSketch;
    delete d->timerImage;
    delete d;
}

SAlbum* FuzzySearchView::currentAlbum() const
{
    return d->searchTreeView->currentAlbum();
}

void FuzzySearchView::setCurrentAlbum(SAlbum* const album)
{
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << album);
}

void FuzzySearchView::newDuplicatesSearch(PAlbum* const album)
{
    if (album)
    {
        d->findDuplicatesPanel->slotSetSelectedAlbum(album);
    }

    d->tabWidget->setCurrentIndex(Private::DUPLICATES);
}

void FuzzySearchView::newDuplicatesSearch(QList<PAlbum*> const albums)
{
    if (!albums.isEmpty())
    {
        d->findDuplicatesPanel->slotSetSelectedAlbums(albums);
    }

    d->tabWidget->setCurrentIndex(Private::DUPLICATES);
}

void FuzzySearchView::newDuplicatesSearch(QList<TAlbum*> const albums)
{
    if (!albums.isEmpty())
    {
        d->findDuplicatesPanel->slotSetSelectedAlbums(albums);
    }

    d->tabWidget->setCurrentIndex(Private::DUPLICATES);
}

void FuzzySearchView::setConfigGroup(const KConfigGroup& group)
{
    StateSavingObject::setConfigGroup(group);
    d->searchTreeView->setConfigGroup(group);
}

void FuzzySearchView::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->tabWidget->setCurrentIndex(group.readEntry(entryName(d->configTabEntry),                    (int)Private::DUPLICATES));
    d->penSize->setValue(group.readEntry(entryName(d->configPenSketchSizeEntry),                   10));
    d->resultsSketch->setValue(group.readEntry(entryName(d->configResultSketchItemsEntry),         10));
    d->hsSelector->setHue(group.readEntry(entryName(d->configPenSketchHueEntry),                   180));
    d->hsSelector->setSaturation(group.readEntry(entryName(d->configPenSketchSaturationEntry),     128));
    d->vSelector->setValue(group.readEntry(entryName(d->configPenSkethValueEntry),                 255));
    d->similarityRange->setInterval(group.readEntry(entryName(d->configSimilarsThresholdEntry),    90),
                                    group.readEntry(entryName(d->configSimilarsMaxThresholdEntry), 100));
    d->hsSelector->updateContents();

    QColor col;
    col.setHsv(d->hsSelector->hue(),
               d->hsSelector->saturation(),
               d->vSelector->value());
    setColor(col);

    d->sketchWidget->setPenWidth(d->penSize->value());

    d->searchTreeView->loadState();
    d->fuzzySearchAlbumSelectors->loadState();
    d->sketchSearchAlbumSelectors->loadState();
}

void FuzzySearchView::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->configTabEntry),                  d->tabWidget->currentIndex());
    group.writeEntry(entryName(d->configPenSketchSizeEntry),        d->penSize->value());
    group.writeEntry(entryName(d->configResultSketchItemsEntry),    d->resultsSketch->value());
    group.writeEntry(entryName(d->configPenSketchHueEntry),         d->hsSelector->hue());
    group.writeEntry(entryName(d->configPenSketchSaturationEntry),  d->hsSelector->saturation());
    group.writeEntry(entryName(d->configPenSkethValueEntry),        d->vSelector->value());
    group.writeEntry(entryName(d->configSimilarsThresholdEntry),    d->similarityRange->minValue());
    group.writeEntry(entryName(d->configSimilarsMaxThresholdEntry), d->similarityRange->maxValue());
    d->searchTreeView->saveState();
    group.sync();
    d->fuzzySearchAlbumSelectors->saveState();
    d->sketchSearchAlbumSelectors->saveState();
}

void FuzzySearchView::setActive(bool val)
{
    d->active = val;

    // at first occasion, warn if no fingerprints are available
    if (val && !d->fingerprintsChecked && isVisible())
    {
        if (!SimilarityDbAccess().db()->hasFingerprints())
        {
            QString msg = i18n("Image fingerprints have not yet been generated for your collection. "
                               "The Similarity Search Tools will not be operational "
                               "without pre-generated fingerprints. Please generate "
                               "the fingerprints first.");
            QMessageBox::information(this, i18n("No Fingerprints"), msg);
        }

        d->fingerprintsChecked = true;
    }

    int tab = d->tabWidget->currentIndex();

    if (val)
    {
        slotTabChanged(tab);
    }
}

void FuzzySearchView::slotTabChanged(int tab)
{
    /**
     * Set a list with only one element, albummanager can set only multiple albums
     */
    QList<Album*> albums;

    switch (tab)
    {
        case Private::SIMILARS:
        {
            albums << d->imageSAlbum;
            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(true);
            break;
        }

        case Private::SKETCH:
        {
            albums << d->sketchSAlbum;
            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(true);
            break;
        }

        default:  // DUPLICATES
        {
            d->findDuplicatesPanel->setActive(true);
            QList<SAlbum*> sAlbums = d->findDuplicatesPanel->currentFindDuplicatesAlbums();

            foreach(SAlbum* const album, sAlbums)
            {
                albums << album;
            }

            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(false);
            break;
        }
    }
}

void FuzzySearchView::slotAlbumSelected(Album* album)
{

    qCDebug(DIGIKAM_GENERAL_LOG) << "Selected new album" << album;

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum || !salbum->isHaarSearch())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Not a haar search, returning";
        return;
    }

    if (!d->active)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Not active, returning";
        return;
    }

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type               = reader.attributes().value(QLatin1String("type"));
    QStringRef numResultsString   = reader.attributes().value(QLatin1String("numberofresults"));
    QStringRef thresholdString    = reader.attributes().value(QLatin1String("threshold"));
    QStringRef maxThresholdString = reader.attributes().value(QLatin1String("maxthreshold"));
    QStringRef sketchTypeString   = reader.attributes().value(QLatin1String("sketchtype"));

    if (type == QLatin1String("imageid"))
    {
        setCurrentImage(reader.valueToLongLong());
        d->imageSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)Private::SIMILARS);
    }
    else if (type == QLatin1String("signature"))
    {
        d->sketchSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)Private::SKETCH);

        if (reader.readToStartOfElement(QLatin1String("SketchImage")))
        {
            d->sketchWidget->setSketchImageFromXML(reader);
        }
    }
}

void FuzzySearchView::slotApplicationSettingsChanged()
{
    d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(),100);
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

void FuzzySearchView::slotPenColorChanged(const QColor& color)
{
    slotHSChanged(color.hue(), color.saturation());
    slotVChanged(color.value());
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
    createNewFuzzySearchAlbumFromSketch(d->nameEditSketch->text());
}

void FuzzySearchView::slotDirtySketch()
{
    if (d->active)
    {
        d->timerSketch->start();
    }
}

void FuzzySearchView::slotTimerSketchDone()
{
    slotCheckNameEditSketchConditions();
    createNewFuzzySearchAlbumFromSketch(SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch), true);
}

void FuzzySearchView::createNewFuzzySearchAlbumFromSketch(const QString& name, bool force)
{
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());

    QList<int> albums = d->sketchSearchAlbumSelectors->selectedAlbumIds();

    d->sketchSAlbum = d->searchModificationHelper->createFuzzySearchFromSketch(name, d->sketchWidget,
                                                                               d->resultsSketch->value(),
                                                                               albums, force);
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->sketchSAlbum);
}

void FuzzySearchView::slotClearSketch()
{
    d->sketchWidget->slotClear();
    slotCheckNameEditSketchConditions();
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
}

void FuzzySearchView::slotCheckNameEditSketchConditions()
{
    if (!d->sketchWidget->isClear())
    {
        d->nameEditSketch->setEnabled(true);

        if (!d->nameEditSketch->text().isEmpty())
        {
            d->saveBtnSketch->setEnabled(true);
        }
    }
    else
    {
        d->nameEditSketch->setEnabled(false);
        d->saveBtnSketch->setEnabled(false);
    }
}

// Similars Searches methods ----------------------------------------------------------------------

void FuzzySearchView::dragEnterEvent(QDragEnterEvent* e)
{
    if (DItemDrag::canDecode(e->mimeData()))
    {
        e->acceptProposedAction();
    }
    else if (e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();

        // If there is at least one URL and the URL is a local file.
        if (!urls.empty())
        {
            if (urls.first().isLocalFile())
            {
                HaarIface haarIface;
                QString path       = urls.first().toLocalFile();
                const QImage image = haarIface.loadQImage(path);

                if (!image.isNull())
                {
                    e->acceptProposedAction();
                }
            }
        }
    }
}

void FuzzySearchView::dropEvent(QDropEvent* e)
{
    if (DItemDrag::canDecode(e->mimeData()))
    {
        QList<QUrl>      urls;
        QList<QUrl>      ioURLs;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, ioURLs, albumIDs, imageIDs))
        {
            return;
        }

        if (imageIDs.isEmpty())
        {
            return;
        }

        setImageInfo(ImageInfo(imageIDs.first()));

        e->acceptProposedAction();
    }

    // Allow dropping urls and handle them as sketch search if the urls represent images.
    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();

        // If there is at least one URL and the URL is a local file.
        if (!urls.empty())
        {
            if (urls.first().isLocalFile())
            {
                HaarIface haarIface;
                QString path       = urls.first().toLocalFile();
                const QImage image = haarIface.loadQImage(path);

                if (!image.isNull())
                {
                    // Set a temporary image id
                    d->imageInfo = ImageInfo(-1);
                    d->imageUrl  = urls.first();

                    d->imageWidget->setPixmap(QPixmap::fromImage(image).scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));

                    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
                    QString haarTitle = SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch);

                    QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

                    d->imageSAlbum = d->searchModificationHelper->createFuzzySearchFromDropped(haarTitle, path,
                                                                                               d->similarityRange->minValue() / 100.0,
                                                                                               d->similarityRange->maxValue() / 100.0,
                                                                                               albums, true);
                    d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
                    d->labelFile->setAdjustedText(urls.first().fileName());
                    d->labelFolder->setAdjustedText(urls.first().adjusted(QUrl::RemoveFilename).toLocalFile());

                    e->acceptProposedAction();
                }
            }
        }
    }
}

void FuzzySearchView::slotMaxLevelImageChanged(int /*newValue*/)
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotMinLevelImageChanged(int /*newValue*/)
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotFuzzyAlbumsChanged()
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotTimerImageDone()
{
    if (d->imageInfo.isNull() && d->imageInfo.id() == -1 && !d->imageUrl.isEmpty())
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
        QString haarTitle = SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch);

        QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

        d->imageSAlbum    = d->searchModificationHelper->createFuzzySearchFromDropped(haarTitle,
                                                                                      d->imageUrl.toLocalFile(),
                                                                                      d->similarityRange->minValue() / 100.0,
                                                                                      d->similarityRange->maxValue() / 100.0,
                                                                                      albums, true);
        d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
        return;
    }

    if (!d->imageInfo.isNull() && d->active)
    {
        setImageInfo(d->imageInfo);
    }
}

void FuzzySearchView::setCurrentImage(qlonglong imageid)
{
    setCurrentImage(ImageInfo(imageid));
}

void FuzzySearchView::setCurrentImage(const ImageInfo& info)
{
    d->imageInfo = info;
    d->imageUrl  = info.fileUrl();
    d->labelFile->setAdjustedText(d->imageInfo.name());
    d->labelFolder->setAdjustedText(d->imageInfo.fileUrl().adjusted(QUrl::RemoveFilename).toLocalFile());
    d->thumbLoadThread->find(d->imageInfo.thumbnailIdentifier());
}

void FuzzySearchView::setImageInfo(const ImageInfo& info)
{
    setCurrentImage(info);
    slotCheckNameEditImageConditions();
    createNewFuzzySearchAlbumFromImage(SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch), true);
    d->tabWidget->setCurrentIndex((int)Private::SIMILARS);
}

void FuzzySearchView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    if (!d->imageInfo.isNull() && QUrl::fromLocalFile(desc.filePath) == d->imageInfo.fileUrl())
    {
        d->imageWidget->setPixmap(pix.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void FuzzySearchView::createNewFuzzySearchAlbumFromImage(const QString& name, bool force)
{
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
    QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

    d->imageSAlbum = d->searchModificationHelper->createFuzzySearchFromImage(name, d->imageInfo,
                                                                             d->similarityRange->minValue() / 100.0,
                                                                             d->similarityRange->maxValue() / 100.0,
                                                                             albums, force);
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
}

void FuzzySearchView::slotCheckNameEditImageConditions()
{
    if (!d->imageInfo.isNull())
    {
        d->nameEditImage->setEnabled(true);

        if (!d->nameEditImage->text().isEmpty())
        {
            d->saveBtnImage->setEnabled(true);
        }
    }
    else
    {
        d->nameEditImage->setEnabled(false);
        d->saveBtnImage->setEnabled(false);
    }
}

void FuzzySearchView::slotSaveImageSAlbum()
{
    createNewFuzzySearchAlbumFromImage(d->nameEditImage->text());
}

}  // namespace Digikam
