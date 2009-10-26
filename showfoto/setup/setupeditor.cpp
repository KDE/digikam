/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup showFoto tab.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Arnd Baecker <arnd dot baecker at web dot de>
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

#include "setupeditor.h"
#include "setupeditor.moc"

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmultitabbar.h>
#include <knuminput.h>
#include <kvbox.h>

namespace ShowFoto
{

class SetupEditorPriv
{
public:

    SetupEditorPriv() :
        configGroupName("ImageViewer Settings"),
        configUseThemeBackgroundColorEntry("UseThemeBackgroundColor"),
        configBackgroundColorEntry("BackgroundColor"),
        configFullScreenHideToolBarEntry("FullScreen Hide ToolBar"),
        configFullScreenHideThumbBarEntry("FullScreenHideThumbBar"),
        configDeleteItem2TrashEntry("DeleteItem2Trash"),
        configShowSplashEntry("ShowSplash"),
        configSidebarTitleStyleEntry("Sidebar Title Style"),
        configUnderExposureColorEntry("UnderExposureColor"),
        configOverExposureColorEntry("OverExposureColor"),
        configSortOrderEntry("SortOrder"),
        configReverseSortEntry("ReverseSort"),
        configUseRawImportToolEntry("UseRawImportTool"),

        sidebarTypeLabel(0),
        hideThumbBar(0),
        hideToolBar(0),
        showSplash(0),
        sortReverse(0),
        themebackgroundColor(0),
        useRawImportTool(0),
        useTrash(0),
        colorBox(0),
        sidebarType(0),
        sortOrderComboBox(0),
        backgroundColor(0),
        overExposureColor(0),
        underExposureColor(0)
        {}

    const QString configGroupName;
    const QString configUseThemeBackgroundColorEntry;
    const QString configBackgroundColorEntry;
    const QString configFullScreenHideToolBarEntry;
    const QString configFullScreenHideThumbBarEntry;
    const QString configDeleteItem2TrashEntry;
    const QString configShowSplashEntry;
    const QString configSidebarTitleStyleEntry;
    const QString configUnderExposureColorEntry;
    const QString configOverExposureColorEntry;
    const QString configSortOrderEntry;
    const QString configReverseSortEntry;
    const QString configUseRawImportToolEntry;

    QLabel*       sidebarTypeLabel;

    QCheckBox*    hideThumbBar;
    QCheckBox*    hideToolBar;
    QCheckBox*    showSplash;
    QCheckBox*    sortReverse;
    QCheckBox*    themebackgroundColor;
    QCheckBox*    useRawImportTool;
    QCheckBox*    useTrash;

    KHBox*        colorBox;

    KComboBox*    sidebarType;
    KComboBox*    sortOrderComboBox;

    KColorButton* backgroundColor;
    KColorButton* overExposureColor;
    KColorButton* underExposureColor;
};

SetupEditor::SetupEditor(QWidget* parent)
           : QScrollArea(parent), d(new SetupEditorPriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout *gLayout1            = new QVBoxLayout();

    d->themebackgroundColor = new QCheckBox(i18n("&Use current theme background color"),
                                            interfaceOptionsGroup);

    d->themebackgroundColor->setWhatsThis(i18n("Enable this option to use the current background theme "
                                               "color in the image editor area"));

    d->colorBox                  = new KHBox(interfaceOptionsGroup);
    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), d->colorBox);
    d->backgroundColor           = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    d->backgroundColor->setWhatsThis(i18n("Select background color to use "
                                          "for image editor area."));

    d->hideToolBar        = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);
    d->hideThumbBar       = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), interfaceOptionsGroup);
    d->useTrash   = new QCheckBox(i18n("&Deleted items should go to the trash"), interfaceOptionsGroup);
    d->showSplash = new QCheckBox(i18n("&Show splash screen at startup"), interfaceOptionsGroup);

    d->useRawImportTool = new QCheckBox(i18n("Use Raw Import Tool to handle Raw images"), interfaceOptionsGroup);
    d->useRawImportTool->setWhatsThis(i18n("Set this option to use Raw Import "
                                           "tool to load a RAW image. "
                                           "With this tool you are able to customize advanced settings."));

    KHBox *hbox = new KHBox(interfaceOptionsGroup);
    d->sidebarTypeLabel  = new QLabel(i18n("Sidebar tab title:"), hbox);
    d->sidebarType       = new KComboBox(hbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    gLayout1->addWidget(d->themebackgroundColor);
    gLayout1->addWidget(d->colorBox);
    gLayout1->addWidget(d->hideToolBar);
    gLayout1->addWidget(d->hideThumbBar);
    gLayout1->addWidget(d->useTrash);
    gLayout1->addWidget(d->showSplash);
    gLayout1->addWidget(d->useRawImportTool);
    gLayout1->addWidget(hbox);

    interfaceOptionsGroup->setLayout(gLayout1);

    // --------------------------------------------------------

    QGroupBox *exposureOptionsGroup = new QGroupBox(i18n("Exposure Indicators"), panel);
    QVBoxLayout *gLayout2           = new QVBoxLayout();

    KHBox *underExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *underExpoColorlabel = new QLabel( i18n("&Under-exposure color:"), underExpoBox);
    d->underExposureColor       = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    d->underExposureColor->setWhatsThis( i18n("Customize the color used in the image editor to identify "
                                              "under-exposed pixels.") );

    KHBox *overExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    d->overExposureColor->setWhatsThis( i18n("Customize the color used in the image editor to identify "
                                             "over-exposed pixels.") );

    gLayout2->addWidget(underExpoBox);
    gLayout2->addWidget(overExpoBox);
    exposureOptionsGroup->setLayout(gLayout2);

    // --------------------------------------------------------

    QGroupBox *sortOptionsGroup = new QGroupBox(i18n("Sort order for images"), panel);
    QVBoxLayout *gLayout4       = new QVBoxLayout();

    KHBox* sortBox       = new KHBox(sortOptionsGroup);
    new QLabel(i18n("Sort images by:"), sortBox);
    d->sortOrderComboBox = new KComboBox(sortBox);
    d->sortOrderComboBox->insertItem(0, i18nc("sort images by date", "Date"));
    d->sortOrderComboBox->insertItem(1, i18nc("sort images by name", "Name"));
    d->sortOrderComboBox->insertItem(2, i18nc("sort images by size", "File Size"));
    d->sortOrderComboBox->setWhatsThis(i18n("Here, select whether newly-loaded "
                                            "images are sorted by their date, name, or size on disk."));

    d->sortReverse = new QCheckBox(i18n("Reverse ordering"), sortOptionsGroup);
    d->sortReverse->setWhatsThis(i18n("If this option is enabled, newly-loaded "
                                      "images will be sorted in descending order."));

    gLayout4->addWidget(sortBox);
    gLayout4->addWidget(d->sortReverse);
    sortOptionsGroup->setLayout(gLayout4);

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(exposureOptionsGroup);
    layout->addWidget(sortOptionsGroup);
    layout->addStretch();
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->themebackgroundColor, SIGNAL(toggled(bool)),
            this, SLOT(slotThemeBackgroundColor(bool)));

    readSettings();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::slotThemeBackgroundColor(bool e)
{
    d->colorBox->setEnabled(!e);
}

void SetupEditor::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->themebackgroundColor->setChecked(group.readEntry(d->configUseThemeBackgroundColorEntry, true));
    d->backgroundColor->setColor(group.readEntry(d->configBackgroundColorEntry,                Black));
    d->hideToolBar->setChecked(group.readEntry(d->configFullScreenHideToolBarEntry,            false));
    d->hideThumbBar->setChecked(group.readEntry(d->configFullScreenHideThumbBarEntry,          true));
    d->useTrash->setChecked(group.readEntry(d->configDeleteItem2TrashEntry,                    false));
    d->showSplash->setChecked(group.readEntry(d->configShowSplashEntry,                        true));
    d->sidebarType->setCurrentIndex(group.readEntry(d->configSidebarTitleStyleEntry,           0));
    d->underExposureColor->setColor(group.readEntry(d->configUnderExposureColorEntry,          White));
    d->overExposureColor->setColor(group.readEntry(d->configOverExposureColorEntry,            Black));
    d->sortOrderComboBox->setCurrentIndex(group.readEntry(d->configSortOrderEntry,             0));
    d->sortReverse->setChecked(group.readEntry(d->configReverseSortEntry,                      false));
    d->useRawImportTool->setChecked(group.readEntry(d->configUseRawImportToolEntry,            false));
}

void SetupEditor::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configUseThemeBackgroundColorEntry, d->themebackgroundColor->isChecked());
    group.writeEntry(d->configBackgroundColorEntry,         d->backgroundColor->color());
    group.writeEntry(d->configFullScreenHideToolBarEntry,   d->hideToolBar->isChecked());
    group.writeEntry(d->configFullScreenHideThumbBarEntry,  d->hideThumbBar->isChecked());
    group.writeEntry(d->configDeleteItem2TrashEntry,        d->useTrash->isChecked());
    group.writeEntry(d->configShowSplashEntry,              d->showSplash->isChecked());
    group.writeEntry(d->configSidebarTitleStyleEntry,       d->sidebarType->currentIndex());
    group.writeEntry(d->configUnderExposureColorEntry,      d->underExposureColor->color());
    group.writeEntry(d->configOverExposureColorEntry,       d->overExposureColor->color());
    group.writeEntry(d->configSortOrderEntry,               d->sortOrderComboBox->currentIndex());
    group.writeEntry(d->configReverseSortEntry,             d->sortReverse->isChecked());
    group.writeEntry(d->configUseRawImportToolEntry,        d->useRawImportTool->isChecked());
    config->sync();
}

}   // namespace ShowFoto
