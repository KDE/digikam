/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 
 * Description :
 *
 * Copyright (C) 2013-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "qualityscandialog.moc"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRadioButton>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandardguiitem.h>

// Local includes

#include "albummodel.h"
#include "albumselectcombobox.h"
#include "albumsettings.h"
#include "albumtreeview.h"
#include "searchutilities.h"

namespace Digikam
{  
class ButtonExtendedLabel : public QLabel
{
public:
dfgfdgdgd
    explicit ButtonExtendedLabel(QWidget* const parent = 0)
        : QLabel(parent), m_button(0)
    {
    }

    ButtonExtendedLabel(const QString& text, QWidget* const parent = 0)
        : QLabel(text, parent), m_button(0)
    {
    }

    void setButton(QAbstractButton* const button)
    {
        setBuddy(button);
        m_button = button;
    }

protected:
    //from face management tool
    // quick & dirty workaround the fact that QRadioButton does not provide a decent label
    void mouseReleaseEvent(QMouseEvent*)
    {
        if (m_button)
        {
            m_button->toggle();
        }
    }

    QAbstractButton* m_button;
};



class QualityScanDialog::Private
{
public:

    Private()
        : configName("Image Quality Sorting Dialog"),
          configMainTask("Image Scan Main Task"),
          configValueSortByQuality("Detect image quality"),
          configAlreadyScannedHandling("Already Scanned Handling"),
          configUseFullCpu("Use Full CPU"),
          configSettingsVisible("Settings Widget Visible")
    {
        optionGroupBox           = 0;
        sortByQualityButton = 0;
        alreadyScannedBox        = 0;
        tabWidget                = 0;
        albumSelectCB            = 0;
        tagSelectCB              = 0;
        albumClearButton         = 0;
        tagClearButton           = 0;
        parametersResetButton    = 0;
        accuracyInput            = 0;
        useFullCpuButton         = 0;
        benchmarkButton          = 0;
    }       
    
    QGroupBox*                   optionGroupBox;
    QRadioButton*                sortByQualityButton;
    QComboBox*                   alreadyScannedBox;

    QTabWidget*                  tabWidget;

    AlbumTreeViewSelectComboBox* albumSelectCB;
    TagTreeViewSelectComboBox*   tagSelectCB;
    ModelClearButton*            albumClearButton;
    ModelClearButton*            tagClearButton;

    QToolButton*                 parametersResetButton;
    KIntNumInput*                accuracyInput;

    QCheckBox*                   useFullCpuButton;
    QCheckBox*                   benchmarkButton;

    const QString                configName;
    const QString                configMainTask;
    const QString                configValueSortByQuality;
    const QString                configAlreadyScannedHandling;
    const QString                configUseFullCpu;
    const QString                configSettingsVisible;
};

QualityScanDialog::QualityScanDialog(QWidget* const parent)
    : KDialog(parent), StateSavingObject(this),
      d(new Private)
{
    setButtons(Ok | Cancel | Details);r
    setDefaultButton(Ok);
    setCaption(i18nc("@title:window", "Processing images"));
    setButtonText(Ok, i18nc("@action:button", "Scan"));
    setButtonGuiItem(Details, KStandardGuiItem::configure());
    setButtonText(Details, i18nc("@action:button", "Options"));
    showButtonSeparator(true);

    setupUi();
    setupConnections();

    setObjectName(d->configName);
    d->albumSelectCB->view()->setObjectName(d->configName);
    d->albumSelectCB->view()->setEntryPrefix("AlbumComboBox-");
    d->albumSelectCB->view()->setRestoreCheckState(true);
    d->tagSelectCB->view()->setObjectName(d->configName);
    d->tagSelectCB->view()->setEntryPrefix("TagComboBox-");
    d->tagSelectCB->view()->setRestoreCheckState(true);
    loadState();

    updateClearButtons();
}


QualityScanDialog::~QualityScanDialog()
{
    delete d;
}

void QualityScanDialog::setDetectionDefaultParameters()
{
    d->accuracyInput->setValue(80);
}

void QualityScanDialog::doLoadState()
{
    kDebug() << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();
    QString mainTask   = group.readEntry(entryName(d->configMainTask), d->configValueDetectAndRecognize);

    if (mainTask == d->configValueRecognizedMarkedFaces)
    {
        d->reRecognizeButton->setChecked(true);
    }
    else // if (mainTask == d->configValueDetectAndRecognize)
    {
        d->detectAndRecognizeButton->setChecked(true);
    }

    QualityScanSettings::AlreadyScannedHandling handling;
    QString skipHandling = group.readEntry(entryName(d->configAlreadyScannedHandling), "Skip");

    if (skipHandling == "Rescan")
    {
        handling = QualityScanSettings::Rescan;
    }
    else if (skipHandling == "Merge")
    {
        handling = QualityScanSettings::Merge;
    }
    else //if (skipHandling == "Skip")
    {
        handling = QualityScanSettings::Skip;
    }

    d->alreadyScannedBox->setCurrentIndex(d->alreadyScannedBox->findData(handling));

    d->accuracyInput->setValue(AlbumSettings::instance()->getFaceDetectionAccuracy() * 100);

    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();

    d->useFullCpuButton->setChecked(group.readEntry(entryName(d->configUseFullCpu), true));

    // do not load and benchmarkButton state from config, dangerous

    setDetailsWidgetVisible(group.readEntry(entryName(d->configSettingsVisible), false));
}

void QualityScanDialog::doSaveState()
{
    kDebug() << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();

    QString mainTask;

    if (d->detectAndRecognizeButton->isChecked())
    {
        mainTask = d->configValueDetectAndRecognize;
    }
    else // if (d->reRecognizeButton->isChecked())
    {
        mainTask = d->configValueRecognizedMarkedFaces;
    }

    group.writeEntry(entryName(d->configMainTask), mainTask);

    QString handling;

    switch ((QualityScanSettings::AlreadyScannedHandling)
            d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt())
    {
        case QualityScanSettings::Skip:
            handling = "Skip";
            break;

        case QualityScanSettings::Rescan:
            handling = "Rescan";
            break;

        case QualityScanSettings::Merge:
            handling = "Merge";
            break;
    }

    group.writeEntry(entryName(d->configAlreadyScannedHandling), handling);

    AlbumSettings::instance()->setFaceDetectionAccuracy(double(d->accuracyInput->value()) / 100);

    d->albumSelectCB->view()->saveState();
    d->tagSelectCB->view()->saveState();

    group.writeEntry(entryName(d->configUseFullCpu), d->useFullCpuButton->isChecked());
    group.writeEntry(entryName(d->configSettingsVisible), isDetailsWidgetVisible());
}





void QualityScanDialog::setupUi()
{
    // --- Main Widget ---

    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout;

    // ---- Introductory labels ----

    QLabel* const qualityIcon   = new QLabel;
    qualityIcon->setPixmap(SmallIcon("edit-image-face-show", KIconLoader::SizeLarge));

    QLabel* const introduction = new QLabel;
    introduction->setText(i18nc("@info",
                                "digiKam can sort images based on quality.<nl/> " ));

    // ---- Main option box ----

    d->optionGroupBox               = new QGroupBox;
    QGridLayout* const optionLayout = new QGridLayout;

    d->sortByQualityButton                        = new QRadioButton(i18nc("@option:radio", "Sort images by quality"));
    ButtonExtendedLabel* const sortByQualityLabel = new ButtonExtendedLabel;
    sortByQualityLabel ->setText(i18nc("@info",
                                           "Find all faces in your photos<nl/> and try to recognize "
                                           "which person is depicted"));
    //sortByQualityLabel->setWordWrap(true);
    sortByQualityLabel->setButton(d->sortByQualityButton);
    ButtonExtendedLabel* const sortByQualityIcon  = new ButtonExtendedLabel;
   // sortByQualityIcon->setPixmap(SmallIcon("edit-image-face-detect", KIconLoader::SizeLarge));
    sortByQualityIcon->setButton(d->sortByQualityButton);
    sortByQualityIcon->setAlignment(Qt::AlignCenter);
    d->alreadyScannedBox                               = new QComboBox;
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Skip images already scanned"), QualityScanSettings::Skip);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Scan again and merge results"), QualityScanSettings::Merge);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Clear unconfirmed results and rescan"), QualityScanSettings::Rescan);
    d->alreadyScannedBox->setCurrentIndex(QualityScanSettings::Skip);
    QGridLayout* const sortByQualityLabelLayout   = new QGridLayout;
    sortByQualityLabelLayout->addWidget(sortByQualityLabel, 0, 0, 1, -1);
    sortByQualityLabelLayout->setColumnMinimumWidth(0, 10);
    sortByQualityLabelLayout->addWidget(d->alreadyScannedBox, 1, 1);

    optionLayout->addWidget(d->sortByQualityButton,   0, 0, 1, 2);
    optionLayout->addWidget(sortByQualityIcon,        0, 2, 2, 1);
    optionLayout->addLayout(sortByQualityLabelLayout, 1, 1);
    
    QStyleOptionButton buttonOption;
    buttonOption.initFrom(d->sortByQualityButton);
    int indent = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &buttonOption, d->sortByQualityButton).width();
    optionLayout->setColumnMinimumWidth(0, indent);

    d->optionGroupBox->setLayout(optionLayout);

    // ---

    mainLayout->addWidget(personIcon,        0, 0);
    mainLayout->addWidget(introduction,      0, 1);
    mainLayout->addWidget(d->optionGroupBox, 1, 0, 1, -1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setRowStretch(2, 1);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    // --- Tab Widget ---

    d->tabWidget = new QTabWidget;

    // ---- Album tab ----

    QWidget* const selectAlbumsTab        = new QWidget;
    QGridLayout* const selectAlbumsLayout = new QGridLayout;

    QLabel* const includeAlbumsLabel      = new QLabel(i18nc("@label", "Search in:"));
    d->albumSelectCB                      = new AlbumTreeViewSelectComboBox();
    //d->albumSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be included in the quality scan."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    d->albumSelectCB->addCheckUncheckContextMenuActions();

    d->albumClearButton = new ModelClearButton(d->albumSelectCB->view()->albumModel());
    d->albumClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    d->tagSelectCB      = new TagTreeViewSelectComboBox();
    //d->tagSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be included in the quality scan."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any tags"));
    d->tagSelectCB->addCheckUncheckContextMenuActions();

    d->tagClearButton   = new ModelClearButton(d->tagSelectCB->view()->albumModel());
    d->tagClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected tags"));

    selectAlbumsLayout->addWidget(includeAlbumsLabel, 0, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->albumSelectCB,   1, 0);
    selectAlbumsLayout->addWidget(d->albumClearButton, 1, 1);
    selectAlbumsLayout->addWidget(d->tagSelectCB,     2, 0);
    selectAlbumsLayout->addWidget(d->tagClearButton,  2, 1);
    selectAlbumsLayout->setRowStretch(3, 1);

    selectAlbumsTab->setLayout(selectAlbumsLayout);
    d->tabWidget->addTab(selectAlbumsTab, i18nc("@title:tab", "Albums"));

    // ---- Parameters tab ----

    QWidget* const parametersTab        = new QWidget;
    QGridLayout* const parametersLayout = new QGridLayout;

    QLabel* const detectionLabel        = new QLabel(i18nc("@label", "Parameters for Image Quality Sorter"));

    d->parametersResetButton            = new QToolButton;
    d->parametersResetButton->setAutoRaise(true);
    d->parametersResetButton->setFocusPolicy(Qt::NoFocus);
    d->parametersResetButton->setIcon(SmallIcon("document-revert"));
    d->parametersResetButton->setToolTip(i18nc("@action:button", "Reset to default values"));

    d->accuracyInput                    = new KIntNumInput;
    d->accuracyInput->setRange(0, 100, 10);
    d->accuracyInput->setSliderEnabled();
    d->accuracyInput->setLabel(i18nc("@label Two extremities of a scale", "Fast   -   Accurate"),
                               Qt::AlignTop | Qt::AlignHCenter);
    d->accuracyInput->setToolTip(i18nc("@info:tooltip",
                                       "Adjust speed versus accuracy: The higher the value, the more accurate the results "
                                       "will be, but it will take more time."));

    parametersLayout->addWidget(detectionLabel, 0, 0);
    parametersLayout->addWidget(d->parametersResetButton, 0, 1);
    parametersLayout->addWidget(d->accuracyInput, 1, 0, 1, -1);
    parametersLayout->setColumnStretch(0, 1);

    parametersTab->setLayout(parametersLayout);
    d->tabWidget->addTab(parametersTab, i18nc("@title:tab", "Parameters"));

    // ---- Advanced tab ----

    QWidget* const advancedTab        = new QWidget;
    QVBoxLayout* const advancedLayout = new QVBoxLayout;

    QLabel* const cpuExplanation      = new QLabel;
    cpuExplanation->setText(i18nc("@info",
                                  "Image Quality Sorting is a time-consuming task. "
                                  "You can choose if you wish to employ all processor cores on your system, "
                                  "or work in the background only on one core."));
    cpuExplanation->setWordWrap(true);

    d->useFullCpuButton = new QCheckBox;
    d->useFullCpuButton->setText(i18nc("@option:check", "Work on all processor cores"));

    d->benchmarkButton = new QCheckBox;
    d->benchmarkButton->setText(i18nc("@option:check", "Benchmark quality sorting"));
    d->benchmarkButton->setToolTip(i18nc("@info:tooltip",
                                         "This will run quality sorting and compare the results ");

    advancedLayout->addWidget(cpuExplanation);
    advancedLayout->addWidget(d->useFullCpuButto1n);
    advancedLayout->addWidget(new KSeparator(Qt::Horizontal));
    advancedLayout->addWidget(d->benchmarkButton);
    advancedLayout->addStretch(1);

    advancedTab->setLayout(advancedLayout);
    d->tabWidget->addTab(advancedTab, i18nc("@title:tab", "Advanced"));

    // ---

    setDetailsWidget(d->tabWidget);
}
}