/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : setup tab for image versioning
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupversioning.moc"

// Qt includes

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHelpEvent>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWhatsThis>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kapplication.h>

// Local includes

#include "applicationsettings.h"
#include "versionmanager.h"

namespace Digikam
{

class SetupVersioning::Private
{
public:

    Private()
        : tab(0),
          nonDestructivePanel(0),
          workspaceGB(0),
          closingGB(0),
          snapshotGB(0),
          viewGB(0),
          enableNonDestructive(0),
          snapshotAfterRaw(0),
          snapshotSession(0),
          snapshotComplex(0),
          viewShowOriginal(0),
          viewShowSnapshots(0),
/*
          jpgFormat(0),
          pngFormat(0),
          tiffFormat(0),
          pgfFormat(0),
*/
          formatBox(0),
          askToSave(0),
          autoSave(0),
          infoNonDestructive(0),
          infoFormat(0),
          infoSnapshot(0),
          infoView(0)
    {
    }

    KTabWidget*    tab;

    QWidget*       nonDestructivePanel;

    QGroupBox*     workspaceGB;
    QGroupBox*     closingGB;
    QGroupBox*     snapshotGB;
    QGroupBox*     viewGB;

    QCheckBox*     enableNonDestructive;

    QCheckBox*     snapshotAfterRaw;
    QCheckBox*     snapshotSession;
    QCheckBox*     snapshotComplex;

    QCheckBox*     viewShowOriginal;
    QCheckBox*     viewShowSnapshots;

    QComboBox*     formatBox;
/*
    QRadioButton*  jpgFormat;
    QRadioButton*  pngFormat;
    QRadioButton*  tiffFormat;
    QRadioButton*  pgfFormat;
*/

    QRadioButton*  askToSave;
    QRadioButton*  autoSave;

    QPushButton*   infoNonDestructive;
    QPushButton*   infoFormat;
    QPushButton*   infoSnapshot;
    QPushButton*   infoView;
};

SetupVersioning::SetupVersioning(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    d->nonDestructivePanel            = new QWidget;
    QVBoxLayout* nonDestructiveLayout = new QVBoxLayout;

    // ---

    QGridLayout* gridHeader = new QGridLayout;

    d->enableNonDestructive = new QCheckBox;
    d->enableNonDestructive->setText(i18n("Enable Non-Destructive Editing and Versioning"));
    d->enableNonDestructive->setToolTip(i18nc("@info:tooltip",
                                              "Enable support for non-destructive editing and image versioning"));
    d->enableNonDestructive->setWhatsThis(i18nc("@info:whatsthis",
                                                "<para><interface>Non-Destructive Editing and Versioning</interface> "
                                                "allows different versions of an image to be created, "
                                                "whilst always preserving the original image.</para> "
                                                "<para> All steps of the editing history are recorded and can be accessed later.</para>"));

    QLabel* iconLabel       = new QLabel;
    iconLabel->setPixmap(SmallIcon("view-catalog", KIconLoader::SizeMedium));//"folder-image"));

    d->infoNonDestructive   = new QPushButton;
    d->infoNonDestructive->setIcon(SmallIcon("dialog-information"));
    d->infoNonDestructive->setToolTip(i18nc("@info:tooltip", "Get information on non-destructive editing and file versioning"));

    gridHeader->addWidget(iconLabel,               0, 0);
    gridHeader->addWidget(d->enableNonDestructive, 0, 1);
    gridHeader->addWidget(d->infoNonDestructive,   0, 3);
    gridHeader->setColumnStretch(2, 1);

    // --------------------------------------------------------

    d->workspaceGB         = new QGroupBox(i18nc("@title:group", "Workspace File Format"));
    QGridLayout* wsLayout  = new QGridLayout;

    QLabel* workIcon       = new QLabel;
    workIcon->setPixmap(SmallIcon("document-save-as", KIconLoader::SizeMedium));
    QLabel* formatLabel    = new QLabel(i18nc("@label", "Save files as"));

    // keep in sync with VersionManager::workspaceFileFormats()
    d->formatBox           = new QComboBox;
    d->formatBox->addItem(i18nc("@label:listbox", "JPEG"), "JPG");
    d->formatBox->addItem(i18nc("@label:listbox", "TIFF"), "TIFF");
    d->formatBox->addItem(i18nc("@label:listbox", "PNG"), "PNG");
    d->formatBox->addItem(i18nc("@label:listbox", "PGF"), "PGF");
    d->formatBox->addItem(i18nc("@label:listbox", "JPEG 2000"), "JP2");
    d->formatBox->insertSeparator(1);
    d->formatBox->insertSeparator(4);

    d->formatBox->setWhatsThis(i18nc("@info:whatsthis",
                                     "<title>Default File Format for Saving</title>"
                                     "<para>Select the file format in which edited images are saved automatically. "
                                     "Format-specific options, like compression settings, "
                                     "can be configured on the <interface>Format Options</interface> tab.</para>"
                                     "<para><list>"
                                     // Lossy: JPEG
                                     "<item>"
                                     "<filename>JPEG</filename>: "
                                     "JPEG is the most commonly used file format, but it employs lossy compression, "
                                     "which means that with each saving operation some image information will be irreversibly lost. "
                                     "JPEG offers a good compression rate, resulting in smaller files. "
                                     "</item>"
                                     // Traditional lossless: PNG, TIFF
                                     "<item>"
                                     "<filename>PNG</filename>: "
                                     "A widely used format employing lossless compression. "
                                     "The files, though, will be larger because PNG does not achieve very good compression rates."
                                     "</item>"
                                     "<item>"
                                     "<filename>TIFF</filename>: "
                                     "A commonly used format, usually uncompressed or with modest lossless compression. "
                                     "Resulting files will be large, but without quality loss due to compression. "
                                     "</item>"
                                     // Modern lossless: PGF, JPEG 2000
                                     "<item>"
                                     "<filename>PGF</filename>: "
                                     "This is a technically superior file format offering good compression rates "
                                     "with either lossy or lossless compression. "
                                     "But it is not yet widely used and supported, so your friends may not directly be able to open these files, "
                                     "and you may not be able to directly publish them on the web. "
                                     "</item>"
                                     "<item>"
                                     "<filename>JPEG 2000</filename>: "
                                     "JPEG 2000 is similar to PGF. Loading or saving is slower, the compression rate is better, "
                                     "and the format more widely supported, though still not comparable "
                                     "to the tradition formats JPEG, PNG or TIFF. "
                                     "</item>"
                                     "</list></para>"));
/*
    d->jpgFormat  = new QRadioButton(i18nc("@option:radio", "JPEG"));
    d->pngFormat  = new QRadioButton(i18nc("@option:radio", "PNG"));
    d->tiffFormat = new QRadioButton(i18nc("@option:radio", "TIFF"));
    d->pgfFormat  = new QRadioButton(i18nc("@option:radio", "PGF"));
    wsLayout->addWidget(d->jpgFormat,    1, 0, 1, 2);
    wsLayout->addWidget(d->pngFormat,    2, 0, 1, 2);
    wsLayout->addWidget(d->tiffFormat,   3, 0, 1, 2);
    wsLayout->addWidget(d->pgfFormat,    4, 0, 1, 2);
    wsLayout->addWidget(d->formatInfoLabel, 5, 0, 1, 2, Qt::AlignRight);
*/

    d->infoFormat = new QPushButton;
    d->infoFormat->setIcon(SmallIcon("dialog-information"));
    d->infoFormat->setToolTip(i18nc("@info:tooltip", "Get information on available image file formats"));

    wsLayout->addWidget(workIcon,           0, 0);
    wsLayout->addWidget(formatLabel,        0, 1);
    wsLayout->addWidget(d->formatBox,       0, 2);
    wsLayout->addWidget(d->infoFormat,      0, 4);
    wsLayout->setColumnStretch(1, 1);
    wsLayout->setColumnStretch(2, 2);
    wsLayout->setColumnStretch(3, 3);

    d->workspaceGB->setLayout(wsLayout);

    // ---

    d->closingGB               = new QGroupBox;//(i18nc("@title:group", "Automatic Saving"));
    QGridLayout* closingLayout = new QGridLayout;

    QLabel* closingExplanation = new QLabel(i18nc("@label", "When closing the editor"));
    QLabel* closingIcon        = new QLabel;
    closingIcon->setPixmap(SmallIcon("dialog-ok-apply", KIconLoader::SizeMedium));
    d->askToSave               = new QRadioButton(i18nc("@option:radio", "Always ask to save changes"));
    d->autoSave                = new QRadioButton(i18nc("@option:radio", "Save changes automatically"));

    closingLayout->addWidget(closingIcon,        0, 0);
    closingLayout->addWidget(closingExplanation, 0, 1);
    closingLayout->addWidget(d->askToSave,       1, 0, 1, 3);
    closingLayout->addWidget(d->autoSave,        2, 0, 1, 3);
    closingLayout->setColumnStretch(3, 1);

    d->closingGB->setLayout(closingLayout);

    // ---

    // --------------------------------------------------------

/*
    QGridLayout* snapshotHeader = new QGridLayout;

    QLabel *snapshotExplanation = new QLabel;
    snapshotExplanation->setText(i18nc("@label",
                                       "For an edited image, there is at least one file representing the current version."
                                       "DigiKam can take and keep additional, intermediate snapshots during editing."));
    snapshotExplanation->setWordWrap(true);


    snapshotHeader->addWidget(snapshotIconLabel,       0, 0);
    snapshotHeader->addWidget(snapshotExplanation,     0, 1);
*/

    d->snapshotGB = new QGroupBox;//(i18nc("@title:group", "Intermediate Version Snapshots"));
    QGridLayout* snapshotLayout = new QGridLayout;

    QString snapshotWhatsThis = i18nc("@info:whatsthis",
                                      "<para>First and foremost, the <emphasis>original image will never be overwritten.</emphasis> "
                                      "Instead, when an image is edited, a new file is created: "
                                      "The <interface>current version</interface>.</para> "
                                      "<para>You can also create multiple <interface>versions</interface> "
                                      "deriving from the same <interface>original image</interface>.</para> "
                                      "<para>In addition to these files representing a current version, "
                                      "digiKam can take and keep additional, <interface>intermediate snapshots</interface> "
                                      "during the editing process. "
                                      "This can be useful if you want to preserve the intermediate steps for later "
                                      "access, for example if some editing steps cannot be automatically reproduced.</para> ");
    d->snapshotGB->setWhatsThis(snapshotWhatsThis);

    QLabel* snapshotIconLabel = new QLabel;
    snapshotIconLabel->setPixmap(SmallIcon("insert-image", KIconLoader::SizeMedium));

    QLabel* snapshotLabel     = new QLabel(i18nc("@label", "Keep a snapshot of an edited image"));

    d->infoSnapshot           = new QPushButton;
    d->infoSnapshot->setIcon(SmallIcon("dialog-information"));
    d->infoSnapshot->setToolTip(i18nc("@info:tooltip", "Get an explanation for these options"));

    d->snapshotAfterRaw       = new QCheckBox(i18nc("@option:check", "After converting from a RAW image"));
    d->snapshotSession        = new QCheckBox(i18nc("@option:check", "After each editing session"));
    d->snapshotComplex        = new QCheckBox(i18nc("@option:check", "After each step that is not completely reproducible"));

    snapshotLayout->addWidget(snapshotIconLabel,   0, 0);
    snapshotLayout->addWidget(snapshotLabel,       0, 1);
    snapshotLayout->addWidget(d->infoSnapshot,     0, 3);
    snapshotLayout->addWidget(d->snapshotAfterRaw, 1, 0, 1, 4);
    snapshotLayout->addWidget(d->snapshotSession,  2, 0, 1, 4);
    snapshotLayout->addWidget(d->snapshotComplex,  3, 0, 1, 4);
    snapshotLayout->setColumnStretch(2, 1);
    d->snapshotGB->setLayout(snapshotLayout);

/*
    / ---

    snapshotLayout->addLayout(snapshotHeader);
    snapshotLayout->addWidget(d->snapshotGB);
    snapshotLayout->addStretch();

    d->snapshotPanel->setLayout(snapshotLayout);
*/

    // --------------------------------------------------------

/*
    d->viewPanel = new QWidget;
    QVBoxLayout* viewLayout = new QVBoxLayout;

    // ---

    QGridLayout* viewHeaderLayout = new QGridLayout;

    QLabel* viewExplanation = new QLabel;
    viewExplanation->setText(i18nc("@label",
                                   "If an image has been edited, only the current versions will be shown. "
                                   "From the right sidebar, you can access all hidden files. "
                                   "Here, you can choose to show certain files permanently."));
    viewExplanation->setWordWrap(true);

    viewHeaderLayout->addWidget(viewIconLabel,       0, 0);
    viewHeaderLayout->addWidget(viewExplanation,     0, 1);

    // ---
*/

    d->viewGB = new QGroupBox;
    QGridLayout* viewGBLayout = new QGridLayout;

    QString viewWhatsThis = i18nc("@info:whatsthis",
                                  "<para>If an image has been edited, only the <interface>current versions</interface> "
                                  "will be shown in the main thumbnail view. "
                                  "From the <interface>right sidebar</interface>, you always have access to all hidden files.</para> "
                                  "<para>With the options here, you can choose to show certain files permanently.</para>");
    d->viewGB->setWhatsThis(viewWhatsThis);

    QLabel* viewLabel     =  new QLabel(i18nc("@label", "In main view"));

    QLabel* viewIconLabel = new QLabel;
    viewIconLabel->setPixmap(SmallIcon("view-list-icons", KIconLoader::SizeMedium));

    d->infoView           = new QPushButton;
    d->infoView->setIcon(SmallIcon("dialog-information"));
    d->infoView->setToolTip(i18nc("@info:tooltip", "Get an explanation for these options"));

    d->viewShowOriginal   = new QCheckBox(i18nc("@option:check", "Always show original images"));
    d->viewShowSnapshots  = new QCheckBox(i18nc("@option:check", "Always show intermediate snapshots"));

    viewGBLayout->addWidget(viewIconLabel,        0, 0);
    viewGBLayout->addWidget(viewLabel,            0, 1);
    viewGBLayout->addWidget(d->infoView,          0, 3);
    viewGBLayout->addWidget(d->viewShowOriginal,  1, 0, 1, 4);
    viewGBLayout->addWidget(d->viewShowSnapshots, 2, 0, 1, 4);
    viewGBLayout->setColumnStretch(2, 1);
    d->viewGB->setLayout(viewGBLayout);

/*
    / ---

    viewLayout->addLayout(viewHeaderLayout);
    viewLayout->addWidget(d->viewGB);
    viewLayout->addStretch();

    d->viewPanel->setLayout(viewLayout);
*/

    // --------------------------------------------------------

    connect(d->enableNonDestructive, SIGNAL(toggled(bool)),
            this, SLOT(enableToggled(bool)));

    connect(d->infoNonDestructive, SIGNAL(clicked()),
            this, SLOT(showNonDestructiveInformation()));

    connect(d->infoFormat, SIGNAL(clicked()),
            this, SLOT(showFormatInformation()));

    connect(d->infoSnapshot, SIGNAL(clicked()),
            this, SLOT(showSnapshotInformation()));

    connect(d->infoView, SIGNAL(clicked()),
            this, SLOT(showViewInformation()));

    // --------------------------------------------------------

    nonDestructiveLayout->addLayout(gridHeader);
    nonDestructiveLayout->addWidget(d->workspaceGB);
    nonDestructiveLayout->addWidget(d->closingGB);
    nonDestructiveLayout->addWidget(d->snapshotGB);
    nonDestructiveLayout->addWidget(d->viewGB);
    nonDestructiveLayout->addStretch();

    d->nonDestructivePanel->setLayout(nonDestructiveLayout);
    setWidget(d->nonDestructivePanel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    readSettings();

    enableToggled(d->enableNonDestructive->isChecked());

    // --------------------------------------------------------
}

SetupVersioning::~SetupVersioning()
{
    delete d;
}

void SetupVersioning::applySettings()
{
    VersionManagerSettings settings;
    settings.enabled = d->enableNonDestructive->isChecked();

    if (d->snapshotSession->isChecked())
    {
        settings.saveIntermediateVersions |= VersionManagerSettings::AfterEachSession;
    }

    if (d->snapshotAfterRaw->isChecked())
    {
        settings.saveIntermediateVersions |= VersionManagerSettings::AfterRawConversion;
    }

    if (d->snapshotComplex->isChecked())
    {
        settings.saveIntermediateVersions |= VersionManagerSettings::WhenNotReproducible;
    }

    if (d->viewShowOriginal->isChecked())
    {
        settings.showInViewFlags |= VersionManagerSettings::ShowOriginal;
    }

    if (d->viewShowSnapshots->isChecked())
    {
        settings.showInViewFlags |= VersionManagerSettings::ShowIntermediates;
    }

    if (d->autoSave->isChecked())
    {
        settings.editorClosingMode = VersionManagerSettings::AutoSave;
    }
    else //if (d->askToSave->isChecked())
    {
        settings.editorClosingMode = VersionManagerSettings::AlwaysAsk;
    }

/*
    if (d->jpgFormat->isChecked())
        settings.format = "JPG";
    else if (d->pngFormat->isChecked())
        settings.format = "PNG";
    else if (d->tiffFormat->isChecked())
        settings.format = "TIFF";
    else if (d->pgfFormat->isChecked())
        settings.format = "PGF";
*/
    settings.format = d->formatBox->itemData(d->formatBox->currentIndex()).toString();

    ApplicationSettings::instance()->setVersionManagerSettings(settings);
    ApplicationSettings::instance()->saveSettings();
}

void SetupVersioning::readSettings()
{
    VersionManagerSettings settings = ApplicationSettings::instance()->getVersionManagerSettings();

    d->enableNonDestructive->setChecked(settings.enabled);
    d->snapshotSession->setChecked(settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession);
    d->snapshotAfterRaw->setChecked(settings.saveIntermediateVersions & VersionManagerSettings::AfterRawConversion);
    d->snapshotComplex->setChecked(settings.saveIntermediateVersions & VersionManagerSettings::WhenNotReproducible);
    d->viewShowOriginal->setChecked(settings.showInViewFlags & VersionManagerSettings::ShowOriginal);
    d->viewShowSnapshots->setChecked(settings.showInViewFlags & VersionManagerSettings::ShowIntermediates);
    d->askToSave->setChecked(settings.editorClosingMode == VersionManagerSettings::AlwaysAsk);
    d->autoSave->setChecked(settings.editorClosingMode == VersionManagerSettings::AutoSave);

/*
    if (settings.format == "JPG")
        d->jpgFormat->setChecked(true);
    else if (settings.format == "PNG")
        d->pngFormat->setChecked(true);
    else if (settings.format == "TIFF")
        d->tiffFormat->setChecked(true);
    else if (settings.format == "PGF")
        d->pgfFormat->setChecked(true);
*/
    d->formatBox->setCurrentIndex(d->formatBox->findData(settings.format));
}

void SetupVersioning::showNonDestructiveInformation()
{
    kapp->postEvent(d->enableNonDestructive, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0),
                                                            d->enableNonDestructive->mapToGlobal(QPoint(0, 0))));
}

void SetupVersioning::showFormatInformation()
{
    kapp->postEvent(d->formatBox, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), d->formatBox->mapToGlobal(QPoint(0, 0))));
}

void SetupVersioning::showSnapshotInformation()
{
    QPoint p(0, 0);
    kapp->postEvent(d->snapshotGB, new QHelpEvent(QEvent::WhatsThis, p, d->snapshotGB->mapToGlobal(p)));
}

void SetupVersioning::showViewInformation()
{
    QPoint p(0, 0);
    kapp->postEvent(d->viewGB, new QHelpEvent(QEvent::WhatsThis, p, d->viewGB->mapToGlobal(p)));
}

void SetupVersioning::enableToggled(bool on)
{
    d->workspaceGB->setEnabled(on);
    d->closingGB->setEnabled(on);
    d->snapshotGB->setEnabled(on);
    d->viewGB->setEnabled(on);
}

} // namespace Digikam
