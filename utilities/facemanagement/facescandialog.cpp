/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Dialog to choose options for face scanning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "facescandialog.moc"

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
#include <QVBoxLayout>

// KDE includes

#include <ktabwidget.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandardguiitem.h>

// Local includes

#include "albummodel.h"
#include "albumselectors.h"
#include "applicationsettings.h"

namespace Digikam
{

class ButtonExtendedLabel : public QLabel
{
public:

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

// ------------------------------------------------------------------------------------------

class FaceScanDialog::Private
{
public:

    Private()
        : configName("Face Detection Dialog"),
          configMainTask("Face Scan Main Task"),
          configValueDetect("Detect"),
          configValueDetectAndRecognize("Detect and Recognize Faces"),
          configValueRecognizedMarkedFaces("Recognize Marked Faces"),
          configAlreadyScannedHandling("Already Scanned Handling"),
          configUseFullCpu("Use Full CPU"),
          configSettingsVisible("Settings Widget Visible")
    {
        optionGroupBox           = 0;
        detectAndRecognizeButton = 0;
        detectButton             = 0;
        alreadyScannedBox        = 0;
        reRecognizeButton        = 0;
        tabWidget                = 0;
        parametersResetButton    = 0;
        albumSelectors           = 0;
        accuracyInput            = 0;
        useFullCpuButton         = 0;
        retrainAllButton         = 0;
        benchmarkDetectionButton   = 0;
        benchmarkRecognitionButton = 0;
    }

    QGroupBox*                   optionGroupBox;
    QRadioButton*                detectAndRecognizeButton;
    QRadioButton*                detectButton;
    QComboBox*                   alreadyScannedBox;
    QRadioButton*                reRecognizeButton;

    KTabWidget*                  tabWidget;

    AlbumSelectors*              albumSelectors;

    QToolButton*                 parametersResetButton;
    KIntNumInput*                accuracyInput;

    QCheckBox*                   useFullCpuButton;
    QCheckBox*                   retrainAllButton;
    QCheckBox*                   benchmarkDetectionButton;
    QCheckBox*                   benchmarkRecognitionButton;

    const QString                configName;
    const QString                configMainTask;
    const QString                configValueDetect;
    const QString                configValueDetectAndRecognize;
    const QString                configValueRecognizedMarkedFaces;
    const QString                configAlreadyScannedHandling;
    const QString                configUseFullCpu;
    const QString                configSettingsVisible;
};

FaceScanDialog::FaceScanDialog(QWidget* const parent)
    : KDialog(parent), StateSavingObject(this),
      d(new Private)
{
    setButtons(Ok | Cancel | Details);
    setDefaultButton(Ok);
    setCaption(i18nc("@title:window", "Scanning faces"));
    setButtonText(Ok, i18nc("@action:button", "Scan"));
    setButtonGuiItem(Details, KStandardGuiItem::configure());
    setButtonText(Details, i18nc("@action:button", "Options"));
    showButtonSeparator(true);

    setupUi();
    setupConnections();

    setObjectName(d->configName);
    loadState();
}

FaceScanDialog::~FaceScanDialog()
{
    delete d;
}

void FaceScanDialog::accept()
{
    KDialog::accept();
    saveState();
}

void FaceScanDialog::setDetectionDefaultParameters()
{
    d->accuracyInput->setValue(80);
}

void FaceScanDialog::doLoadState()
{
    kDebug() << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();
    QString mainTask   = group.readEntry(entryName(d->configMainTask), d->configValueDetectAndRecognize);

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
    QString skipHandling = group.readEntry(entryName(d->configAlreadyScannedHandling), "Skip");

    if (skipHandling == "Rescan")
    {
        handling = FaceScanSettings::Rescan;
    }
    else if (skipHandling == "Merge")
    {
        handling = FaceScanSettings::Merge;
    }
    else //if (skipHandling == "Skip")
    {
        handling = FaceScanSettings::Skip;
    }

    d->alreadyScannedBox->setCurrentIndex(d->alreadyScannedBox->findData(handling));

    d->accuracyInput->setValue(ApplicationSettings::instance()->getFaceDetectionAccuracy() * 100);

    d->albumSelectors->loadState();

    d->useFullCpuButton->setChecked(group.readEntry(entryName(d->configUseFullCpu), true));

    // do not load retrainAllButton and benchmarkDetectionButton state from config, dangerous

    setDetailsWidgetVisible(group.readEntry(entryName(d->configSettingsVisible), false));
}

void FaceScanDialog::doSaveState()
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

    switch ((FaceScanSettings::AlreadyScannedHandling)
            d->alreadyScannedBox->itemData(d->alreadyScannedBox->currentIndex()).toInt())
    {
        case FaceScanSettings::Skip:
            handling = "Skip";
            break;

        case FaceScanSettings::Rescan:
            handling = "Rescan";
            break;

        case FaceScanSettings::Merge:
            handling = "Merge";
            break;
    }

    group.writeEntry(entryName(d->configAlreadyScannedHandling), handling);

    ApplicationSettings::instance()->setFaceDetectionAccuracy(double(d->accuracyInput->value()) / 100);

    d->albumSelectors->saveState();

    group.writeEntry(entryName(d->configUseFullCpu), d->useFullCpuButton->isChecked());
    group.writeEntry(entryName(d->configSettingsVisible), isDetailsWidgetVisible());
}

void FaceScanDialog::setupUi()
{
    // --- Main Widget ---

    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout;

    // ---- Introductory labels ----

    QLabel* const personIcon   = new QLabel;
    personIcon->setPixmap(SmallIcon("edit-image-face-show", KIconLoader::SizeLarge));

    QLabel* const introduction = new QLabel;
    introduction->setText(i18nc("@info",
                                "digiKam can search for faces in your photos.<nl/> "
                                "When you have identified your friends on a number of photos,<nl/> "
                                "it can also recognize the people shown on your photos."));
    //introduction->setWordWrap(true);

    // ---- Main option box ----

    d->optionGroupBox               = new QGroupBox;
    QGridLayout* const optionLayout = new QGridLayout;

    d->detectButton                                    = new QRadioButton(i18nc("@option:radio", "Detect faces"));
    d->detectAndRecognizeButton                        = new QRadioButton(i18nc("@option:radio", "Detect and recognize faces (experimental)"));
    ButtonExtendedLabel* const detectAndRecognizeLabel = new ButtonExtendedLabel;
    ButtonExtendedLabel* const detectLabel             = new ButtonExtendedLabel;
    detectAndRecognizeLabel->setText(i18nc("@info",
                                           "Find all faces in your photos<nl/> and try to recognize "
                                           "which person is depicted"));
    //detectAndRecognizeLabel->setWordWrap(true);
    detectLabel->setButton(d->detectButton);
    detectLabel->setText(i18nc("@info",
                                "Find all faces in your photos"));
    detectAndRecognizeLabel->setButton(d->detectAndRecognizeButton);
    ButtonExtendedLabel* const detectAndRecognizeIcon  = new ButtonExtendedLabel;
    ButtonExtendedLabel* const detectIcon              = new ButtonExtendedLabel;
    detectAndRecognizeIcon->setPixmap(SmallIcon("edit-image-face-detect", KIconLoader::SizeLarge));
    detectIcon->setButton(d->detectButton);
    detectAndRecognizeIcon->setButton(d->detectAndRecognizeButton);
    detectAndRecognizeIcon->setAlignment(Qt::AlignCenter);
    d->alreadyScannedBox                               = new QComboBox;
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Skip images already scanned"), FaceScanSettings::Skip);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Scan again and merge results"), FaceScanSettings::Merge);
    d->alreadyScannedBox->addItem(i18nc("@label:listbox", "Clear unconfirmed results and rescan"), FaceScanSettings::Rescan);
    d->alreadyScannedBox->setCurrentIndex(FaceScanSettings::Skip);
    QGridLayout* const detectAndRecognizeLabelLayout   = new QGridLayout;
    detectAndRecognizeLabelLayout->addWidget(detectAndRecognizeLabel, 0, 0, 1, -1);
    detectAndRecognizeLabelLayout->setColumnMinimumWidth(0, 10);

    d->reRecognizeButton                        = new QRadioButton(i18nc("@option:radio", "Recognize faces (experimental)"));
    ButtonExtendedLabel* const reRecognizeLabel = new ButtonExtendedLabel;
    reRecognizeLabel->setText(i18nc("@info",
                                    "Try again to recognize the people depicted<nl/> on marked but yet unconfirmed faces."));
    //reRecognizeLabel->setWordWrap(true);
    reRecognizeLabel->setButton(d->reRecognizeButton);
    ButtonExtendedLabel* const reRecognizeIcon = new ButtonExtendedLabel;
    reRecognizeIcon->setPixmap(SmallIcon("edit-image-face-recognize", KIconLoader::SizeLarge));
    reRecognizeIcon->setButton(d->reRecognizeButton);
    reRecognizeIcon->setAlignment(Qt::AlignCenter);

    optionLayout->addWidget(d->alreadyScannedBox,          0, 0, 1, 2);
    optionLayout->addWidget(d->detectButton,               1, 0, 1, 2);
    optionLayout->addWidget(detectAndRecognizeIcon,        1, 2, 2, 1);
    optionLayout->addWidget(detectLabel,                   2, 1);
    optionLayout->addWidget(d->detectAndRecognizeButton,   3, 0, 1, 2);
    optionLayout->addLayout(detectAndRecognizeLabelLayout, 4, 1);
    optionLayout->addWidget(d->reRecognizeButton,          5, 0, 1, 2);
    optionLayout->addWidget(reRecognizeIcon,               5, 2, 2, 1);
    optionLayout->addWidget(reRecognizeLabel,              6, 1);

    QStyleOptionButton buttonOption;
    buttonOption.initFrom(d->detectAndRecognizeButton);
    int indent = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &buttonOption, d->detectAndRecognizeButton).width();
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

    d->tabWidget = new KTabWidget;

    // ---- Album tab ----

    d->albumSelectors = new AlbumSelectors(i18nc("@label", "Search in:"), d->configName, d->tabWidget);
    d->tabWidget->addTab(d->albumSelectors, i18nc("@title:tab", "Albums"));

    // ---- Parameters tab ----

    QWidget* const parametersTab        = new QWidget(d->tabWidget);
    QGridLayout* const parametersLayout = new QGridLayout(parametersTab);

    QLabel* const detectionLabel        = new QLabel(i18nc("@label", "Parameters for face detection and Recognition"), (parametersTab));

    d->parametersResetButton            = new QToolButton(parametersTab);
    d->parametersResetButton->setAutoRaise(true);
    d->parametersResetButton->setFocusPolicy(Qt::NoFocus);
    d->parametersResetButton->setIcon(SmallIcon("document-revert"));
    d->parametersResetButton->setToolTip(i18nc("@action:button", "Reset to default values"));

    d->accuracyInput                    = new KIntNumInput(parametersTab);
    d->accuracyInput->setRange(0, 100, 10);
    d->accuracyInput->setSliderEnabled();
    d->accuracyInput->setLabel(i18nc("@label Two extremities of a scale", "Fast   -   Accurate"),
                               Qt::AlignTop | Qt::AlignHCenter);
    d->accuracyInput->setToolTip(i18nc("@info:tooltip",
                                       "Adjust speed versus accuracy: The higher the value, the more accurate the results "
                                       "will be, but it will take more time."));

    parametersLayout->addWidget(detectionLabel,           0, 0);
    parametersLayout->addWidget(d->parametersResetButton, 0, 1);
    parametersLayout->addWidget(d->accuracyInput,         1, 0, 1, -1);
    parametersLayout->setColumnStretch(0, 10);
    parametersLayout->setRowStretch(2, 10);

    d->tabWidget->addTab(parametersTab, i18nc("@title:tab", "Parameters"));

    // ---- Advanced tab ----

    QWidget* const advancedTab        = new QWidget(d->tabWidget);
    QGridLayout* const advancedLayout = new QGridLayout(advancedTab);

    QLabel* const cpuExplanation      = new QLabel(advancedTab);
    cpuExplanation->setText(i18nc("@info",
                                  "Face detection is a time-consuming task. "
                                  "You can choose if you wish to employ all processor cores "
                                  "on your system, or work in the background only on one core."));
    cpuExplanation->setWordWrap(true);

    d->useFullCpuButton = new QCheckBox(advancedTab);
    d->useFullCpuButton->setText(i18nc("@option:check", "Work on all processor cores"));

    d->retrainAllButton = new QCheckBox(advancedTab);
    d->retrainAllButton->setText(i18nc("@option:check", "Clear and rebuild all training data"));
    d->retrainAllButton->setToolTip(i18nc("@info:tooltip",
                                          "This will clear all training data for recognition "
                                          "and rebuild it from all available faces. "
                                          "Be careful if any other application helped in building your training database. "));

    d->benchmarkDetectionButton = new QCheckBox(advancedTab);
    d->benchmarkDetectionButton->setText(i18nc("@option:check", "Benchmark face detection"));
    d->benchmarkDetectionButton->setToolTip(i18nc("@info:tooltip",
                                         "This will run face detection and compare the results "
                                         "with faces already marked, which are taken as ground truth. "
                                         "At the end, benchmark results will be presented. "));

    d->benchmarkRecognitionButton = new QCheckBox(advancedTab);
    d->benchmarkRecognitionButton->setText(i18nc("@option:check", "Benchmark face recognition"));
    d->benchmarkRecognitionButton->setToolTip(i18nc("@info:tooltip",
                                         "This will run face recognition on known faces compare the results "
                                         "with the known faces, which are taken as ground truth. "
                                         "For some recognition modes, this procedure does not make sense. "
                                         "At the end, benchmark results will be presented. "));
    QButtonGroup* benchmarkGroup = new QButtonGroup(this);
    benchmarkGroup->setExclusive(true);
    benchmarkGroup->addButton(d->benchmarkDetectionButton);
    benchmarkGroup->addButton(d->benchmarkRecognitionButton);

    advancedLayout->addWidget(cpuExplanation,                 0, 0);
    advancedLayout->addWidget(d->useFullCpuButton,            1, 0);
    advancedLayout->addWidget(new KSeparator(Qt::Horizontal), 2, 0);
    advancedLayout->addWidget(d->retrainAllButton,            3, 0);
    advancedLayout->addWidget(d->benchmarkDetectionButton,    4, 0);
    advancedLayout->addWidget(d->benchmarkRecognitionButton,  5, 0);
    parametersLayout->setRowStretch(5, 10);

    d->tabWidget->addTab(advancedTab, i18nc("@title:tab", "Advanced"));

    // ---

    d->tabWidget->setAutomaticResizeTabs(true);
    setDetailsWidget(d->tabWidget);
}

void FaceScanDialog::setupConnections()
{
    connect(d->detectButton, SIGNAL(toggled(bool)),
            d->alreadyScannedBox, SLOT(setEnabled(bool)));

    connect(d->detectAndRecognizeButton, SIGNAL(toggled(bool)),
            d->alreadyScannedBox, SLOT(setEnabled(bool)));

    connect(d->parametersResetButton, SIGNAL(clicked()),
            this, SLOT(setDetectionDefaultParameters()));

    connect(d->retrainAllButton, SIGNAL(toggled(bool)),
            this, SLOT(retrainAllButtonToggled(bool)));

    connect(d->benchmarkDetectionButton, SIGNAL(toggled(bool)),
            this, SLOT(benchmarkButtonToggled(bool)));

    connect(d->benchmarkRecognitionButton, SIGNAL(toggled(bool)),
            this, SLOT(benchmarkButtonToggled(bool)));
}

void FaceScanDialog::retrainAllButtonToggled(bool on)
{
    d->optionGroupBox->setEnabled(!on);
    d->albumSelectors->setEnabled(!on);
    d->benchmarkDetectionButton->setEnabled(!on);
}

void FaceScanDialog::benchmarkButtonToggled(bool)
{
    bool anyOn = d->benchmarkDetectionButton->isChecked() || d->benchmarkRecognitionButton->isChecked();
    d->optionGroupBox->setEnabled(!anyOn);
    d->retrainAllButton->setEnabled(!anyOn);
}

FaceScanSettings FaceScanDialog::settings() const
{
    FaceScanSettings settings;

    if (d->retrainAllButton->isChecked())
    {
        settings.task = FaceScanSettings::RetrainAll;
    }
    else if (d->benchmarkDetectionButton->isChecked())
    {
        settings.task = FaceScanSettings::BenchmarkDetection;
    }
    else if (d->benchmarkRecognitionButton->isChecked())
    {
        settings.task = FaceScanSettings::BenchmarkRecognition;
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

    settings.albums << d->albumSelectors->selectedAlbums();

    settings.useFullCpu             = d->useFullCpuButton->isChecked();

    return settings;
}

} // namespace Digikam
