/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuesettingsview.h"

// Qt includes

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "advancedrenamewidget.h"
#include "defaultrenameparser.h"
#include "album.h"
#include "albumselectwidget.h"
#include "batchtool.h"
#include "queuesettings.h"
#include "jpegsettings.h"
#include "tiffsettings.h"
#include "pngsettings.h"
#include "pgfsettings.h"
#include "drawdecoderwidget.h"
#include "filesaveconflictbox.h"

#ifdef HAVE_JASPER
#   include "jp2ksettings.h"
#endif // HAVE_JASPER

namespace Digikam
{

class QueueSettingsView::Private
{
public:

    enum SettingsTabs
    {
        TARGET = 0,
        RENAMING,
        BEHAVIOR,
        RAW,
        SAVE
    };

public:

    Private() :
        rawLoadingLabel(0),
        renamingButtonGroup(0),
        rawLoadingButtonGroup(0),
        renameOriginal(0),
        renameManual(0),
        extractJPEGButton(0),
        demosaicingButton(0),
        useOrgAlbum(0),
        useMutiCoreCPU(0),
        conflictBox(0),
        albumSel(0),
        advancedRenameManager(0),
        advancedRenameWidget(0),
        rawSettings(0),
        jpgSettings(0),
        pngSettings(0),
        tifSettings(0),
#ifdef HAVE_JASPER
        j2kSettings(0),
#endif // HAVE_JASPER
        pgfSettings(0)
    {
    }

    QLabel*                rawLoadingLabel;

    QButtonGroup*          renamingButtonGroup;
    QButtonGroup*          rawLoadingButtonGroup;

    QRadioButton*          renameOriginal;
    QRadioButton*          renameManual;
    QRadioButton*          extractJPEGButton;
    QRadioButton*          demosaicingButton;

    QCheckBox*             useOrgAlbum;
    QCheckBox*             useMutiCoreCPU;

    FileSaveConflictBox*   conflictBox;
    AlbumSelectWidget*     albumSel;

    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;

    DRawDecoderWidget*   rawSettings;

    JPEGSettings*          jpgSettings;
    PNGSettings*           pngSettings;
    TIFFSettings*          tifSettings;
#ifdef HAVE_JASPER
    JP2KSettings*          j2kSettings;
#endif // HAVE_JASPER
    PGFSettings*           pgfSettings;
};

QueueSettingsView::QueueSettingsView(QWidget* const parent)
    : QTabWidget(parent),
      d(new Private)
{
    setTabsClosable(false);

    // --------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QScrollArea* const sv3   = new QScrollArea(this);
    DVBox* const vbox3       = new DVBox(sv3->viewport());
    sv3->setWidget(vbox3);
    sv3->setWidgetResizable(true);
    vbox3->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox3->setSpacing(spacing);

    d->useOrgAlbum           = new QCheckBox(i18n("Use original Album"), vbox3);
    d->albumSel              = new AlbumSelectWidget(vbox3);
    insertTab(Private::TARGET, sv3, QIcon::fromTheme(QLatin1String("folder-pictures")), i18n("Target"));

    // --------------------------------------------------------

    QScrollArea* const sv2   = new QScrollArea(this);
    DVBox* const vbox2       = new DVBox(sv2->viewport());
    sv2->setWidget(vbox2);
    sv2->setWidgetResizable(true);

    d->renamingButtonGroup   = new QButtonGroup(vbox2);
    d->renameOriginal        = new QRadioButton(i18n("Use original filenames"), vbox2);
    d->renameOriginal->setWhatsThis(i18n("Turn on this option to use original "
                                         "filenames without modifications."));

    d->renameManual          = new QRadioButton(i18n("Customize filenames:"), vbox2);

    d->advancedRenameWidget  = new AdvancedRenameWidget(vbox2);
    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    QWidget* const space     = new QWidget(vbox2);

    d->renamingButtonGroup->setExclusive(true);
    d->renamingButtonGroup->addButton(d->renameOriginal, QueueSettings::USEORIGINAL);
    d->renamingButtonGroup->addButton(d->renameManual,   QueueSettings::CUSTOMIZE);

    vbox2->setStretchFactor(space, 10);
    vbox2->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox2->setSpacing(spacing);

    insertTab(Private::RENAMING, sv2, QIcon::fromTheme(QLatin1String("insert-image")), i18n("File Renaming"));

    // --------------------------------------------------------

    QScrollArea* const sv     = new QScrollArea(this);
    QWidget* const panel      = new QWidget(sv->viewport());
    QVBoxLayout* const layout = new QVBoxLayout(panel);
    sv->setWidget(panel);
    sv->setWidgetResizable(true);

    // --------------------------------------------------------

    d->rawLoadingLabel           = new QLabel(i18n("Raw Files Loading:"), panel);
    QWidget* const rawLoadingBox = new QWidget(panel);
    QVBoxLayout* const vlay2     = new QVBoxLayout(rawLoadingBox);
    d->rawLoadingButtonGroup     = new QButtonGroup(rawLoadingBox);
    d->demosaicingButton         = new QRadioButton(i18n("Perform RAW demosaicing"),           rawLoadingBox);
    d->extractJPEGButton         = new QRadioButton(i18n("Extract embedded preview (faster)"), rawLoadingBox);
    d->rawLoadingButtonGroup->addButton(d->extractJPEGButton, QueueSettings::USEEMBEDEDJPEG);
    d->rawLoadingButtonGroup->addButton(d->demosaicingButton, QueueSettings::DEMOSAICING);
    d->rawLoadingButtonGroup->setExclusive(true);
    d->demosaicingButton->setChecked(true);

    vlay2->addWidget(d->demosaicingButton);
    vlay2->addWidget(d->extractJPEGButton);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    // -------------

    d->conflictBox    = new FileSaveConflictBox(panel);

    d->useMutiCoreCPU = new QCheckBox(i18nc("@option:check", "Work on all processor cores"), panel);
    d->useMutiCoreCPU->setWhatsThis(i18n("Turn on this option to use all CPU core from your computer "
                                         "to process more than one item from a queue at the same time."));
    // -------------

    layout->addWidget(d->rawLoadingLabel);
    layout->addWidget(rawLoadingBox);
    layout->addWidget(d->conflictBox);
    layout->addWidget(d->useMutiCoreCPU);
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addStretch();

    insertTab(Private::BEHAVIOR, sv, QIcon::fromTheme(QLatin1String("dialog-information")), i18n("Behavior"));

    // --------------------------------------------------------

    d->rawSettings = new DRawDecoderWidget(panel, DRawDecoderWidget::SIXTEENBITS | DRawDecoderWidget::COLORSPACE);
    d->rawSettings->setItemIcon(0, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));
    d->rawSettings->setItemIcon(1, QIcon::fromTheme(QLatin1String("bordertool")));
    d->rawSettings->setItemIcon(2, QIcon::fromTheme(QLatin1String("zoom-draw")));

    insertTab(Private::RAW, d->rawSettings, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")), i18n("Raw Decoding"));

    // --------------------------------------------------------

    QScrollArea* const sv4   = new QScrollArea(this);
    QWidget* const spanel    = new QWidget(sv4->viewport());
    QVBoxLayout* const slay  = new QVBoxLayout(spanel);
    sv4->setWidget(spanel);
    sv4->setWidgetResizable(true);

    QGroupBox* const  box1   = new QGroupBox;
    QVBoxLayout* const lbox1 = new QVBoxLayout;
    d->jpgSettings           = new JPEGSettings();
    lbox1->addWidget(d->jpgSettings);
    box1->setLayout(lbox1);
    slay->addWidget(box1);

    QGroupBox* const  box2   = new QGroupBox;
    QVBoxLayout* const lbox2 = new QVBoxLayout;
    d->pngSettings           = new PNGSettings();
    lbox2->addWidget(d->pngSettings);
    box2->setLayout(lbox2);
    slay->addWidget(box2);

    QGroupBox* const  box3   = new QGroupBox;
    QVBoxLayout* const lbox3 = new QVBoxLayout;
    d->tifSettings           = new TIFFSettings();
    lbox3->addWidget(d->tifSettings);
    box3->setLayout(lbox3);
    slay->addWidget(box3);

#ifdef HAVE_JASPER
    QGroupBox* const  box4   = new QGroupBox;
    QVBoxLayout* const lbox4 = new QVBoxLayout;
    d->j2kSettings           = new JP2KSettings();
    lbox4->addWidget(d->j2kSettings);
    box4->setLayout(lbox4);
    slay->addWidget(box4);
#endif // HAVE_JASPER

    QGroupBox* const  box5   = new QGroupBox;
    QVBoxLayout* const lbox5 = new QVBoxLayout;
    d->pgfSettings           = new PGFSettings();
    lbox5->addWidget(d->pgfSettings);
    box5->setLayout(lbox5);
    slay->addWidget(box5);

    slay->setContentsMargins(spacing, spacing, spacing, spacing);
    slay->setSpacing(spacing);
    slay->addStretch();

    insertTab(Private::SAVE, sv4, QIcon::fromTheme(QLatin1String("document-save-all")), i18n("Saving Images"));

    // --------------------------------------------------------

    connect(d->useOrgAlbum, SIGNAL(toggled(bool)),
            this, SLOT(slotUseOrgAlbum()));

    connect(d->useMutiCoreCPU, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->renamingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->conflictBox, SIGNAL(signalConflictButtonChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawLoadingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->jpgSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->pngSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->tifSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

#ifdef HAVE_JASPER
    connect(d->j2kSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
#endif // HAVE_JASPER

    connect(d->pgfSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    // --------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));

    // --------------------------------------------------------
}

QueueSettingsView::~QueueSettingsView()
{
    delete d->advancedRenameManager;
    delete d;
}

void QueueSettingsView::setBusy(bool b)
{
    for (int i = 0; i < count(); ++i)
    {
        widget(i)->setEnabled(!b);
    }
}

void QueueSettingsView::slotUseOrgAlbum()
{
    if (!d->useOrgAlbum->isChecked())
    {
        PAlbum* const album = AlbumManager::instance()->currentPAlbum();

        if (album)
        {
            blockSignals(true);
            d->albumSel->setCurrentAlbum(album);
            blockSignals(false);
        }
    }

    slotSettingsChanged();
}

void QueueSettingsView::slotResetSettings()
{
    blockSignals(true);
    d->useOrgAlbum->setChecked(true);
    d->useMutiCoreCPU->setChecked(false);
    // TODO: reset d->albumSel
    d->renamingButtonGroup->button(QueueSettings::USEORIGINAL)->setChecked(true);
    d->conflictBox->setConflictRule(FileSaveConflictBox::DIFFNAME);
    d->rawLoadingButtonGroup->button(QueueSettings::DEMOSAICING)->setChecked(true);
    d->advancedRenameWidget->clearParseString();
    d->rawSettings->resetToDefault();
    d->jpgSettings->setCompressionValue(75);
    d->jpgSettings->setSubSamplingValue(1);
    d->pngSettings->setCompressionValue(9);
    d->tifSettings->setCompression(false);
#ifdef HAVE_JASPER
    d->j2kSettings->setLossLessCompression(true);
    d->j2kSettings->setCompressionValue(75);
#endif // HAVE_JASPER
    d->pgfSettings->setLossLessCompression(true);
    d->pgfSettings->setCompressionValue(3);
    blockSignals(false);
    slotSettingsChanged();
}

void QueueSettingsView::slotQueueSelected(int, const QueueSettings& settings, const AssignedBatchTools&)
{
    d->useOrgAlbum->setChecked(settings.useOrgAlbum);
    d->useMutiCoreCPU->setChecked(settings.useMultiCoreCPU);
    d->albumSel->setEnabled(!settings.useOrgAlbum);
    d->albumSel->setCurrentAlbumUrl(settings.workingUrl);

    int btn = (int)settings.renamingRule;
    d->renamingButtonGroup->button(btn)->setChecked(true);

    d->conflictBox->setConflictRule(settings.conflictRule);

    btn     = (int)settings.rawLoadingRule;
    d->rawLoadingButtonGroup->button(btn)->setChecked(true);

    d->advancedRenameWidget->setParseString(settings.renamingParser);

    d->rawSettings->setSettings(settings.rawDecodingSettings);

    d->jpgSettings->setCompressionValue(settings.ioFileSettings.JPEGCompression);
    d->jpgSettings->setSubSamplingValue(settings.ioFileSettings.JPEGSubSampling);
    d->pngSettings->setCompressionValue(settings.ioFileSettings.PNGCompression);
    d->tifSettings->setCompression(settings.ioFileSettings.TIFFCompression);
#ifdef HAVE_JASPER
    d->j2kSettings->setLossLessCompression(settings.ioFileSettings.JPEG2000LossLess);
    d->j2kSettings->setCompressionValue(settings.ioFileSettings.JPEG2000Compression);
#endif // HAVE_JASPER
    d->pgfSettings->setLossLessCompression(settings.ioFileSettings.PGFLossLess);
    d->pgfSettings->setCompressionValue(settings.ioFileSettings.PGFCompression);
}

void QueueSettingsView::slotSettingsChanged()
{
    QueueSettings settings;

    d->albumSel->setEnabled(!d->useOrgAlbum->isChecked());
    settings.useOrgAlbum         = d->useOrgAlbum->isChecked();
    settings.useMultiCoreCPU     = d->useMutiCoreCPU->isChecked();
    settings.workingUrl          = d->albumSel->currentAlbumUrl();

    settings.renamingRule        = (QueueSettings::RenamingRule)d->renamingButtonGroup->checkedId();
    settings.renamingParser      = d->advancedRenameWidget->parseString();
    d->advancedRenameWidget->setEnabled(settings.renamingRule == QueueSettings::CUSTOMIZE);

    settings.conflictRule        = d->conflictBox->conflictRule();

    settings.rawLoadingRule      = (QueueSettings::RawLoadingRule)d->rawLoadingButtonGroup->checkedId();
    setTabEnabled(Private::RAW, (settings.rawLoadingRule == QueueSettings::DEMOSAICING));

    settings.rawDecodingSettings = d->rawSettings->settings();

    settings.ioFileSettings.JPEGCompression     = d->jpgSettings->getCompressionValue();
    settings.ioFileSettings.JPEGSubSampling     = d->jpgSettings->getSubSamplingValue();
    settings.ioFileSettings.PNGCompression      = d->pngSettings->getCompressionValue();
    settings.ioFileSettings.TIFFCompression     = d->tifSettings->getCompression();
#ifdef HAVE_JASPER
    settings.ioFileSettings.JPEG2000LossLess    = d->j2kSettings->getLossLessCompression();
    settings.ioFileSettings.JPEG2000Compression = d->j2kSettings->getCompressionValue();
#endif // HAVE_JASPER
    settings.ioFileSettings.PGFLossLess         = d->pgfSettings->getLossLessCompression();
    settings.ioFileSettings.PGFCompression      = d->pgfSettings->getCompressionValue();

    emit signalSettingsChanged(settings);
}

} // namespace Digikam
