/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuplighttable.h"
#include "setuplighttable.moc"

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

namespace Digikam
{

class SetupLightTablePriv
{
public:

    SetupLightTablePriv() :
        configGroupName("LightTable Settings"), 
        configFullScreenHideToolBarEntry("FullScreen Hide ToolBar"),
        configAutoSyncPreviewEntry("Auto Sync Preview"),
        configAutoLoadRightPanelEntry("Auto Load Right Panel"),
        configLoadFullImagesizeEntry("Load Full Image size"),
        configClearOnCloseEntry("Clear On Close"),

        hideToolBar(0),
        autoSyncPreview(0),
        autoLoadOnRightPanel(0),
        loadFullImageSize(0),
        clearOnClose(0)
    {}

    const QString configGroupName; 
    const QString configFullScreenHideToolBarEntry;
    const QString configAutoSyncPreviewEntry;
    const QString configAutoLoadRightPanelEntry;
    const QString configLoadFullImagesizeEntry;
    const QString configClearOnCloseEntry;

    QCheckBox*    hideToolBar;
    QCheckBox*    autoSyncPreview;
    QCheckBox*    autoLoadOnRightPanel;
    QCheckBox*    loadFullImageSize;
    QCheckBox*    clearOnClose;
};

SetupLightTable::SetupLightTable(QWidget* parent)
               : QScrollArea(parent), d(new SetupLightTablePriv)
{
    QWidget *panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout *gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis(i18n("Set this option to automatically synchronize "
                                          "zooming and panning between left and right panels if the "
                                          "images have the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads the image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis( i18n("Set this option to automatically load an image "
                     "into the right panel when the corresponding item is selected on the thumbbar."));

    d->loadFullImageSize = new QCheckBox(i18n("Load full-sized image"), interfaceOptionsGroup);
    d->loadFullImageSize->setWhatsThis( i18n("Set this option to load the full-sized image "
                     "into the preview panel instead of one at a reduced size. As this option will make it take longer "
                     "to load images, only use it if you have a fast computer."));

    d->hideToolBar  = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);

    d->clearOnClose = new QCheckBox(i18n("Clear the light table on close"));
    d->clearOnClose->setWhatsThis(i18n("Set this option to remove all images "
                                   "from the light table when you close it, "
                                   "or unset it to preserve the images "
                                   "currently on the light table."));

    gLayout->addWidget(d->autoSyncPreview);
    gLayout->addWidget(d->autoLoadOnRightPanel);
    gLayout->addWidget(d->loadFullImageSize);
    gLayout->addWidget(d->hideToolBar);
    gLayout->addWidget(d->clearOnClose);
    gLayout->setMargin(KDialog::spacingHint());
    gLayout->setSpacing(0);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupLightTable::~SetupLightTable()
{
    delete d;
}

void SetupLightTable::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    QColor Black(Qt::black);
    QColor White(Qt::white);

    d->hideToolBar->setChecked(group.readEntry(d->configFullScreenHideToolBarEntry,       false));
    d->autoSyncPreview->setChecked(group.readEntry(d->configAutoSyncPreviewEntry,         true));
    d->autoLoadOnRightPanel->setChecked(group.readEntry(d->configAutoLoadRightPanelEntry, true));
    d->loadFullImageSize->setChecked(group.readEntry(d->configLoadFullImagesizeEntry,     false));
    d->clearOnClose->setChecked(group.readEntry(d->configClearOnCloseEntry,               false));
}

void SetupLightTable::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configFullScreenHideToolBarEntry, d->hideToolBar->isChecked());
    group.writeEntry(d->configAutoSyncPreviewEntry,       d->autoSyncPreview->isChecked());
    group.writeEntry(d->configAutoLoadRightPanelEntry,    d->autoLoadOnRightPanel->isChecked());
    group.writeEntry(d->configLoadFullImagesizeEntry,     d->loadFullImageSize->isChecked());
    group.writeEntry(d->configClearOnCloseEntry,          d->clearOnClose->isChecked());
    config->sync();
}

}  // namespace Digikam
