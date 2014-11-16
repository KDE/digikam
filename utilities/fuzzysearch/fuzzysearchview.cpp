/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fuzzysearchview.moc"

// Qt includes

#include <qevent.h>
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
#include <kdebug.h>
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
#include "albumdb.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "albummodel.h"
#include "databaseaccess.h"
#include "ddragobjects.h"
#include "editablesearchtreeview.h"
#include "findduplicatesview.h"
#include "haariface.h"
#include "imageinfo.h"
#include "imagelister.h"
#include "searchmodificationhelper.h"
#include "searchtextbar.h"
#include "searchxml.h"
#include "sketchwidget.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "fingerprintsgenerator.h"

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
        levelImage(0),
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
        searchModel(0),
        searchModificationHelper(0)
    {
    }

    static const QString configTabEntry;
    static const QString configPenSketchSizeEntry;
    static const QString configResultSketchItemsEntry;
    static const QString configPenSketchHueEntry;
    static const QString configPenSketchSaturationEntry;
    static const QString configPenSkethValueEntry;
    static const QString configSimilarsThresholdEntry;

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
    QSpinBox*                 levelImage;

    QLabel*                   imageWidget;

    QTimer*                   timerSketch;
    QTimer*                   timerImage;

    KVBox*                    folderView;

    KLineEdit*                nameEditSketch;
    KLineEdit*                nameEditImage;

    KTabWidget*               tabWidget;

    KHueSaturationSelector*   hsSelector;

    KColorValueSelector*      vSelector;

    KSqueezedTextLabel*       labelFile;
    KSqueezedTextLabel*       labelFolder;

    ImageInfo                 imageInfo;

    SearchTextBar*            searchFuzzyBar;

    EditableSearchTreeView*   searchTreeView;

    SketchWidget*             sketchWidget;

    ThumbnailLoadThread*      thumbLoadThread;

    FindDuplicatesView*       findDuplicatesPanel;

    AlbumPointer<SAlbum>      imageSAlbum;
    AlbumPointer<SAlbum>      sketchSAlbum;

    SearchModel*              searchModel;
    SearchModificationHelper* searchModificationHelper;
};

const QString FuzzySearchView::Private::configTabEntry("FuzzySearch Tab");
const QString FuzzySearchView::Private::configPenSketchSizeEntry("Pen Sketch Size");
const QString FuzzySearchView::Private::configResultSketchItemsEntry("Result Sketch items");
const QString FuzzySearchView::Private::configPenSketchHueEntry("Pen Sketch Hue");
const QString FuzzySearchView::Private::configPenSketchSaturationEntry("Pen Sketch Saturation");
const QString FuzzySearchView::Private::configPenSkethValueEntry("Pen Sketch Value");
const QString FuzzySearchView::Private::configSimilarsThresholdEntry("Similars Threshold");

// --------------------------------------------------------

FuzzySearchView::FuzzySearchView(SearchModel* const searchModel,
                                 SearchModificationHelper* const searchModificationHelper,
                                 QWidget* const parent)
    : QScrollArea(parent), StateSavingObject(this), d(new Private)
{
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

    d->tabWidget               = new KTabWidget();
    d->tabWidget->insertTab(Private::DUPLICATES, d->findDuplicatesPanel, i18n("Duplicates"));
    d->tabWidget->insertTab(Private::SIMILARS,   imagePanel,             i18n("Image"));
    d->tabWidget->insertTab(Private::SKETCH,     sketchPanel,            i18n("Sketch"));

    // ---------------------------------------------------------------

    d->folderView     = new KVBox();
    d->searchTreeView = new EditableSearchTreeView(d->folderView, searchModel, searchModificationHelper);
    d->searchTreeView->filteredModel()->listHaarSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);

    d->searchFuzzyBar = new SearchTextBar(d->folderView, "FuzzySearchViewSearchFuzzyBar");
    d->searchFuzzyBar->setModel(d->searchTreeView->filteredModel(),
                                AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchFuzzyBar->setFilterModel(d->searchTreeView->albumFilterModel());
    d->folderView->setSpacing(KDialog::spacingHint());
    d->folderView->setMargin(0);

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout();
    mainLayout->addWidget(d->tabWidget);
    mainLayout->addWidget(d->folderView);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);

    // ---------------------------------------------------------------

    setupConnections();

    // ---------------------------------------------------------------

    slotCheckNameEditSketchConditions();
    slotCheckNameEditImageConditions();
}

QWidget* FuzzySearchView::setupFindSimilarPanel() const
{
    KHBox* const imageBox = new KHBox();
    d->imageWidget        = new QLabel(imageBox);
    d->imageWidget->setFixedSize(256, 256);
    d->imageWidget->setText(i18n("<p>Drag & drop an image here<br/>to perform similar<br/>items search.</p>"
                                 "<p>You can also use the context menu<br/> when browsing through your images.</p>"));
    d->imageWidget->setAlignment(Qt::AlignCenter);
    imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    imageBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QLabel* const file   = new QLabel(i18n("<b>File</b>:"));
    d->labelFile         = new KSqueezedTextLabel(0);
    QLabel* const folder = new QLabel(i18n("<b>Folder</b>:"));
    d->labelFolder       = new KSqueezedTextLabel(0);
    int hgt              = fontMetrics().height() - 2;
    file->setMaximumHeight(hgt);
    folder->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);

    // ---------------------------------------------------------------

    QLabel* const resultsLabel = new QLabel(i18n("Threshold:"));
    d->levelImage              = new QSpinBox();
    d->levelImage->setSuffix(QChar('%'));
    d->levelImage->setRange(1, 100);
    d->levelImage->setSingleStep(1);
    d->levelImage->setValue(90);
    d->levelImage->setWhatsThis(i18n("Select here the approximate threshold "
                                     "value, as a percentage. "
                                     "This value is used by the algorithm to distinguish two "
                                     "similar images. The default value is 90."));

    // ---------------------------------------------------------------

    KHBox* const saveBox = new KHBox();
    saveBox->setMargin(0);
    saveBox->setSpacing(KDialog::spacingHint());

    d->nameEditImage = new KLineEdit(saveBox);
    d->nameEditImage->setClearButtonShown(true);
    d->nameEditImage->setWhatsThis(i18n("Enter the name of the current similar image search to save in the "
                                        "\"Fuzzy Searches\" view."));

    d->saveBtnImage  = new QToolButton(saveBox);
    d->saveBtnImage->setIcon(SmallIcon("document-save"));
    d->saveBtnImage->setEnabled(false);
    d->saveBtnImage->setToolTip(i18n("Save current similar image search to a new virtual Album"));
    d->saveBtnImage->setWhatsThis(i18n("If you press this button, the current "
                                       "similar image search will be saved to a new search "
                                       "virtual album using name "
                                       "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget();
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(imageBox,       0, 0, 1, -1);
    mainLayout->addWidget(file,           1, 0, 1, 1);
    mainLayout->addWidget(d->labelFile,   1, 1, 1, -1);
    mainLayout->addWidget(folder,         2, 0, 1, 1);
    mainLayout->addWidget(d->labelFolder, 2, 1, 1, -1);
    mainLayout->addWidget(resultsLabel,   3, 0, 1, 1);
    mainLayout->addWidget(d->levelImage,  3, 2, 1, -1);
    mainLayout->addWidget(saveBox,        4, 0, 1, 3);
    mainLayout->setRowStretch(5, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

QWidget* FuzzySearchView::setupSketchPanel() const
{
    KHBox* const drawingBox = new KHBox();
    d->sketchWidget         = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QString tooltip(i18n("Set here the brush color used to draw sketch."));

    d->hsSelector = new KHueSaturationSelector();
    d->hsSelector->setMinimumSize(200, 96);
    d->hsSelector->setChooserMode(ChooserValue);
    d->hsSelector->setColorValue(255);
    d->hsSelector->setWhatsThis(tooltip);

    d->vSelector  = new KColorValueSelector();
    d->vSelector->setMinimumSize(26, 96);
    d->vSelector->setChooserMode(ChooserValue);
    d->vSelector->setIndent(false);
    d->vSelector->setWhatsThis(tooltip);

    // ---------------------------------------------------------------

    d->undoBtnSketch   = new QToolButton();
    d->undoBtnSketch->setAutoRepeat(true);
    d->undoBtnSketch->setIcon(SmallIcon("edit-undo"));
    d->undoBtnSketch->setToolTip(i18n("Undo last draw on sketch"));
    d->undoBtnSketch->setWhatsThis(i18n("Use this button to undo last drawing action on sketch."));
    d->undoBtnSketch->setEnabled(false);

    d->redoBtnSketch   = new QToolButton();
    d->redoBtnSketch->setAutoRepeat(true);
    d->redoBtnSketch->setIcon(SmallIcon("edit-redo"));
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
    settingsLayout->setMargin(0);
    settingsLayout->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox* const saveBox = new KHBox();
    saveBox->setMargin(0);
    saveBox->setSpacing(KDialog::spacingHint());

    d->resetButton = new QToolButton(saveBox);
    d->resetButton->setIcon(SmallIcon("document-revert"));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("Use this button to clear sketch contents."));

    d->nameEditSketch = new KLineEdit(saveBox);
    d->nameEditSketch->setClearButtonShown(true);
    d->nameEditSketch->setWhatsThis(i18n("Enter the name of the current sketch search to save in the "
                                         "\"Fuzzy Searches\" view."));

    d->saveBtnSketch = new QToolButton(saveBox);
    d->saveBtnSketch->setIcon(SmallIcon("document-save"));
    d->saveBtnSketch->setEnabled(false);
    d->saveBtnSketch->setToolTip(i18n("Save current sketch search to a new virtual Album"));
    d->saveBtnSketch->setWhatsThis(i18n("If you press this button, the current sketch "
                                        "fuzzy search will be saved to a new search "
                                        "virtual album using the name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(drawingBox,     0, 0, 1, 3);
    mainLayout->addWidget(d->hsSelector,  1, 0, 1, 2);
    mainLayout->addWidget(d->vSelector,   1, 2, 1, 1);
    mainLayout->addLayout(settingsLayout, 2, 0, 1, 3);
    mainLayout->addWidget(saveBox,        3, 0, 1, 3);
    mainLayout->setRowStretch(5, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

void FuzzySearchView::setupConnections()
{
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

    connect(d->levelImage, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelImageChanged()));

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

    connect(d->nameEditSketch, SIGNAL(returnPressed(QString)),
            d->saveBtnSketch, SLOT(animateClick()));

    connect(d->nameEditImage, SIGNAL(textChanged(QString)),
            this, SLOT(slotCheckNameEditImageConditions()));

    connect(d->nameEditImage, SIGNAL(returnPressed(QString)),
            d->saveBtnImage, SLOT(animateClick()));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
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

void FuzzySearchView::newDuplicatesSearch(Album* const album)
{
    if (album)
    {
        if (album->type() == Album::PHYSICAL)
        {
            d->findDuplicatesPanel->slotSetSelectedAlbum(album);
        }
        else if (album->type() == Album::TAG)
        {
            d->findDuplicatesPanel->slotSetSelectedTag(album);
        }
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

    d->tabWidget->setCurrentIndex(group.readEntry(entryName(d->configTabEntry),                (int)Private::DUPLICATES));
    d->penSize->setValue(group.readEntry(entryName(d->configPenSketchSizeEntry),               10));
    d->resultsSketch->setValue(group.readEntry(entryName(d->configResultSketchItemsEntry),     10));
    d->hsSelector->setHue(group.readEntry(entryName(d->configPenSketchHueEntry),               180));
    d->hsSelector->setSaturation(group.readEntry(entryName(d->configPenSketchSaturationEntry), 128));
    d->vSelector->setValue(group.readEntry(entryName(d->configPenSkethValueEntry),             255));
    d->levelImage->setValue(group.readEntry(entryName(d->configSimilarsThresholdEntry),        90));
    d->hsSelector->updateContents();

    QColor col;
    col.setHsv(d->hsSelector->hue(),
               d->hsSelector->saturation(),
               d->vSelector->value());
    setColor(col);

    d->sketchWidget->setPenWidth(d->penSize->value());

    d->searchTreeView->loadState();
}

void FuzzySearchView::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->configTabEntry),                 d->tabWidget->currentIndex());
    group.writeEntry(entryName(d->configPenSketchSizeEntry),       d->penSize->value());
    group.writeEntry(entryName(d->configResultSketchItemsEntry),   d->resultsSketch->value());
    group.writeEntry(entryName(d->configPenSketchHueEntry),        d->hsSelector->hue());
    group.writeEntry(entryName(d->configPenSketchSaturationEntry), d->hsSelector->saturation());
    group.writeEntry(entryName(d->configPenSkethValueEntry),       d->vSelector->value());
    group.writeEntry(entryName(d->configSimilarsThresholdEntry),   d->levelImage->value());
    d->searchTreeView->saveState();
    group.sync();
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
                               "any time later using 'Tools/Rebuild all Fingerprints'.");
            int result = KMessageBox::questionYesNo(this, msg, i18n("No Fingerprints"));

            if (result == KMessageBox::Yes)
            {
                FingerPrintsGenerator* tool = new FingerPrintsGenerator(true);
                tool->start();
            }
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
            Album* album = d->findDuplicatesPanel->currentFindDuplicatesAlbum();
            if(album)
                albums << album;
            AlbumManager::instance()->setCurrentAlbums(albums);
            d->folderView->setVisible(false);
            break;
        }
    }
}

void FuzzySearchView::slotAlbumSelected(Album* album)
{

    kDebug() << "Selected new album" << album;

    SAlbum* salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum || !salbum->isHaarSearch())
    {
        kDebug() << "Not a haar search, returning";
        return;
    }

    if (!d->active)
    {
        kDebug() << "Not active, returning";
        return;
    }

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
        d->tabWidget->setCurrentIndex((int)Private::SIMILARS);
    }
    else if (type == "signature")
    {
        d->sketchSAlbum = salbum;
        d->tabWidget->setCurrentIndex((int)Private::SKETCH);

        if (reader.readToStartOfElement("SketchImage"))
        {
            d->sketchWidget->setSketchImageFromXML(reader);
        }
    }
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
    if (d->timerSketch)
    {
        d->timerSketch->stop();
        delete d->timerSketch;
    }

    d->timerSketch = new QTimer(this);

    connect( d->timerSketch, SIGNAL(timeout()),
             this, SLOT(slotTimerSketchDone()) );

    d->timerSketch->setSingleShot(true);
    d->timerSketch->start(500);
}

void FuzzySearchView::slotTimerSketchDone()
{
    slotCheckNameEditSketchConditions();
    createNewFuzzySearchAlbumFromSketch(SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch), true);
}

void FuzzySearchView::createNewFuzzySearchAlbumFromSketch(const QString& name, bool force)
{
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
    d->sketchSAlbum = d->searchModificationHelper->createFuzzySearchFromSketch(name, d->sketchWidget, d->resultsSketch->value(), force);
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
}

void FuzzySearchView::dropEvent(QDropEvent* e)
{
    if (DItemDrag::canDecode(e->mimeData()))
    {
        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
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
}

void FuzzySearchView::slotLevelImageChanged()
{
    if (d->timerImage)
    {
        d->timerImage->stop();
    }
    else
    {
        d->timerImage = new QTimer(this);

        connect(d->timerImage, SIGNAL(timeout()),
                this, SLOT(slotTimerImageDone()));

        d->timerImage->setSingleShot(true);
        d->timerImage->setInterval(500);
    }
    d->timerImage->start();
}

void FuzzySearchView::slotTimerImageDone()
{
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
    d->labelFile->setText(d->imageInfo.name());
    d->labelFolder->setText(d->imageInfo.fileUrl().directory());
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
    if (!d->imageInfo.isNull() && KUrl(desc.filePath) == d->imageInfo.fileUrl())
        d->imageWidget->setPixmap(pix.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void FuzzySearchView::createNewFuzzySearchAlbumFromImage(const QString& name, bool force)
{
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
    d->imageSAlbum = d->searchModificationHelper->createFuzzySearchFromImage(name, d->imageInfo, d->levelImage->value() / 100.0, force);
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
