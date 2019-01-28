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

#include "dstalker.h"

// C++ includes

#include <ctime>

// Qt includes

#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QtAlgorithms>
#include <QApplication>

// KDE includes

#include <kio/job.h>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"
#include "dsmpform.h"
#include "dscommon.h"
#include "dmessagebox.h"

using namespace Digikam;

namespace DigikamGenericDebianScreenshotsPlugin
{

DSTalker::DSTalker(QWidget* const parent)
    : QObject(parent),
      m_job(0)
{
    m_userAgent = QString::fromUtf8("KIPI-Plugin-DebianScreenshots/%1 (pgquiles@elpauer.org)").arg(digiKamVersion());
    m_uploadUrl = DigikamGenericDebianScreenshotsPlugin::debshotsUrl + QLatin1String("/uploadfile");
}

DSTalker::~DSTalker()
{
    if (m_job)
    {
        m_job->kill();
    }
}

bool DSTalker::addScreenshot(const QString& imgPath, const QString& packageName,
                             const QString& packageVersion, const QString& description)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Adding screenshot " << imgPath << " to package "
             << packageName << " " << packageVersion<< " using description '" << description << "'";

    if (m_job)
    {
        m_job->kill();
        m_job = 0;
    }

    emit signalBusy(true);

    DSMPForm form;
    form.addPair(QLatin1String("packagename"), packageName);
    form.addPair(QLatin1String("version"), packageVersion);
    form.addPair(QLatin1String("description"), description);
    form.addFile(imgPath, imgPath, QLatin1String("file"));
    form.finish();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "FORM: " << endl << form.formData();

    KIO::TransferJob* const job = KIO::http_post(QUrl(m_uploadUrl), form.formData(), KIO::HideProgressInfo);
    job->addMetaData(QLatin1String("UserAgent"), m_userAgent);
    job->addMetaData(QLatin1String("content-type"), form.contentType());

    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(data(KIO::Job*,QByteArray)));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    m_job   = job;
    m_buffer.resize(0);
    return true;
}

 void DSTalker::data(KIO::Job*, const QByteArray& data)
 {
     qCDebug(DIGIKAM_WEBSERVICES_LOG) << Q_FUNC_INFO;

     if (data.isEmpty())
     {
         return;
     }

     int oldSize = m_buffer.size();
     m_buffer.resize(m_buffer.size() + data.size());
     memcpy(m_buffer.data() + oldSize, data.data(), data.size());
 }
 void DSTalker::slotResult(KJob* kjob)
 {
     m_job               = 0;
     KIO::Job* const job = static_cast<KIO::Job*>(kjob);

     if (job->error())
     {
         emit signalBusy(false);
         emit signalAddScreenshotDone(job->error(), job->errorText());
     }
/*
     else
     {
         qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Uploaded successfully screenshot "
                                          << job->queryMetaData("Screenshot")
                                          << " to Debian Screenshots for package "
                                          << job->queryMetaData("Package")
                                          << " " << job->queryMetaData("Version")
                                          << " with description "
                                          << job->queryMetaData("Description");
     }
*/
     emit signalBusy(false);
     emit signalAddScreenshotDone(0, QString());
 }

} // namespace DigikamGenericDebianScreenshotsPlugin
