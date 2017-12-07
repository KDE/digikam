/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "setupdatabase.h"

// Qt includes

#include <QCursor>
#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QHelpEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QSpinBox>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QApplication>
#include <QUrl>
#include <QIcon>
#include <QMessageBox>
#include <QWhatsThis>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummanager.h"
#include "applicationsettings.h"
#include "coredbschemaupdater.h"
#include "databaseserverstarter.h"
#include "dbengineparameters.h"
#include "dbsettingswidget.h"
#include "digikam_debug.h"
#include "scancontroller.h"
#include "setuputils.h"

namespace Digikam
{

class SetupDatabase::Private
{
public:

    Private() :
        databaseWidget(0),
        updateBox(0),
        hashesButton(0),
        ignoreEdit(0),
        ignoreLabel(0)
    {
    }

    DatabaseSettingsWidget* databaseWidget;
    QGroupBox*              updateBox;
    QPushButton*            hashesButton;
    QLineEdit*              ignoreEdit;
    QLabel*                 ignoreLabel;
};

SetupDatabase::SetupDatabase(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QTabWidget* tab = new QTabWidget(viewport());
    setWidget(tab);
    setWidgetResizable(true);

    // --------------------------------------------------------

    QWidget* const settingsPanel = new QWidget(tab);
    QVBoxLayout* settingsLayout  = new QVBoxLayout(settingsPanel);

    d->databaseWidget = new DatabaseSettingsWidget;
    settingsLayout->addWidget(d->databaseWidget);

    if (!CoreDbSchemaUpdater::isUniqueHashUpToDate())
    {
        createUpdateBox();
        settingsLayout->addStretch(10);
        settingsLayout->addWidget(d->updateBox);
    }

    tab->insertTab(DbSettings, settingsPanel, i18nc("@title:tab", "Database Settings"));

    // --------------------------------------------------------

    QWidget* const ignorePanel = new QWidget(tab);
    QGridLayout* ignoreLayout    = new QGridLayout(ignorePanel);

    QLabel* const ignoreInfoLabel = new QLabel(
                i18n("<p>Set the names of directories that you want to ignore "
                     "from your photo collections. The names are case sensitive "
                     "and should be separated by a semicolon.</p>"
                     "<p>This is for example useful when you store your photos "
                     "on a Synology NAS (Network Attached Storage). In every "
                     "directory the system creates a subdirectory @eaDir to "
                     "store thumbnails. To avoid digiKam inserting the original "
                     "photo and its corresponding thumbnail twice, @eaDir is "
                     "ignored by default.</p>"
                     "<p>To re-include directories that are ignored by default "
                     "prefix it with a minus, e.g. -@eaDir.</p>"),
                ignorePanel);
    ignoreInfoLabel->setWordWrap(true);

    QLabel* const logoLabel1 = new QLabel(ignorePanel);
    logoLabel1->setPixmap(QIcon::fromTheme(QLatin1String("folder")).pixmap(48));

    d->ignoreLabel = new QLabel(ignorePanel);
    d->ignoreLabel->setText(i18n("Additional directories to ignore "
                                 "(<a href='image'>Currently ignored directories</a>):"));

    d->ignoreEdit = new QLineEdit(ignorePanel);
    d->ignoreEdit->setClearButtonEnabled(true);
    d->ignoreEdit->setPlaceholderText(i18n("Enter directories that you want to "
                                           "ignore from adding to your collections."));
    ignoreInfoLabel->setBuddy(d->ignoreEdit);

    int row = 0;
    ignoreLayout->addWidget(ignoreInfoLabel, row, 0, 1, 2);
    row++;
    ignoreLayout->addWidget(logoLabel1,      row, 0, 2, 1);
    ignoreLayout->addWidget(d->ignoreLabel,  row, 1, 1, 1);
    row++;
    ignoreLayout->addWidget(d->ignoreEdit,   row, 1, 1, 1);
    row++;
    ignoreLayout->setColumnStretch(1, 10);
    ignoreLayout->setRowStretch(row, 10);
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    ignoreLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    ignoreLayout->setSpacing(spacing);

    connect(d->ignoreLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentIgnoredDirectoriesSettings()));

    tab->insertTab(IgnoreDirs, ignorePanel, i18nc("@title:tab", "Ignored Directories"));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupDatabase::~SetupDatabase()
{
    delete d;
}

void SetupDatabase::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    QString ignoreDirectory;
    CoreDbAccess().db()->getUserIgnoreDirectoryFilterSettings(&ignoreDirectory);
    if (d->ignoreEdit->text() != ignoreDirectory)
    {
        CoreDbAccess().db()->setUserIgnoreDirectoryFilterSettings(
                    cleanUserFilterString(d->ignoreEdit->text(), true, true));

        ScanController::instance()->completeCollectionScanInBackground(false);
    }

    if (d->databaseWidget->getDbEngineParameters() == d->databaseWidget->orgDatabasePrm())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No DB settings changes. Do nothing...";
        return;
    }

    if (!d->databaseWidget->checkDatabaseSettings())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DB settings check invalid. Do nothing...";
        return;
    }

    switch (d->databaseWidget->databaseType())
    {
        case DatabaseSettingsWidget::SQlite:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to SQlite DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }
        case DatabaseSettingsWidget::MysqlInternal:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to Mysql Internal DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }
        default: // DatabaseSettingsWidget::MysqlServer
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to Mysql server DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }
    }
}

void SetupDatabase::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    QString ignoreDirectory;
    CoreDbAccess().db()->getUserIgnoreDirectoryFilterSettings(&ignoreDirectory);
    d->ignoreEdit->setText(ignoreDirectory);

    d->databaseWidget->setParametersFromSettings(settings);
}

void SetupDatabase::upgradeUniqueHashes()
{
    int result = QMessageBox::warning(this, qApp->applicationName(),
                                      i18nc("@info",
                                            "<p>The process of updating the file hashes takes a few minutes.</p> "
                                            "<p>Please ensure that any important collections on removable media are connected. "
                                            "<i>After the upgrade you cannot use your database with a digiKam version "
                                            "prior to 2.0.</i></p> "
                                            "<p>Do you want to begin the update?</p>"),
                                            QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        ScanController::instance()->updateUniqueHash();
    }
}

void SetupDatabase::createUpdateBox()
{
    d->updateBox                    = new QGroupBox(i18nc("@title:group", "Updates"));
    QGridLayout* const updateLayout = new QGridLayout;

    d->hashesButton                 = new QPushButton(i18nc("@action:button", "Update File Hashes"));
    d->hashesButton->setWhatsThis(i18nc("@info:tooltip",
                                        "<qt>File hashes are used to identify identical files and to display thumbnails. "
                                        "A new, improved algorithm to create the hash is now used. "
                                        "The old algorithm, though, still works quite well, so it is recommended to "
                                        "carry out this upgrade, but not required.<br> "
                                        "After the upgrade you cannot use your database with a digiKam version "
                                        "prior to 2.0.</qt>"));

    QPushButton* const infoHash     = new QPushButton;
    infoHash->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    infoHash->setToolTip(i18nc("@info:tooltip", "Get information about <interface>Update File Hashes</interface>"));

    updateLayout->addWidget(d->hashesButton, 0, 0);
    updateLayout->addWidget(infoHash,        0, 1);
    updateLayout->setColumnStretch(2, 1);

    d->updateBox->setLayout(updateLayout);

    connect(d->hashesButton, SIGNAL(clicked()),
            this, SLOT(upgradeUniqueHashes()));

    connect(infoHash, SIGNAL(clicked()),
            this, SLOT(showHashInformation()));
}

void SetupDatabase::showHashInformation()
{
    qApp->postEvent(d->hashesButton, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
}

void SetupDatabase::slotShowCurrentIgnoredDirectoriesSettings() const
{
    QStringList ignoreDirectoryList;
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectoryList);
    QString text = i18n("<p>Directories starting with a dot are ignored by "
                        "default.<br/> <code>%1</code></p>",
                        ignoreDirectoryList.join(QLatin1Char(';')));
    QWhatsThis::showText(d->ignoreLabel->mapToGlobal(QPoint(0, 0)), text, d->ignoreLabel);
}


} // namespace Digikam
