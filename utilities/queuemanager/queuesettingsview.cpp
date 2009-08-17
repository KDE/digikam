/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "queuesettingsview.moc"

// Qt includes

#include <QTimer>
#include <QLabel>
#include <QScrollArea>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QGroupBox>
#include <QTreeWidget>

// KDE includes

#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kvbox.h>

// Local includes

#include "batchtool.h"
#include "album.h"
#include "albumselectwidget.h"
#include "manualrenamewidget.h"

using namespace Digikam::ManualRename;
namespace Digikam
{

class QueueSettingsViewPriv
{

public:

    QueueSettingsViewPriv()
    {
        conflictLabel       = 0;
        conflictButtonGroup = 0;
        overwriteButton     = 0;
        promptButton        = 0;
        albumSel            = 0;
    }

    QLabel            *conflictLabel;

    QButtonGroup      *renamingButtonGroup;
    QButtonGroup      *conflictButtonGroup;

    QRadioButton      *renameOriginal;
    QRadioButton      *renameManual;
    QRadioButton      *overwriteButton;
    QRadioButton      *promptButton;

    AlbumSelectWidget *albumSel;

    ManualRenameWidget *manualRenameInput;
};

QueueSettingsView::QueueSettingsView(QWidget *parent)
                 : KTabWidget(parent), d(new QueueSettingsViewPriv)
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

    QScrollArea *sv = new QScrollArea(this);
    QWidget *panel  = new QWidget(sv->viewport());
    panel->setAutoFillBackground(false);
    sv->setWidget(panel);
    sv->setWidgetResizable(true);
    sv->viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout    = new QVBoxLayout(panel);
    d->conflictLabel       = new QLabel(i18n("If Target File Exists:"), panel);
    QWidget *conflictBox   = new QWidget(panel);
    QVBoxLayout *vlay      = new QVBoxLayout(conflictBox);
    d->conflictButtonGroup = new QButtonGroup(conflictBox);
    d->overwriteButton     = new QRadioButton(i18n("Overwrite automatically"), conflictBox);
    d->promptButton        = new QRadioButton(i18n("Open rename-file dialog"), conflictBox);
    d->conflictButtonGroup->addButton(d->overwriteButton, QueueSettings::OVERWRITE);
    d->conflictButtonGroup->addButton(d->promptButton,    QueueSettings::ASKTOUSER);
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

    QScrollArea *sv2 = new QScrollArea(this);
    KVBox *vbox2     = new KVBox(sv2->viewport());
    vbox2->setAutoFillBackground(false);
    sv2->setWidget(vbox2);
    sv2->setWidgetResizable(true);
    sv2->viewport()->setAutoFillBackground(false);

    d->renamingButtonGroup = new QButtonGroup(vbox2);
    d->renameOriginal      = new QRadioButton(i18n("Use original filenames"), vbox2);
    d->renameOriginal->setWhatsThis(i18n("Turn on this option to use original "
                                         "filenames without modifications."));

    d->renameManual        = new QRadioButton(i18n("Customize filenames:"), vbox2);
    d->manualRenameInput   = new ManualRenameWidget(vbox2);
    QWidget *space         = new QWidget(vbox2);

    d->renamingButtonGroup->setExclusive(true);
    d->renamingButtonGroup->addButton(d->renameOriginal, QueueSettings::USEORIGINAL);
    d->renamingButtonGroup->addButton(d->renameManual,   QueueSettings::CUSTOMIZE);

    vbox2->setStretchFactor(space, 10);
    vbox2->setMargin(KDialog::spacingHint());
    vbox2->setSpacing(KDialog::spacingHint());

    addTab(sv2, SmallIcon("insert-image"), i18n("File Renaming"));

    // --------------------------------------------------------

    connect(d->albumSel->albumView(), SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->conflictButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->renamingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->manualRenameInput, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotSettingsChanged()));

    // --------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

QueueSettingsView::~QueueSettingsView()
{
    delete d;
}

void QueueSettingsView::setBusy(bool b)
{
    for (int i = 0; i < count(); ++i)
        widget(i)->setEnabled(!b);
}

void QueueSettingsView::slotResetSettings()
{
    blockSignals(true);
    // TODO: reset d->albumSel
    d->conflictButtonGroup->button(QueueSettings::ASKTOUSER)->setChecked(true);
    d->renamingButtonGroup->button(QueueSettings::USEORIGINAL)->setChecked(true);
    d->manualRenameInput->input()->clear();
    blockSignals(false);
    slotSettingsChanged();
}

void QueueSettingsView::slotQueueSelected(int, const QueueSettings& settings, const AssignedBatchTools&)
{
    d->albumSel->setCurrentAlbumUrl(settings.targetUrl);
    int btn = (int)settings.conflictRule;
    d->conflictButtonGroup->button(btn)->setChecked(true);
    btn     = (int)settings.renamingRule;
    d->renamingButtonGroup->button(btn)->setChecked(true);
    d->manualRenameInput->setText(settings.renamingParser);
}

void QueueSettingsView::slotUpdateTrackerPos()
{
    d->manualRenameInput->slotUpdateTrackerPos();
}

void QueueSettingsView::slotHideToolTipTracker()
{
    d->manualRenameInput->slotHideToolTipTracker();
}

void QueueSettingsView::slotSettingsChanged()
{
    QueueSettings settings;
    settings.conflictRule   = (QueueSettings::ConflictRule)d->conflictButtonGroup->checkedId();
    settings.targetUrl      = d->albumSel->currentAlbumUrl();
    settings.renamingRule   = (QueueSettings::RenamingRule)d->renamingButtonGroup->checkedId();
    settings.renamingParser = d->manualRenameInput->text();
    d->manualRenameInput->setEnabled(settings.renamingRule == QueueSettings::CUSTOMIZE);
    emit signalSettingsChanged(settings);
}

}  // namespace Digikam
