/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Dialog to choose options for face scanning
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QCheckBox>
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

#include <KDebug>
#include <KDialog>
#include <KIconLoader>
#include <KLocale>
#include <KIntNumInput>
#include <KStandardGuiItem>

// Local includes

#include "albummodel.h"
#include "albumselectcombobox.h"
#include "albumsettings.h"
#include "albumtreeview.h"

namespace Digikam
{

class ButtonExtendedLabel : public QLabel
{
public:

    ButtonExtendedLabel(QWidget *parent = 0)
        : QLabel(parent), m_button(0) {}
    ButtonExtendedLabel(const QString& text, QWidget *parent = 0)
        : QLabel(text, parent), m_button(0) {}

    void setButton(QAbstractButton *button)
    {
        setBuddy(button);
        m_button = button;
    }

protected:

    // quick & dirty workaround the fact that QRadioButton doesnt provide a decent label
    void mouseReleaseEvent(QMouseEvent *)
    {
        if (m_button)
            m_button->toggle();
    }

    QAbstractButton *m_button;
};

class FaceScanDialog::FaceScanDialogPriv
{
public:

    FaceScanDialogPriv()
        : configName("Face Detection Dialog"),
          configDetectAndRecognize("Detect and Recognize Faces"),
          configRecognizedMarkedFaces("Recognize Marked Faces"),
          configSkipAlreadyScanned("Skip Already Scanned"),
          configUseFullCpu("Use Full CPU"),
          configSettingsVisible("Settings Widget Visible")

    {
    }

    QRadioButton    *detectAndRecognizeButton;
    QCheckBox       *skipAlreadyScannedBox;
    QRadioButton    *reRecognizeButton;

    QTabWidget      *tabWidget;

    AlbumTreeViewSelectComboBox* albumSelectCB;
    TagTreeViewSelectComboBox* tagSelectCB;

    QToolButton  *parametersResetButton;
    KIntNumInput *accuracyInput;
    KIntNumInput *specificityInput;

    QCheckBox    *useFullCpuButton;

    const QString configName;
    const QString configDetectAndRecognize;
    const QString configRecognizedMarkedFaces;
    const QString configSkipAlreadyScanned;
    const QString configUseFullCpu;
    const QString configSettingsVisible;
};

FaceScanDialog::FaceScanDialog(QWidget *parent)
    : KDialog(parent), StateSavingObject(this), d(new FaceScanDialogPriv)
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
    d->albumSelectCB->view()->setObjectName(d->configName);
    d->albumSelectCB->view()->setEntryPrefix("AlbumComboBox-");
    d->albumSelectCB->view()->setRestoreCheckState(true);
    d->tagSelectCB->view()->setObjectName(d->configName);
    d->tagSelectCB->view()->setEntryPrefix("TagComboBox-");
    d->tagSelectCB->view()->setRestoreCheckState(true);
    loadState();
}

void FaceScanDialog::accept()
{
    KDialog::accept();
    saveState();
}

void FaceScanDialog::setDetectionDefaultParameters()
{
    d->accuracyInput->setValue(80);
    d->specificityInput->setValue(80);
}

void FaceScanDialog::doLoadState()
{
    kDebug() << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();

    if (group.readEntry(entryName(d->configRecognizedMarkedFaces), false))
        d->reRecognizeButton->setChecked(true);
    else
        d->detectAndRecognizeButton->setChecked(true);

    d->skipAlreadyScannedBox->setChecked(group.readEntry(entryName(d->configSkipAlreadyScanned), true));

    d->accuracyInput->setValue(AlbumSettings::instance()->getFaceDetectionAccuracy() * 100);
    d->specificityInput->setValue(AlbumSettings::instance()->getFaceDetectionSpecificity() * 100);

    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();

    d->useFullCpuButton->setChecked(group.readEntry(entryName(d->configUseFullCpu), true));

    setDetailsWidgetVisible(group.readEntry(entryName(d->configSettingsVisible), false));
}

void FaceScanDialog::doSaveState()
{
    kDebug() << getConfigGroup().name();
    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configDetectAndRecognize), d->detectAndRecognizeButton->isChecked());
    group.writeEntry(entryName(d->configRecognizedMarkedFaces), d->reRecognizeButton->isChecked());
    group.writeEntry(entryName(d->configSkipAlreadyScanned), d->skipAlreadyScannedBox->isChecked());

    AlbumSettings::instance()->setFaceDetectionAccuracy(double(d->accuracyInput->value()) / 100);
    AlbumSettings::instance()->setFaceDetectionSpecificity(double(d->specificityInput->value()) / 100);

    d->albumSelectCB->view()->saveState();
    d->tagSelectCB->view()->saveState();

    group.writeEntry(entryName(d->configUseFullCpu), d->useFullCpuButton->isChecked());

    group.writeEntry(entryName(d->configSettingsVisible), isDetailsWidgetVisible());
}

void FaceScanDialog::setupUi()
{
    // --- Main Widget ---

    QWidget *mainWidget = new QWidget;
    QGridLayout *mainLayout = new QGridLayout;

    // ---- Introductory labels ----

    QLabel *personIcon = new QLabel;
    personIcon->setPixmap(SmallIcon("user-identity", KIconLoader::SizeLarge));

    QLabel *introduction = new QLabel;
    introduction->setText(i18nc("@info",
                                "DigiKam can search for faces in your photos.<nl/> "
                                "When you have identified your friends on a number of photos,<nl/> "
                                "it can also recognize the people shown on your photos."));
    //introduction->setWordWrap(true);

    // ---- Main option box ----

    QGroupBox *optionWidget = new QGroupBox;
    QGridLayout *optionLayout = new QGridLayout;

    d->detectAndRecognizeButton = new QRadioButton(i18nc("@option:radio", "Detect and recognize faces"));
    ButtonExtendedLabel *detectAndRecognizeLabel = new ButtonExtendedLabel;
    detectAndRecognizeLabel->setText(i18nc("@info",
                                           "Find all faces in your photos<nl/> and try to recognize "
                                           "which person is depicted"));
    //detectAndRecognizeLabel->setWordWrap(true);
    detectAndRecognizeLabel->setButton(d->detectAndRecognizeButton);
    ButtonExtendedLabel *detectAndRecognizeIcon = new ButtonExtendedLabel;
    detectAndRecognizeIcon->setPixmap(SmallIcon("user-group-new", KIconLoader::SizeLarge));
    detectAndRecognizeIcon->setButton(d->detectAndRecognizeButton);
    detectAndRecognizeIcon->setAlignment(Qt::AlignCenter);
    d->skipAlreadyScannedBox = new QCheckBox;
    d->skipAlreadyScannedBox->setText(i18nc("@option:check", "Skip images already scanned"));
    QGridLayout *detectAndRecognizeLabelLayout = new QGridLayout;
    detectAndRecognizeLabelLayout->addWidget(detectAndRecognizeLabel, 0, 0, 1, -1);
    detectAndRecognizeLabelLayout->setColumnMinimumWidth(0, 10);
    detectAndRecognizeLabelLayout->addWidget(d->skipAlreadyScannedBox, 1, 1);

    d->reRecognizeButton = new QRadioButton(i18nc("@option:radio", "Recognize faces"));
    ButtonExtendedLabel *reRecognizeLabel = new ButtonExtendedLabel;
    reRecognizeLabel->setText(i18nc("@info",
                                    "Try again to recognize the people depicted<nl/> on marked but yet unconfirmed faces."));
    //reRecognizeLabel->setWordWrap(true);
    reRecognizeLabel->setButton(d->reRecognizeButton);
    ButtonExtendedLabel *reRecognizeIcon = new ButtonExtendedLabel;
    reRecognizeIcon->setPixmap(SmallIcon("view-media-artist", KIconLoader::SizeMedium));
    reRecognizeIcon->setButton(d->reRecognizeButton);
    reRecognizeIcon->setAlignment(Qt::AlignCenter);

    optionLayout->addWidget(d->detectAndRecognizeButton, 0, 0, 1, 2);
    optionLayout->addWidget(detectAndRecognizeIcon, 0, 2, 2, 1);
    optionLayout->addLayout(detectAndRecognizeLabelLayout, 1, 1);
    optionLayout->addWidget(d->reRecognizeButton, 2, 0, 1, 2);
    optionLayout->addWidget(reRecognizeIcon, 2, 2, 2, 1);
    optionLayout->addWidget(reRecognizeLabel, 3, 1);

    QStyleOptionButton buttonOption;
    buttonOption.initFrom(d->detectAndRecognizeButton);
    int indent = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &buttonOption, d->detectAndRecognizeButton).width();
    optionLayout->setColumnMinimumWidth(0, indent);

    optionWidget->setLayout(optionLayout);

    // ---

    mainLayout->addWidget(personIcon, 0, 0);
    mainLayout->addWidget(introduction, 0, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->addWidget(optionWidget, 1, 0, 1, -1);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);


    // --- Tab Widget ---

    d->tabWidget = new QTabWidget;

    // ---- Album tab ----

    QWidget *selectAlbumsTab = new QWidget;
    QVBoxLayout *selectAlbumsLayout = new QVBoxLayout;

    QLabel *includeAlbumsLabel = new QLabel(i18nc("@label", "Search in:"));

    d->albumSelectCB = new AlbumTreeViewSelectComboBox();
    //d->albumSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be included in the face scan."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));

    d->tagSelectCB = new TagTreeViewSelectComboBox();
    //d->tagSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be included in the face scan."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any tags"));

    selectAlbumsLayout->addWidget(includeAlbumsLabel);
    selectAlbumsLayout->addWidget(d->albumSelectCB);
    selectAlbumsLayout->addWidget(d->tagSelectCB);
    selectAlbumsLayout->addStretch(1);

    selectAlbumsTab->setLayout(selectAlbumsLayout);
    d->tabWidget->addTab(selectAlbumsTab, i18nc("@title:tab", "Albums"));

    // ---- Parameters tab ----

    QWidget *parametersTab = new QWidget;
    QGridLayout *parametersLayout = new QGridLayout;

    QLabel *detectionLabel = new QLabel(i18nc("@label", "Parameters for face detection"));

    d->parametersResetButton = new QToolButton;
    d->parametersResetButton->setAutoRaise(true);
    d->parametersResetButton->setFocusPolicy(Qt::NoFocus);
    d->parametersResetButton->setIcon(SmallIcon("document-revert"));
    d->parametersResetButton->setToolTip(i18nc("@action:button", "Reset to default values"));

    d->accuracyInput = new KIntNumInput;
    d->accuracyInput->setRange(0, 100, 10);
    d->accuracyInput->setSliderEnabled();
    d->accuracyInput->setLabel(i18nc("@label Two extremities of a scale", "Fast   -   Accurate"),
                               Qt::AlignTop | Qt::AlignHCenter);
    d->accuracyInput->setToolTip(i18nc("@info:tooltip",
                                       "Adjust speed versus accuracy: The higher the value, the more accurate the results "
                                       "will be, but it will take more time."));

    d->specificityInput = new KIntNumInput;
    d->specificityInput->setRange(0, 100, 10);
    d->specificityInput->setSliderEnabled();
    d->specificityInput->setLabel(i18nc("@label Two extremities of a scale", "Sensitive   -   Specific"),
                                  Qt::AlignTop | Qt::AlignHCenter);
    d->specificityInput->setToolTip(i18nc("@info:tooltip",
                                          "Adjust sensitivity versus specificity: If the value is high, most faces returned will "
                                          "really be a face - few false positive. If the value is low, more faces will be found, "
                                          "but some returned faces will not really be faces."));

    parametersLayout->addWidget(detectionLabel, 0, 0);
    parametersLayout->addWidget(d->parametersResetButton, 0, 1);
    parametersLayout->addWidget(d->accuracyInput, 1, 0, 1, -1);
    parametersLayout->addWidget(d->specificityInput, 2, 0, 1, -1);
    parametersLayout->setColumnStretch(0, 1);

    parametersTab->setLayout(parametersLayout);
    d->tabWidget->addTab(parametersTab, i18nc("@title:tab", "Parameters"));

    // ---- Performance tab ----

    QWidget *performanceTab = new QWidget;
    QVBoxLayout *performanceLayout = new QVBoxLayout;

    QLabel *cpuExplanation = new QLabel;
    cpuExplanation->setText(i18nc("@info",
                                  "Face detection is a time-consuming task. "
                                  "You can choose if you wish to employ all processor cores on your system, "
                                  "or work in the background only on one core."));
    cpuExplanation->setWordWrap(true);

    d->useFullCpuButton = new QCheckBox;
    d->useFullCpuButton->setText(i18nc("@option:check", "Work on all processor cores"));

    performanceLayout->addWidget(cpuExplanation);
    performanceLayout->addWidget(d->useFullCpuButton);
    performanceLayout->addStretch(1);

    performanceTab->setLayout(performanceLayout);
    d->tabWidget->addTab(performanceTab, i18nc("@title:tab", "Resources"));

    // ---

    setDetailsWidget(d->tabWidget);
}

void FaceScanDialog::setupConnections()
{
    connect(d->detectAndRecognizeButton, SIGNAL(toggled(bool)),
            d->skipAlreadyScannedBox, SLOT(setEnabled(bool)));

    connect(d->parametersResetButton, SIGNAL(clicked()),
            this, SLOT(setDetectionDefaultParameters()));
}

FaceScanSettings FaceScanDialog::settings() const
{
    FaceScanSettings settings;

    if (d->detectAndRecognizeButton->isChecked())
        settings.task = FaceScanSettings::DetectAndRecognize;
    else
        settings.task = FaceScanSettings::RecognizeMarkedFaces;

    settings.skipAlreadyScanned = d->skipAlreadyScannedBox->isChecked();

    settings.accuracy    = double(d->accuracyInput->value()) / 100;
    settings.specificity = double(d->specificityInput->value()) / 100;

    settings.albums << d->albumSelectCB->model()->checkedAlbums();
    settings.albums << d->tagSelectCB->model()->checkedAlbums();

    settings.useFullCpu = d->useFullCpuButton->isChecked();

    return settings;
}

}

