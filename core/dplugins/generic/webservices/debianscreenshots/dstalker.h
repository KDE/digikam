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

#ifndef DIGIKAM_DS_TALKER_H
#define DIGIKAM_DS_TALKER_H

// KDE includes

#include <kio/job.h>

namespace GenericDigikamDebianScreenshotsPlugin
{

class DSTalker : public QObject
{
    Q_OBJECT

public:

    explicit DSTalker(QWidget* const parent);
    ~DSTalker();

    bool addScreenshot(const QString& imgPath, const QString& packageName,
                       const QString& packageVersion = QString(),
                       const QString& description = QString() );

Q_SIGNALS:

     void signalBusy(bool val);
     void signalAddScreenshotDone(int errCode, const QString& errMsg);

private Q_SLOTS:

     void data(KIO::Job* job, const QByteArray& data);
     void slotResult(KJob* job);

private:

    QByteArray      m_buffer;

    QString         m_userAgent;
    QString         m_uploadUrl;
    KIO::Job*       m_job;
};

} // namespace GenericDigikamDebianScreenshotsPlugin

#endif // DIGIKAM_DS_TALKER_H
