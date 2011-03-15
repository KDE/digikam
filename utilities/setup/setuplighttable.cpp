/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class SetupLightTable::SetupLightTablePriv
{
public:

    SetupLightTablePriv() :
        hideToolBar(0),
        autoSyncPreview(0),
        autoLoadOnRightPanel(0),
        loadFullImageSize(0),
        clearOnClose(0)
    {}

    static const QString configGroupName;
    static const QString configFullScreenHideToolBarEntry;
    static const QString configAutoSyncPreviewEntry;
    static const QString configAutoLoadRightPanelEntry;
    static const QString configLoadFullImagesizeEntry;
    static const QString configClearOnCloseEntry;

    QCheckBox*           hideToolBar;
    QCheckBox*           autoSyncPreview;
    QCheckBox*           autoLoadOnRightPanel;
    QCheckBox*           loadFullImageSize;
    QCheckBox*           clearOnClose;
};

const QString SetupLightTable::SetupLightTablePriv::configGroupName("LightTable Settings");
const QString SetupLightTable::SetupLightTablePriv::configFullScreenHideToolBarEntry("FullScreen Hide ToolBar");
const QString SetupLightTable::SetupLightTablePriv::configAutoSyncPreviewEntry("Auto Sync Preview");
const QString SetupLightTable::SetupLightTablePriv::configAutoLoadRightPanelEntry("Auto Load Right Panel");
const QString SetupLightTable::SetupLightTablePriv::configLoadFullImagesizeEntry("Load Full Image size");
const QString SetupLightTable::SetupLightTablePriv::configClearOnCloseEntry("Clear On Close");

// --------------------------------------------------------

SetupLightTable::SetupLightTable(QWidget* parent)
    : QScrollArea(parent), d(new SetupLightTablePriv)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout* gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis(i18n("Set this option to automatically synchronize "
                                          "zooming and panning between left and right panels if the "
                                          "images have the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads the image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis( i18n("Set this option to automatically load an image "
                                                "into the right panel when the corresponding item is selected on the thumbbar."));

    d->loadFullImageSize = new QCheckBox(i18n("Load full-sized image"), interfaceOptionsGroup);
    d->loadFullImageSize->setWhatsThis( i18n("<p>Set this option to load images at their full size "
                                             "for preview, rather than at a reduced size. As this option "
                                             "will make it take longer to load images, only use it if you have "
                                             "a fast computer.</p>"
                                             "<p><b>Note:</b> for Raw images, a half size version of the Raw data "
                                             "is used instead of the embedded JPEG preview.</p>"));

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
