/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-29
 * Description : a tool to export images to Debian Screenshots
 *
 * Copyright (C) 2010 by Pau Garcia i Quiles <pgquiles at elpauer dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dswidget.h"

// Qt includes

#include <QApplication>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QStandardItemModel>
#include <QCompleter>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDialog>
#include <QListView>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>
#include <kio/accessmanager.h>

// QJSON includes

//FIXME #include <qjson/parser.h>

// Local includes

#include "digikam_debug.h"
#include "dspackagedelegate.h"
#include "dscommon.h"

namespace GenericDigikamDebianScreenshotsPlugin
{

DSWidget::DSWidget(QWidget* const parent)
    : QWidget(parent),
      m_dlGrp(0),
      m_lastTip( QString() ),
      m_lastQueryUrl( QUrl() ),
      m_httpManager( new KIO::AccessManager(this) ),
      m_jsonManager( new KIO::AccessManager(this) )
{
    setObjectName(QLatin1String("DSWidget"));

    QHBoxLayout* const mainLayout = new QHBoxLayout(this);

    // -------------------------------------------------------------------

    m_imgList  = new DItemsList(this);
    m_imgList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    m_imgList->setAllowRAW(true);
    m_imgList->loadImagesFromCurrentSelection();
    m_imgList->listView()->setWhatsThis( i18n("This is the list of images to upload to Debian Screenshots.") );

    QWidget* const settingsBox           = new QWidget(this);
    QVBoxLayout* const settingsBoxLayout = new QVBoxLayout(settingsBox);

    m_headerLabel = new DActiveLabel(QUrl(GenericDigikamDebianScreenshotsPlugin::debshotsUrl),
                                     QLatin1String(":/debianscreenshots/sdnlogo.png"),
                                     settingsBox);
    m_headerLabel->setWhatsThis( i18n("This is a clickable link to open the Debian Screenshots home page in a web browser.") );

    QGroupBox* const pkgGroupBox   = new QGroupBox(settingsBox);
    pkgGroupBox->setTitle(i18n("Package"));
    pkgGroupBox->setWhatsThis(i18n("This is the Debian Screenshots package to which selected photos will be uploaded."));

    QGridLayout* const sdnLayout   = new QGridLayout(pkgGroupBox);

    QLabel* const pkgLabel         = new QLabel(i18n("Package:"), pkgGroupBox);

    m_pkgLineEdit                  = new QLineEdit(pkgGroupBox);
    QCompleter* const pkgCompleter = new QCompleter(this);
    pkgCompleter->setCompletionMode(QCompleter::PopupCompletion);
    pkgCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pkgLineEdit->setCompleter(pkgCompleter);

    QListView* const listView      = new QListView;
    pkgCompleter->setPopup(listView);
    listView->setItemDelegateForColumn(0, new DSPackageDelegate);

    connect(m_pkgLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotCompletePackageName(QString)));

    connect(m_httpManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotCompletePackageNameFinished(QNetworkReply*)));

    connect(pkgCompleter, SIGNAL(activated(QString)),
            this, SLOT(slotFindVersionsForPackage(QString)));

    connect(m_jsonManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFindVersionsForPackageFinished(QNetworkReply*)));

    QLabel* const versionLabel = new QLabel(i18n("Software version:"), pkgGroupBox);
    m_versionsComboBox         = new QComboBox(pkgGroupBox);
    m_versionsComboBox->setEditable(false);
    m_versionsComboBox->setEnabled(false); // Disable until we have a package name
    m_versionsComboBox->setMinimumContentsLength(40);

    connect(m_versionsComboBox, SIGNAL(activated(int)),
            this, SLOT(slotEnableUpload()));

    QLabel* const descriptionLabel  = new QLabel(i18n("Screenshot description:"), pkgGroupBox);
    m_descriptionLineEdit           = new QLineEdit(pkgGroupBox);
    m_descriptionLineEdit->setMaxLength(40); // 40 is taken from screenshots.debian.net/upload page source
    m_descriptionLineEdit->setEnabled(false);

    sdnLayout->addWidget(pkgLabel,              1, 0, 1, 1);
    sdnLayout->addWidget(m_pkgLineEdit,         1, 1, 1, 4);
    sdnLayout->addWidget(versionLabel,          2, 0, 1, 1);
    sdnLayout->addWidget(m_versionsComboBox,    2, 1, 1, 4);
    sdnLayout->addWidget(descriptionLabel,      3, 0, 1, 1);
    sdnLayout->addWidget(m_descriptionLineEdit, 3, 1, 1, 4);

    m_progressBar = new DProgressWdg(settingsBox);
    m_progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_progressBar->hide();

    settingsBoxLayout->addWidget(m_headerLabel);
    settingsBoxLayout->addWidget(pkgGroupBox);
    settingsBoxLayout->addWidget(m_progressBar);

    mainLayout->addWidget(m_imgList);
    mainLayout->addWidget(settingsBox);
    mainLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    mainLayout->setContentsMargins(QMargins());
}

DSWidget::~DSWidget()
{
}

DItemsList* DSWidget::imagesList() const
{
    return m_imgList;
}

DProgressWdg* DSWidget::progressBar() const
{
    return m_progressBar;
}

void DSWidget::slotCompletePackageName(const QString& tip)
{
    if ((!tip.isEmpty()) && (QString::compare(tip, m_lastTip, Qt::CaseInsensitive) != 0) )
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        m_versionsComboBox->clear();
        m_versionsComboBox->setEnabled(false);
        m_descriptionLineEdit->setEnabled(false);
        emit requiredPackageInfoAvailable(false);

        QUrl sdnUrl(GenericDigikamDebianScreenshotsPlugin::debshotsUrl +
                    QLatin1String("/packages/ajax_autocomplete_packages")); // DOES NOT RETURN JSON
        QUrlQuery query(sdnUrl);
        query.addQueryItem(QLatin1String("q"), tip);
        // No matter what 'limit' we use, s.d.n will always return 30 results
        query.addQueryItem(QLatin1String("limit"), QLatin1String("30"));
        sdnUrl.setQuery(query);
        
        QNetworkRequest request(sdnUrl);
        m_httpManager->get(request);
        m_lastQueryUrl = sdnUrl;
    }

    m_lastTip = tip;
}

void DSWidget::slotCompletePackageNameFinished(QNetworkReply* reply)
{
    QUrl replyUrl = reply->url();

    QApplication::restoreOverrideCursor();

    // Check if this is the reply for the last request, or a delayed reply we are receiving just now
    if ( QString::compare(replyUrl.toString(), m_lastQueryUrl.toString(), Qt::CaseInsensitive) != 0 )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received a delayed reply, discarding it";
        return; // It was a delayed reply, discard it
    }

    if ( reply->error() )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Didn't receive a reply for request "
                                         << replyUrl.toEncoded().constData() << " - "
                                         <<  qPrintable(reply->errorString());
    }
    else
    {
        QByteArray ba = reply->readAll();

        if( ba.isEmpty() )
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "No completion data received for request "
                                             << replyUrl.toEncoded().constData()
                                             << "(probably no package matches that pattern)";
            return;
        }

        QList<QByteArray> pkgSuggestions = ba.split('\n');

        QStandardItemModel* const m = new QStandardItemModel(pkgSuggestions.count(), 2, m_pkgLineEdit->completer());

        for ( int i = 0; i < pkgSuggestions.count(); ++i)
        {
            QModelIndex pkgIdx             = m->index(i, 0);
            QModelIndex descIdx            = m->index(i, 1);
            QList<QByteArray> pkgDescSplit = pkgSuggestions.at(i).split('|');
            QString pkg                    = QString::fromUtf8(pkgDescSplit.at(0));
            QString desc                   = QString::fromUtf8(pkgDescSplit.at(1));
            m->setData(pkgIdx, pkg);
            m->setData(descIdx, desc);
        }
        m_pkgLineEdit->completer()->setModel(m);
    }

    m_pkgLineEdit->completer()->complete();

    reply->deleteLater();
}

void DSWidget::slotFindVersionsForPackage(const QString& package)
{
    QUrl sdnVersionUrl(GenericDigikamDebianScreenshotsPlugin::debshotsUrl + 
                       QLatin1String("/packages/ajax_get_version_for_package")); // DOES RETURN JSON
    QUrlQuery query(sdnVersionUrl);
    query.addQueryItem(QLatin1String("q"),     QString::fromUtf8(QUrl::toPercentEncoding(package)));
    query.addQueryItem(QLatin1String("limit"), QLatin1String("30"));
    sdnVersionUrl.setQuery(query);

    QNetworkRequest request(sdnVersionUrl);
    m_jsonManager->get(request);
}

void DSWidget::slotFindVersionsForPackageFinished(QNetworkReply* reply)
{
    QUrl replyUrl = reply->url();

    if (reply->error())
    {
        qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Download of "
                                           << replyUrl.toEncoded().constData()
                                           << "failed: " << qPrintable(reply->errorString());
    }
    else
    {
        QByteArray ba = reply->readAll();
        bool ok       = false;

/* FIXME
        QJson::Parser jsonParser;
        QVariant versionSuggestions = jsonParser.parse(ba, &ok);
*/
        QVariant versionSuggestions;

        if (ok)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Query "
                                             << replyUrl.toEncoded().constData()
                                             << "succeeded";

            QMap<QString, QVariant> versions = versionSuggestions.toMap();

            QMap<QString, QVariant>::const_iterator i = versions.constBegin();

            while (i != versions.constEnd())
            {
                m_versionsComboBox->addItem(i.value().toString());
                ++i;
            }

            m_versionsComboBox->setEnabled(true);

            if ( versions.size() == 1 )
            {
                m_descriptionLineEdit->setEnabled(true);
                slotEnableUpload();
            }

        }
        else
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Query "
                                             << replyUrl.toEncoded().constData()
                                             << "failed";
        }
    }

    reply->deleteLater();
}

void DSWidget::slotEnableUpload()
{
    if (!m_imgList->imageUrls().isEmpty())
    {
        emit requiredPackageInfoAvailable(true);
    }
}

} // namespace GenericDigikamDebianScreenshotsPlugin
