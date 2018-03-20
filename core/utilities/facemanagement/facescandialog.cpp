/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Dialog to choose options for face scanning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "facescandialog.h"

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRadioButton>
#include <QToolButton>
#include <QTabWidget>
#include <QIcon>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "albummodel.h"
#include "albumselectors.h"
#include "applicationsettings.h"
#include "dexpanderbox.h"

namespace Digikam
{

class FaceScanDialog::Private
{
public:

    Private()
        : configName(QLatin1String("Face Detection Dialog")),
          configMainTask(QLatin1String("Face Scan Main Task")),
          configValueDetect(QLatin1String("Detect")),
          configValueDetectAndRecognize(QLatin1String("Detect and Recognize Faces")),
          configValueRecognizedMarkedFaces(QLatin1String("Recognize Marked Faces")),
          configAlreadyScannedHandling(QLatin1String("Already Scanned Handling")),
          configUseFullCpu(QLatin1String("Use Full CPU")),
          configSettingsVisible(QLatin1String("Settings Widget Visible")),
          configRecognizeAlgorithm(QLatin1String("Recognize Algorithm")) 
    {
        buttons                    = 0;
        optionGroupBox             = 0;
        detectAndRecognizeButton   = 0;
        detectButton               = 0;
        alreadyScannedBox          = 0;
        reRecognizeButton          = 0;
        tabWidget                  = 0;
        albumSelectors             = 0;
        accuracyInput              = 0;
        useFullCpuButton           = 0;
        retrainAllButton           = 0;
        recognizeBox               = 0;
    }

    QDialogButtonBox*            buttons;

    QGroupBox*                   optionGroupBox;
    QRadioButton*                detectAndRecognizeButton;
    QRadioButton*                detectButton;
    QComboBox*                   alreadyScannedBox;
    QRadioButton*                reRecognizeButton;

    QTabWidget*                  tabWidget;

    AlbumSelectors*              albumSelectors;

    DIntNumInput*                accuracyInput;

    QCheckBox*                   useFullCpuButton;
    QCheckBox*                   retrainAllButton;

    QComboBox*                   recognizeBox;

    const QString                configName;
    const QString                configMainTask;
    const QString                configValueDetect;
    const QString                configValueDetectAndRecognize;
    const QString                configValueRecognizedMarkedFaces;
    const QString                configAlreadyScannedHandling;
    const QString                configUseFullCpu;
    const QString                configSettingsVisible;
    const QString                configRecognizeAlgorithm;
};

FaceScanDialog::FaceScanDialog(QWidget* const parent)
    : QDialog(parent),
      StateSavingObject(this),
      d(new Private)
{
    setWindowTitle(i18nc("@title:window", "Scanning faces"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Reset | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "Scan"));

    setupUi();
    setupConnections();

    setObjectName(d->configName);
    loadState();
}

FaceScanDialog::~FaceScanDialog()
{
    delete d;
}

void FaceScanDialog::doLoadState()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();
    QString mainTask   = group.readEntry(entryName(d->configMainTask),
                                         d->configValueDetect);

    if (mainTask == d->configValueRecognizedMarkedFaces)
    {
        d->reRecognizeButton->setChecked(true);
    }
    else if (mainTask == d->configValueDetectAndRecognize)
    {
        d->detectAndRecognizeButton->setChecked(true);
    }
    else
    {
        d->detectButton->setChecked(true);
    }

    FaceScanSettings::AlreadyScannedHandling handling;
    QString skipHandling = group.readEntry(entryName(d->configAlreadyScannedHandling),
                                           QString::fromLatin1("Skip"));

    if (skipHandling == QLatin1String("Rescan"))
    {
        handling = FaceScanSettings::Rescan;
    }
    else if (skipHandling == QLatin1String("Merge"))
    {
        handling = FaceScanSettings::Merge;
    }
    else // Skip
    {
        handling = FaceScanSettings::Skip;
    }

    d->alreadyScannedBox->setCurrentIndex(d->alreadyScannedBox->findData(handling));

    d->accuracyInput->setValue(ApplicationSettings::instance()->getFaceDetectionAccuracy() * 100);

    d->albumSelectors->loadState();

    d->useFullCpuButton->setChecked(group.readEntry(entryName(d->configUseFullCpu), false));

    RecognitionDatabase::RecognizeAlgorithm algo = (RecognitionDatabase::RecognizeAlgorithm)group.readEntry(entryName(d->configRecognizeAlgorithm),
                                                                                                            (int)RecognitionDatabase::RecognizeAlgorithm::DNN);

    d->recognizeBox->setCurrentIndex(d->recognizeBox->findData(algo));

    // do not load retrainAllButton state from config, dangerous

    d->tabWidget->setVisible(group.readEntry(entryName(d->configSettingsVisible), false));
    adjustDetailsButton(d->tabWidget->isVisible());
}

void FaceScanDialog::doSaveState()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << getConfigGroup().name();

    KConfigGroup group = getConfigGroup();

    QString mainTask;

    if (d->detectButton->isChecked())
    {
        mainTask = d->configValueDetect;
    }
    else if (d->detectAndRecognizeButton->isChecked())
    {
        mainTask = d->configValueDetectAndRecognize;
    }
    else // d->reRecognizeButton
    {
        mainTask = d->configValueRecognizedMarkedFaces;
    }

    group.writeEntry(entryName(d->configMainTask), mainTask);

    QString handling;

    switch ((FaceScanSettings::AlreadyScannedHandling)(d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt()))
    {
        case FaceScanSettings::Skip:
            handling = QLatin1String("Skip");
            break;

        case FaceScanSettings::Rescan:
            handling = QLatin1String("Rescan");
            break;

        case FaceScanSettings::Merge:
            handling = QLatin1String("Merge");
            break;
    }

    group.writeEntry(entryName(d->configAlreadyScannedHandling), handling);

    ApplicationSettings::instance()->setFaceDetectionAccuracy(double(d->accuracyInput->value()) / 100);
    d->albumSelectors->saveState();

    group.writeEntry(entryName(d->configUseFullCpu),         d->useFullCpuButton->isChecked());
    group.writeEntry(entryName(d->configSettingsVisible),    d->tabWidget->isVisible());
    group.writeEntry(entryName(d->configRecognizeAlgorithm), d->recognizeBox->itemData(d->recognizeBox->currentIndex()));
}

void FaceScanDialog::setupUi()
{
    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout;
    d->tabWidget                  = new QTabWidget;

    // ---- Introductory labels ----

    QLabel* const personIcon   = new QLabel;
    personIcon->setPixmap(QIcon::fromTheme(QLatin1String("edit-image-face-show")).pixmap(48));

    QLabel* const introduction = new QLabel;
    introduction->setTextFormat(Qt::RichText);
    introduction->setText(i18nc("@info",
                                "<qt>digiKam can search for faces in your photos.<br> "
                                "When you have identified your friends on a number of photos,<br> "
                                "it can also recognize the people shown on your photos.</qt>"));

    // ---- Main option box --------

    d->optionGroupBox               = new QGroupBox;
    QGridLayout* const optionLayout = new QGridLayout;

    d->detectButton                 = new QRadioButton(i18nc("@option:radio", "Detect faces"));
    d->detectButton->setToolTip(i18nc("@info", "Find all faces in your photos"));

    d->detectAndRecognizeButton     = new QRadioButton(i18nc("@option:radio", "Detect and recognize faces (experimental)"));
    d->detectAndRecognizeButton->setToolTip(i18nc("@info", "Find all faces in your photos and try to recognize which person is depicted"));

    d->alreadyScannedBox            = new QComboBox;
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Skip images already scanned"),          FaceScanSettings::Skip);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Scan again and merge results"),         FaceScanSettings::Merge);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->alreadyScannedBox->setCurrentIndex(FaceScanSettings::Skip);

    d->reRecognizeButton            = new QRadioButton(i18nc("@option:radio", "Recognize faces (experimental)"));
    d->reRecognizeButton->setToolTip(i18nc("@info", "Try again to recognize the people depicted on marked but yet unconfirmed faces."));

    optionLayout->addWidget(d->alreadyScannedBox,          0, 0, 1, 2);
    optionLayout->addWidget(d->detectButton,               1, 0, 1, 2);
    optionLayout->addWidget(d->detectAndRecognizeButton,   2, 0, 1, 2);
    optionLayout->addWidget(d->reRecognizeButton,          3, 0, 1, 2);

    QStyleOptionButton buttonOption;
    buttonOption.initFrom(d->detectAndRecognizeButton);
    int indent = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &buttonOption, d->detectAndRecognizeButton).width();
    optionLayout->setColumnMinimumWidth(0, indent);

    d->optionGroupBox->setLayout(optionLayout);

    // ------------------------

    mainLayout->addWidget(personIcon,        0, 0);
    mainLayout->addWidget(introduction,      0, 1);
    mainLayout->addWidget(d->optionGroupBox, 1, 0, 1, -1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setRowStretch(2, 1);
    mainWidget->setLayout(mainLayout);

    // ---- Album tab ---------

    d->albumSelectors = new AlbumSelectors(i18nc("@label", "Search in:"), d->configName, d->tabWidget);
    d->tabWidget->addTab(d->albumSelectors, i18nc("@title:tab", "Albums"));

    // ---- Parameters tab ----

    QWidget* const parametersTab        = new QWidget(d->tabWidget);
    QGridLayout* const parametersLayout = new QGridLayout(parametersTab);

    QLabel* const detectionLabel        = new QLabel(i18nc("@label", "Parameters for face detection and Recognition"), parametersTab);

    QLabel* const accuracyLabel         = new QLabel(i18nc("@label Two extremities of a scale", "Fast     -     Accurate"), parametersTab);
    accuracyLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    d->accuracyInput                    = new DIntNumInput(parametersTab);
    d->accuracyInput->setDefaultValue(80);
    d->accuracyInput->setRange(0, 100, 10);
    d->accuracyInput->setToolTip(i18nc("@info:tooltip",
                                       "Adjust speed versus accuracy: The higher the value, the more accurate the results "
                                       "will be, but it will take more time."));

    parametersLayout->addWidget(detectionLabel,   0, 0, 1, 1);
    parametersLayout->addWidget(d->accuracyInput, 1, 0, 1, 1);
    parametersLayout->addWidget(accuracyLabel,    2, 0, 1, 1);
    parametersLayout->setColumnStretch(0, 10);
    parametersLayout->setRowStretch(3, 10);

    d->tabWidget->addTab(parametersTab, i18nc("@title:tab", "Parameters"));

    // ---- Advanced tab ------

    QWidget* const advancedTab        = new QWidget(d->tabWidget);
    QGridLayout* const advancedLayout = new QGridLayout(advancedTab);

    QLabel* const cpuExplanation      = new QLabel(advancedTab);
    cpuExplanation->setText(i18nc("@info",
                                  "Face detection is a time-consuming task. "
                                  "You can choose if you wish to employ all processor cores "
                                  "on your system, or work in the background only on one core. "
                                  "Warning: this features still experimental and it's disabled by default."));
    cpuExplanation->setWordWrap(true);

    d->useFullCpuButton = new QCheckBox(advancedTab);
    d->useFullCpuButton->setText(i18nc("@option:check", "Work on all processor cores (experimental)"));

    // ---- Recognize algorithm ComboBox -----

    d->recognizeBox        = new QComboBox;
    d->recognizeBox->addItem(i18nc("@label:listbox", "Recognize faces using LBP algorithm"),           RecognitionDatabase::RecognizeAlgorithm::LBP);
    d->recognizeBox->addItem(i18nc("@label:listbox", "Recognize faces using EigenFaces algorithm"),    RecognitionDatabase::RecognizeAlgorithm::EigenFace);
    d->recognizeBox->addItem(i18nc("@label:listbox", "Recognize faces using FisherFaces algorithm"),   RecognitionDatabase::RecognizeAlgorithm::FisherFace);
    d->recognizeBox->addItem(i18nc("@label:listbox", "Recognize faces using Deep Learning algorithm"), RecognitionDatabase::RecognizeAlgorithm::DNN);
    d->recognizeBox->setCurrentIndex(RecognitionDatabase::RecognizeAlgorithm::DNN);

    d->retrainAllButton = new QCheckBox(advancedTab);
    d->retrainAllButton->setText(i18nc("@option:check", "Clear and rebuild all training data"));
    d->retrainAllButton->setToolTip(i18nc("@info:tooltip",
                                          "This will clear all training data for recognition "
                                          "and rebuild it from all available faces."));

    advancedLayout->addWidget(cpuExplanation,                  0, 0);
    advancedLayout->addWidget(d->useFullCpuButton,             1, 0);
    advancedLayout->addWidget(new DLineWidget(Qt::Horizontal), 2, 0);
    advancedLayout->addWidget(d->retrainAllButton,             3, 0);
    advancedLayout->addWidget(d->recognizeBox,                 4, 0);
    advancedLayout->setRowStretch(5, 10);

    d->tabWidget->addTab(advancedTab, i18nc("@title:tab", "Advanced"));

    // ------------------------

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(d->tabWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
}

void FaceScanDialog::setupConnections()
{
    connect(d->detectButton, SIGNAL(toggled(bool)),
            d->alreadyScannedBox, SLOT(setEnabled(bool)));

    connect(d->detectAndRecognizeButton, SIGNAL(toggled(bool)),
            d->alreadyScannedBox, SLOT(setEnabled(bool)));

    connect(d->retrainAllButton, SIGNAL(toggled(bool)),
            this, SLOT(retrainAllButtonToggled(bool)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(slotDetails()));
}

void FaceScanDialog::slotDetails()
{
    bool on = !d->tabWidget->isVisible();
    d->tabWidget->setVisible(on);
    adjustSize();
    adjustDetailsButton(on);
}

void FaceScanDialog::adjustDetailsButton(bool on)
{
    d->buttons->button(QDialogButtonBox::Reset)->setText(on ? i18nc("@action:button", "Options <<")
                                                            : i18nc("@action:button", "Options >>"));
}

void FaceScanDialog::slotOk()
{
    accept();
    saveState();
}

void FaceScanDialog::retrainAllButtonToggled(bool on)
{
    d->optionGroupBox->setEnabled(!on);
    d->albumSelectors->setEnabled(!on);
    d->recognizeBox->setEnabled(!on);
}

FaceScanSettings FaceScanDialog::settings() const
{
    FaceScanSettings settings;

    if (d->retrainAllButton->isChecked())
    {
        settings.task = FaceScanSettings::RetrainAll;
    }
    else if(d->detectButton->isChecked())
    {
        settings.task = FaceScanSettings::Detect;
    }
    else
    {
        if (d->detectAndRecognizeButton->isChecked())
        {
            settings.task = FaceScanSettings::DetectAndRecognize;
        }
        else
        {
            settings.task = FaceScanSettings::RecognizeMarkedFaces;
        }
    }

    settings.alreadyScannedHandling = (FaceScanSettings::AlreadyScannedHandling)
                                      d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt();

    settings.accuracy               = double(d->accuracyInput->value()) / 100;

    settings.albums << d->albumSelectors->selectedAlbumsAndTags();

    settings.useFullCpu             = d->useFullCpuButton->isChecked();

    settings.recognizeAlgorithm     = (RecognitionDatabase::RecognizeAlgorithm)
                                      d->recognizeBox->itemData(d->recognizeBox->currentIndex()).toInt();

    return settings;
}

} // namespace Digikam
