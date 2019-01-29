/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

// local includes

#include "twittermpform.h"

// Qt includes

#include <QFile>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

namespace DigikamGenericTwitterPlugin
{

TwMPForm::TwMPForm()
{
	m_boundary = "00TwDK";
	m_boundary += Digikam::WSToolUtils::randomString(42 + 13).toLatin1();
	m_boundary += "KDwT99";
}

TwMPForm::~TwMPForm()
{
}

void TwMPForm::reset()
{
	m_buffer.resize(0);
}

bool TwMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

	QByteArray data("--");
	data += m_boundary;
	data += "\r\n";
	data += "Content-Disposition: form-data; name=\"media\"; filename=\"";
	data += QFile::encodeName(QUrl(imgPath).fileName());
	data += "\"\r\n";
	data += "Content-Type: application/octet-stream\r\n\r\n";
	data += file.readAll();
	file.close();
	data += "\r\n--";
	data += m_boundary;
	data += "--\r\n";

    m_buffer.append(data);

    return true;
}

QString TwMPForm::contentType() const
{
	QString type = QString::fromLatin1("multipart/form-data, boundary=\"%1\"").arg(QLatin1String(m_boundary));
	qCDebug(DIGIKAM_WEBSERVICES_LOG) << type;

	return type;
}

QByteArray TwMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericTwitterPlugin
