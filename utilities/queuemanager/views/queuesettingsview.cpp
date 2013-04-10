/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuesettingsview.moc"

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

// KDE includes

#include <kconfig.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/dcrawsettingswidget.h>

// Local includes

#include "advancedrenamewidget.h"
#include "defaultrenameparser.h"
#include "album.h"
#include "albumselectwidget.h"
#include "batchtool.h"
#include "queuesettings.h"

using namespace KDcrawIface;

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
        RAW
    };

public:

    Private() :
        conflictLabel(0),
        rawLoadingLabel(0),
        renamingButtonGroup(0),
        conflictButtonGroup(0),
        rawLoadingButtonGroup(0),
        renameOriginal(0),
        renameManual(0),
        overwriteButton(0),
        storeDiffButton(0),
        extractJPEGButton(0),
        demosaicingButton(0),
        useOrgAlbum(0),
        albumSel(0),
        advancedRenameManager(0),
        advancedRenameWidget(0),
        rawSettings(0)
    {
    }

    QLabel*                conflictLabel;
    QLabel*                rawLoadingLabel;

    QButtonGroup*          renamingButtonGroup;
    QButtonGroup*          conflictButtonGroup;
    QButtonGroup*          rawLoadingButtonGroup;

    QRadioButton*          renameOriginal;
    QRadioButton*          renameManual;
    QRadioButton*          overwriteButton;
    QRadioButton*          storeDiffButton;
    QRadioButton*          extractJPEGButton;
    QRadioButton*          demosaicingButton;

    QCheckBox*             useOrgAlbum;

    AlbumSelectWidget*     albumSel;

    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;

    DcrawSettingsWidget*   rawSettings;
};

QueueSettingsView::QueueSettingsView(QWidget* const parent)
    : KTabWidget(parent), d(new Private)
{
    setTabBarHidden(false);
#if KDE_IS_VERSION(4,3,0)
    setTabsClosable(false);
#else
    setCloseButtonEnabled(false);
#endif

    // --------------------------------------------------------

    QScrollArea* const sv3   = new QScrollArea(this);
    KVBox* const vbox3       = new KVBox(sv3->viewport());
    sv3->setWidget(vbox3);
    sv3->setWidgetResizable(true);
    vbox3->setMargin(KDialog::spacingHint());
    vbox3->setSpacing(KDialog::spacingHint());

    d->useOrgAlbum           = new QCheckBox(i18n("Use original Album"), vbox3);
    d->albumSel              = new AlbumSelectWidget(vbox3);
    insertTab(Private::TARGET, sv3, SmallIcon("folder-image"), i18n("Target"));

    // --------------------------------------------------------

    QScrollArea* const sv2   = new QScrollArea(this);
    KVBox* const vbox2       = new KVBox(sv2->viewport());
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
    vbox2->setMargin(KDialog::spacingHint());
    vbox2->setSpacing(KDialog::spacingHint());

    insertTab(Private::RENAMING, sv2, SmallIcon("insert-image"), i18n("File Renaming"));

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
    d->demosaicingButton         = new QRadioButton(i18n("Perform RAW demosaicing"),          rawLoadingBox);
    d->extractJPEGButton         = new QRadioButton(i18n("Extract embedded preview (faster)"), rawLoadingBox);
    d->rawLoadingButtonGroup->addButton(d->extractJPEGButton, QueueSettings::USEEMBEDEDJPEG);
    d->rawLoadingButtonGroup->addButton(d->demosaicingButton, QueueSettings::DEMOSAICING);
    d->rawLoadingButtonGroup->setExclusive(true);
    d->demosaicingButton->setChecked(true);

    vlay2->addWidget(d->demosaicingButton);
    vlay2->addWidget(d->extractJPEGButton);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // -------------

    d->conflictLabel           = new QLabel(i18n("If Target File Exists:"), panel);
    QWidget* const conflictBox = new QWidget(panel);
    QVBoxLayout* const vlay    = new QVBoxLayout(conflictBox);
    d->conflictButtonGroup     = new QButtonGroup(conflictBox);
    d->storeDiffButton         = new QRadioButton(i18n("Store as a different name"), conflictBox);
    d->overwriteButton         = new QRadioButton(i18n("Overwrite automatically"),   conflictBox);
    d->conflictButtonGroup->addButton(d->overwriteButton, QueueSettings::OVERWRITE);
    d->conflictButtonGroup->addButton(d->storeDiffButton, QueueSettings::DIFFNAME);
    d->conflictButtonGroup->setExclusive(true);
    d->storeDiffButton->setChecked(true);

    vlay->addWidget(d->storeDiffButton);
    vlay->addWidget(d->overwriteButton);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    // -------------

    layout->addWidget(d->rawLoadingLabel);
    layout->addWidget(rawLoadingBox);
    layout->addWidget(d->conflictLabel);
    layout->addWidget(conflictBox);
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    insertTab(Private::BEHAVIOR, sv, SmallIcon("dialog-information"), i18n("Behavior"));

    // --------------------------------------------------------

    d->rawSettings = new DcrawSettingsWidget(panel, DcrawSettingsWidget::SIXTEENBITS | DcrawSettingsWidget::COLORSPACE);
    d->rawSettings->setItemIcon(0, SmallIcon("kdcraw"));
    d->rawSettings->setItemIcon(1, SmallIcon("whitebalance"));
    d->rawSettings->setItemIcon(2, SmallIcon("lensdistortion"));

    insertTab(Private::RAW, d->rawSettings, SmallIcon("kdcraw"), i18n("Raw Decoding"));

    // --------------------------------------------------------

    connect(d->useOrgAlbum, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->renamingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->conflictButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawLoadingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->rawSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    // --------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));

    // --------------------------------------------------------

    setAutoFillBackground(false);

    sv->setAutoFillBackground(false);
    sv->viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);

    sv2->setAutoFillBackground(false);
    sv2->viewport()->setAutoFillBackground(false);
    vbox2->setAutoFillBackground(false);
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

void QueueSettingsView::slotResetSettings()
{
    blockSignals(true);
    d->useOrgAlbum->setChecked(true);
    // TODO: reset d->albumSel
    d->renamingButtonGroup->button(QueueSettings::USEORIGINAL)->setChecked(true);
    d->conflictButtonGroup->button(QueueSettings::DIFFNAME)->setChecked(true);
    d->rawLoadingButtonGroup->button(QueueSettings::DEMOSAICING)->setChecked(true);
    d->advancedRenameWidget->clearParseString();
    d->rawSettings->resetToDefault();
    blockSignals(false);
    slotSettingsChanged();
}

void QueueSettingsView::slotQueueSelected(int, const QueueSettings& settings, const AssignedBatchTools&)
{
    d->useOrgAlbum->setChecked(settings.useOrgAlbum);
    d->albumSel->setEnabled(!settings.useOrgAlbum);
    d->albumSel->setCurrentAlbumUrl(settings.workingUrl);

    int btn = (int)settings.renamingRule;
    d->renamingButtonGroup->button(btn)->setChecked(true);

    btn     = (int)settings.conflictRule;
    d->conflictButtonGroup->button(btn)->setChecked(true);

    btn     = (int)settings.rawLoadingRule;
    d->rawLoadingButtonGroup->button(btn)->setChecked(true);

    d->advancedRenameWidget->setParseString(settings.renamingParser);

    d->rawSettings->setSettings(settings.rawDecodingSettings);
}

void QueueSettingsView::slotSettingsChanged()
{
    QueueSettings settings;

    d->albumSel->setEnabled(!d->useOrgAlbum->isChecked());
    settings.useOrgAlbum         = d->useOrgAlbum->isChecked();
    settings.workingUrl          = d->albumSel->currentAlbumUrl();

    settings.renamingRule        = (QueueSettings::RenamingRule)d->renamingButtonGroup->checkedId();
    settings.renamingParser      = d->advancedRenameWidget->parseString();
    d->advancedRenameWidget->setEnabled(settings.renamingRule == QueueSettings::CUSTOMIZE);

    settings.conflictRule        = (QueueSettings::ConflictRule)d->conflictButtonGroup->checkedId();

    settings.rawLoadingRule      = (QueueSettings::RawLoadingRule)d->rawLoadingButtonGroup->checkedId();
    setTabEnabled(Private::RAW, (settings.rawLoadingRule == QueueSettings::DEMOSAICING));

    settings.rawDecodingSettings = d->rawSettings->settings();

    emit signalSettingsChanged(settings);
}

}  // namespace Digikam
