/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : setup tab for image versioning
 *
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "setupversioning.h"

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
#include <QApplication>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

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
          formatBox(0),
          askToSave(0),
          autoSave(0),
          infoNonDestructive(0),
          infoFormat(0),
          infoSnapshot(0),
          infoView(0)
    {
    }

    QTabWidget*    tab;

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

    QRadioButton*  askToSave;
    QRadioButton*  autoSave;

    QPushButton*   infoNonDestructive;
    QPushButton*   infoFormat;
    QPushButton*   infoSnapshot;
    QPushButton*   infoView;
};

SetupVersioning::SetupVersioning(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->nonDestructivePanel                  = new QWidget;
    QVBoxLayout* const nonDestructiveLayout = new QVBoxLayout;

    // ---

    QGridLayout* const gridHeader = new QGridLayout;

    d->enableNonDestructive       = new QCheckBox;
    d->enableNonDestructive->setText(i18n("Enable Non-Destructive Editing and Versioning"));
    d->enableNonDestructive->setToolTip(i18nc("@info:tooltip",
                                              "Enable support for non-destructive editing and image versioning"));
    d->enableNonDestructive->setWhatsThis(xi18nc("@info:whatsthis",
                                                "<para><interface>Non-Destructive Editing and Versioning</interface> "
                                                "allows different versions of an image to be created, "
                                                "whilst always preserving the original image.</para> "
                                                "<para> All steps of the editing history are recorded and can be accessed later.</para>"));

    QLabel* const iconLabel = new QLabel;
    iconLabel->setPixmap(QIcon::fromTheme(QLatin1String("view-catalog")).pixmap(32));

    d->infoNonDestructive   = new QPushButton;
    d->infoNonDestructive->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoNonDestructive->setToolTip(i18nc("@info:tooltip", "Get information on non-destructive editing and file versioning"));

    gridHeader->addWidget(iconLabel,               0, 0);
    gridHeader->addWidget(d->enableNonDestructive, 0, 1);
    gridHeader->addWidget(d->infoNonDestructive,   0, 3);
    gridHeader->setColumnStretch(2, 1);

    // --------------------------------------------------------

    d->workspaceGB              = new QGroupBox(i18nc("@title:group", "Workspace File Format"));
    QGridLayout* const wsLayout = new QGridLayout;

    QLabel* const workIcon      = new QLabel;
    workIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-save-as")).pixmap(32));
    QLabel* const formatLabel   = new QLabel(i18nc("@label", "Save files as"));

    // keep in sync with VersionManager::workspaceFileFormats()
    d->formatBox           = new QComboBox;
    d->formatBox->addItem(i18nc("@label:listbox", "JPEG"),      QLatin1String("JPG"));
    d->formatBox->addItem(i18nc("@label:listbox", "TIFF"),      QLatin1String("TIFF"));
    d->formatBox->addItem(i18nc("@label:listbox", "PNG"),       QLatin1String("PNG"));
    d->formatBox->addItem(i18nc("@label:listbox", "PGF"),       QLatin1String("PGF"));
    d->formatBox->addItem(i18nc("@label:listbox", "JPEG 2000"), QLatin1String("JP2"));
    d->formatBox->insertSeparator(1);
    d->formatBox->insertSeparator(4);

    d->formatBox->setWhatsThis(xi18nc("@info:whatsthis",
                                     "<title>Default File Format for Saving</title>"
                                     "<para>Select the file format in which edited images are saved automatically. "
                                     "Format-specific options, like compression settings, "
                                     "can be configured on the <interface>Format Options</interface> tab.</para>"
                                     "<para><list>"
                                     // Lossy: JPEG
                                     "<item>"
                                     "<emphasis strong='true'>JPEG</emphasis>: "
                                     "JPEG is the most commonly used file format, but it employs lossy compression, "
                                     "which means that with each saving operation some image information will be irreversibly lost. "
                                     "JPEG offers a good compression rate, resulting in smaller files. "
                                     "</item>"
                                     // Traditional lossless: PNG, TIFF
                                     "<item>"
                                     "<emphasis strong='true'>PNG</emphasis>: "
                                     "A widely used format employing lossless compression. "
                                     "The files, though, will be larger because PNG does not achieve very good compression rates."
                                     "</item>"
                                     "<item>"
                                     "<emphasis strong='true'>TIFF</emphasis>: "
                                     "A commonly used format, usually uncompressed or with modest lossless compression. "
                                     "Resulting files will be large, but without quality loss due to compression. "
                                     "</item>"
                                     // Modern lossless: PGF, JPEG 2000
                                     "<item>"
                                     "<emphasis strong='true'>PGF</emphasis>: "
                                     "This is a technically superior file format offering good compression rates "
                                     "with either lossy or lossless compression. "
                                     "But it is not yet widely used and supported, so your friends may not directly be able to open these files, "
                                     "and you may not be able to directly publish them on the web. "
                                     "</item>"
                                     "<item>"
                                     "<emphasis strong='true'>JPEG 2000</emphasis>: "
                                     "JPEG 2000 is similar to PGF. Loading or saving is slower, the compression rate is better, "
                                     "and the format more widely supported, though still not comparable "
                                     "to the tradition formats JPEG, PNG or TIFF. "
                                     "</item>"
                                     "</list></para>"));

    d->infoFormat = new QPushButton;
    d->infoFormat->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
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

    d->closingGB                     = new QGroupBox;//(i18nc("@title:group", "Automatic Saving"));
    QGridLayout* const closingLayout = new QGridLayout;

    QLabel* const closingExplanation = new QLabel(i18nc("@label", "When closing the editor"));
    QLabel* const closingIcon        = new QLabel;
    closingIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-ok-apply")).pixmap(32));
    d->askToSave                     = new QRadioButton(i18nc("@option:radio", "Always ask to save changes"));
    d->autoSave                      = new QRadioButton(i18nc("@option:radio", "Save changes automatically"));

    closingLayout->addWidget(closingIcon,        0, 0);
    closingLayout->addWidget(closingExplanation, 0, 1);
    closingLayout->addWidget(d->askToSave,       1, 0, 1, 3);
    closingLayout->addWidget(d->autoSave,        2, 0, 1, 3);
    closingLayout->setColumnStretch(3, 1);

    d->closingGB->setLayout(closingLayout);

    // ---

    // --------------------------------------------------------

/*
    QGridLayout* const snapshotHeader = new QGridLayout;

    QLabel* const snapshotExplanation = new QLabel;
    snapshotExplanation->setText(i18nc("@label",
                                       "For an edited image, there is at least one file representing the current version."
                                       "digiKam can take and keep additional, intermediate snapshots during editing."));
    snapshotExplanation->setWordWrap(true);


    snapshotHeader->addWidget(snapshotIconLabel,       0, 0);
    snapshotHeader->addWidget(snapshotExplanation,     0, 1);
*/

    d->snapshotGB                     = new QGroupBox;//(i18nc("@title:group", "Intermediate Version Snapshots"));
    QGridLayout* const snapshotLayout = new QGridLayout;

    QString snapshotWhatsThis = xi18nc("@info:whatsthis",
                                      "<para>First and foremost, the <emphasis>original image will never be overwritten.</emphasis> "
                                      "Instead, when an image is edited, a new file is created: "
                                      "The <interface>current version</interface>.</para> "
                                      "<para>You can also create multiple <interface>versions</interface> "
                                      "deriving from the same <interface>original image</interface>.</para> "
                                      "<para>In addition to these files representing a current version, "
                                      "digiKam can take and keep additional, <interface>intermediate snapshots</interface> "
                                      "during the editing process. "
                                      "This can be useful if you want to preserve the intermediate steps for later "
                                      "access, for example if some editing steps cannot be automatically reproduced.</para>");
    d->snapshotGB->setWhatsThis(snapshotWhatsThis);

    QLabel* const snapshotIconLabel = new QLabel;
    snapshotIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("insert-image")).pixmap(32));

    QLabel* const snapshotLabel     = new QLabel(i18nc("@label", "Keep a snapshot of an edited image"));

    d->infoSnapshot           = new QPushButton;
    d->infoSnapshot->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
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

    d->viewGB = new QGroupBox;
    QGridLayout* const viewGBLayout = new QGridLayout;

    QString viewWhatsThis = xi18nc("@info:whatsthis",
                                  "<para>If an image has been edited, only the <interface>current versions</interface> "
                                  "will be shown in the main thumbnail view. "
                                  "From the <interface>right sidebar</interface>, you always have access to all hidden files.</para> "
                                  "<para>With the options here, you can choose to show certain files permanently.</para>");
    d->viewGB->setWhatsThis(viewWhatsThis);

    QLabel* const viewLabel     =  new QLabel(i18nc("@label", "In main view"));

    QLabel* const viewIconLabel = new QLabel;
    viewIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("view-list-icons")).pixmap(32));

    d->infoView           = new QPushButton;
    d->infoView->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
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
    else
    {
        settings.showInViewFlags &= ~VersionManagerSettings::ShowOriginal;
    }

    if (d->viewShowSnapshots->isChecked())
    {
        settings.showInViewFlags |= VersionManagerSettings::ShowIntermediates;
    }

    if (d->autoSave->isChecked())
    {
        settings.editorClosingMode = VersionManagerSettings::AutoSave;
    }
    else // d->askToSave->isChecked()
    {
        settings.editorClosingMode = VersionManagerSettings::AlwaysAsk;
    }

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

    d->formatBox->setCurrentIndex(d->formatBox->findData(settings.format));
}

void SetupVersioning::showNonDestructiveInformation()
{
    qApp->postEvent(d->enableNonDestructive, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0),
                                                            d->enableNonDestructive->mapToGlobal(QPoint(0, 0))));
}

void SetupVersioning::showFormatInformation()
{
    qApp->postEvent(d->formatBox, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), d->formatBox->mapToGlobal(QPoint(0, 0))));
}

void SetupVersioning::showSnapshotInformation()
{
    QPoint p(0, 0);
    qApp->postEvent(d->snapshotGB, new QHelpEvent(QEvent::WhatsThis, p, d->snapshotGB->mapToGlobal(p)));
}

void SetupVersioning::showViewInformation()
{
    QPoint p(0, 0);
    qApp->postEvent(d->viewGB, new QHelpEvent(QEvent::WhatsThis, p, d->viewGB->mapToGlobal(p)));
}

void SetupVersioning::enableToggled(bool on)
{
    d->workspaceGB->setEnabled(on);
    d->closingGB->setEnabled(on);
    d->snapshotGB->setEnabled(on);
    d->viewGB->setEnabled(on);
}

} // namespace Digikam
