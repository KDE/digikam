/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible
 *               location
 *
 * Copyright (C) 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "KioExportWidget.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kcombobox.h>

// Local includes

#include "kipiplugins_debug.h"
#include "kpimageslist.h"
#include "kputil.h"

namespace Digikam
{

KioExportWidget::KioExportWidget(QWidget* const parent)
    : QWidget(parent)
{
    // setup remote target selection

    KPHBox* const hbox  = new KPHBox(this);
    QLabel* const label = new QLabel(hbox);
    m_targetLabel       = new KUrlComboRequester(hbox);
    m_targetDialog      = 0;

    if (m_targetLabel->button())
        m_targetLabel->button()->hide();

    m_targetLabel->comboBox()->setEditable(true);

    label->setText(i18n("Target location: "));
    m_targetLabel->setWhatsThis(i18n("Sets the target address to upload the images to. "
                                     "This can be any address as used in Dolphin or Konqueror, "
                                     "e.g. ftp://my.server.org/sub/folder."));

    m_targetSearchButton = new QPushButton(i18n("Select target location..."), this);
    m_targetSearchButton->setIcon(QIcon::fromTheme(QString::fromLatin1("folder-remote")));

    // setup image list
    m_imageList = new KPImagesList(this);
    m_imageList->setAllowRAW(true);
    m_imageList->listView()->setWhatsThis(i18n("This is the list of images to upload "
                                               "to the specified target."));

    // layout dialog
    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(hbox);
    layout->addWidget(m_targetSearchButton);
    layout->addWidget(m_imageList);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(m_targetSearchButton, SIGNAL(clicked(bool)),
            this, SLOT(slotShowTargetDialogClicked(bool)));

    connect(m_targetLabel, SIGNAL(textChanged(QString)),
            this, SLOT(slotLabelUrlChanged()));

    // ------------------------------------------------------------------------

    updateTargetLabel();
}

KioExportWidget::~KioExportWidget()
{
}

QUrl KioExportWidget::targetUrl() const
{
    return m_targetUrl;
}

QList<QUrl> KioExportWidget::history() const
{
    QList<QUrl> urls;

    for (int i = 0 ; i <= m_targetLabel->comboBox()->count() ; i++)
        urls << QUrl(m_targetLabel->comboBox()->itemText(i));

    return urls;
}

void KioExportWidget::setHistory(const QList<QUrl>& urls)
{
    m_targetLabel->comboBox()->clear();

    foreach (QUrl url, urls)
        m_targetLabel->comboBox()->addUrl(url);
}

void KioExportWidget::setTargetUrl(const QUrl& url)
{
    m_targetUrl = url;
    updateTargetLabel();
}

void KioExportWidget::slotShowTargetDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    m_targetDialog = new QFileDialog(this, i18n("Select target..."),
                                     m_targetUrl.toString(), i18n("All Files (*)"));
    m_targetDialog->setAcceptMode(QFileDialog::AcceptSave);
    m_targetDialog->setFileMode(QFileDialog::DirectoryOnly);

    if (m_targetDialog->exec() == QDialog::Accepted)
    {
        m_targetUrl = m_targetDialog->selectedUrls().isEmpty() ? QUrl() : m_targetDialog->selectedUrls().at(0);
        updateTargetLabel();
        emit signalTargetUrlChanged(m_targetUrl);
    }

    delete m_targetDialog;
}

void KioExportWidget::updateTargetLabel()
{
    qCDebug(KIPIPLUGINS_LOG) << "Call for url "
                             << m_targetUrl.toDisplayString() << ", valid = "
                             << m_targetUrl.isValid();

    QString urlString = i18n("<not selected>");

    if (m_targetUrl.isValid())
    {
        urlString = m_targetUrl.toDisplayString();
        m_targetLabel->setUrl(QUrl(urlString));
    }
}

KPImagesList* KioExportWidget::imagesList() const
{
    return m_imageList;
}

void KioExportWidget::slotLabelUrlChanged()
{
    m_targetUrl = m_targetLabel->url();
    emit signalTargetUrlChanged(m_targetUrl);
}

} // namespace Digikam
