/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009-2012 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "advancedrenamewidget.h"
#include "defaultrenameparser.h"
#include "album.h"
#include "albumselectwidget.h"
#include "batchtool.h"
#include "queuesettings.h"

namespace Digikam
{

class QueueSettingsView::Private
{

public:

    Private() :
        conflictLabel(0),
        renamingButtonGroup(0),
        conflictButtonGroup(0),
        renameOriginal(0),
        renameManual(0),
        overwriteButton(0),
        promptButton(0),
        albumSel(0),
        advancedRenameManager(0),
        advancedRenameWidget(0)
    {
    }

    QLabel*                conflictLabel;

    QButtonGroup*          renamingButtonGroup;
    QButtonGroup*          conflictButtonGroup;

    QRadioButton*          renameOriginal;
    QRadioButton*          renameManual;
    QRadioButton*          overwriteButton;
    QRadioButton*          promptButton;

    AlbumSelectWidget*     albumSel;

    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;
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

    d->albumSel = new AlbumSelectWidget(this);
    addTab(d->albumSel, SmallIcon("folder-image"), i18n("Target"));

    // --------------------------------------------------------

    QScrollArea* sv = new QScrollArea(this);
    QWidget* panel  = new QWidget(sv->viewport());
    sv->setWidget(panel);
    sv->setWidgetResizable(true);

    QVBoxLayout* layout    = new QVBoxLayout(panel);
    d->conflictLabel       = new QLabel(i18n("If Target File Exists:"), panel);
    QWidget* conflictBox   = new QWidget(panel);
    QVBoxLayout* vlay      = new QVBoxLayout(conflictBox);
    d->conflictButtonGroup = new QButtonGroup(conflictBox);
    d->overwriteButton     = new QRadioButton(i18n("Overwrite automatically"),   conflictBox);
    d->promptButton        = new QRadioButton(i18n("Store as a different name"), conflictBox);
    d->conflictButtonGroup->addButton(d->overwriteButton, QueueSettings::OVERWRITE);
    d->conflictButtonGroup->addButton(d->promptButton,    QueueSettings::DIFFNAME);
    d->conflictButtonGroup->setExclusive(true);
    d->promptButton->setChecked(true);

    vlay->addWidget(d->overwriteButton);
    vlay->addWidget(d->promptButton);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    layout->addWidget(d->conflictLabel);
    layout->addWidget(conflictBox);
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    addTab(sv, SmallIcon("dialog-information"), i18n("Behavior"));

    // --------------------------------------------------------

    QScrollArea* sv2 = new QScrollArea(this);
    KVBox* vbox2     = new KVBox(sv2->viewport());
    sv2->setWidget(vbox2);
    sv2->setWidgetResizable(true);

    d->renamingButtonGroup = new QButtonGroup(vbox2);
    d->renameOriginal      = new QRadioButton(i18n("Use original filenames"), vbox2);
    d->renameOriginal->setWhatsThis(i18n("Turn on this option to use original "
                                         "filenames without modifications."));

    d->renameManual          = new QRadioButton(i18n("Customize filenames:"), vbox2);

    d->advancedRenameWidget  = new AdvancedRenameWidget(vbox2);
    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    QWidget* space           = new QWidget(vbox2);

    d->renamingButtonGroup->setExclusive(true);
    d->renamingButtonGroup->addButton(d->renameOriginal, QueueSettings::USEORIGINAL);
    d->renamingButtonGroup->addButton(d->renameManual,   QueueSettings::CUSTOMIZE);

    vbox2->setStretchFactor(space, 10);
    vbox2->setMargin(KDialog::spacingHint());
    vbox2->setSpacing(KDialog::spacingHint());

    addTab(sv2, SmallIcon("insert-image"), i18n("File Renaming"));

    // --------------------------------------------------------

    connect(d->albumSel, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->conflictButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->renamingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
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
    // TODO: reset d->albumSel
    d->conflictButtonGroup->button(QueueSettings::OVERWRITE)->setChecked(true);
    d->renamingButtonGroup->button(QueueSettings::USEORIGINAL)->setChecked(true);
    d->advancedRenameWidget->clearParseString();
    blockSignals(false);
    slotSettingsChanged();
}

void QueueSettingsView::slotQueueSelected(int, const QueueSettings& settings, const AssignedBatchTools&)
{
    d->albumSel->setCurrentAlbumUrl(settings.workingUrl);
    int btn = (int)settings.conflictRule;
    d->conflictButtonGroup->button(btn)->setChecked(true);
    btn     = (int)settings.renamingRule;
    d->renamingButtonGroup->button(btn)->setChecked(true);
    d->advancedRenameWidget->setParseString(settings.renamingParser);
}

void QueueSettingsView::slotSettingsChanged()
{
    QueueSettings settings;
    settings.conflictRule   = (QueueSettings::ConflictRule)d->conflictButtonGroup->checkedId();
    settings.workingUrl     = d->albumSel->currentAlbumUrl();
    settings.renamingRule   = (QueueSettings::RenamingRule)d->renamingButtonGroup->checkedId();
    settings.renamingParser = d->advancedRenameWidget->parseString();
    d->advancedRenameWidget->setEnabled(settings.renamingRule == QueueSettings::CUSTOMIZE);

    emit signalSettingsChanged(settings);
}

}  // namespace Digikam
