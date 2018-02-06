/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect binary program and version
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "dbinaryiface.h"

// Qt includes

#include <QProcess>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dfiledialog.h"
#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

DBinaryIface::DBinaryIface(const QString& binaryName, const QString& projectName, const QString& url,
                           const QString& toolName, const QStringList& args, const QString& desc)
    : m_checkVersion(false),
      m_headerStarts(QLatin1String("")),
      m_headerLine(0),
      m_minimalVersion(QLatin1String("")),
      m_configGroup(!toolName.isEmpty() ? QString::fromLatin1("%1 Settings").arg(toolName) : QLatin1String("")),
      m_binaryBaseName(goodBaseName(binaryName)),
      m_binaryArguments(args),
      m_projectName(projectName),
      m_url(QUrl(url)),
      m_isFound(false),
      m_developmentVersion(false),
      m_version(QLatin1String("")),
      m_pathDir(QLatin1String("")),
      m_description(desc),
      m_pathWidget(0),
      m_binaryLabel(0),
      m_versionLabel(0),
      m_pathButton(0),
      m_downloadButton(0),
      m_lineEdit(0),
      m_statusIcon(0)
{
}

DBinaryIface::DBinaryIface(const QString& binaryName, const QString& minimalVersion, const QString& header,
                           const int headerLine, const QString& projectName, const QString& url,
                           const QString& toolName, const QStringList& args, const QString& desc)
    : m_checkVersion(true),
      m_headerStarts(header),
      m_headerLine(headerLine),
      m_minimalVersion(minimalVersion),
      m_configGroup(!toolName.isEmpty() ? QString::fromLatin1("%1 Settings").arg(toolName) : QLatin1String("")),
      m_binaryBaseName(goodBaseName(binaryName)),
      m_binaryArguments(args),
      m_projectName(projectName),
      m_url(QUrl(url)),
      m_isFound(false),
      m_developmentVersion(false),
      m_version(QLatin1String("")),
      m_pathDir(QLatin1String("")),
      m_description(desc),
      m_pathWidget(0),
      m_binaryLabel(0),
      m_versionLabel(0),
      m_pathButton(0),
      m_downloadButton(0),
      m_lineEdit(0),
      m_statusIcon(0)
{
}

DBinaryIface::~DBinaryIface()
{
}

const QString& DBinaryIface::version() const
{
    return m_version;
}

bool DBinaryIface::versionIsRight() const
{
    if (!m_checkVersion)
        return true;

    QRegExp reg(QLatin1String("^(\\d*[.]\\d*)"));
    version().indexOf(reg);
    float floatVersion = reg.capturedTexts()[0].toFloat();

    return (!version().isNull() &&
            isFound()           &&
            floatVersion >= minimalVersion().toFloat());
}

bool DBinaryIface::versionIsRight(const float customVersion) const
{
    if (!m_checkVersion)
        return true;

    QRegExp reg(QLatin1String("^(\\d*[.]\\d*)"));
    version().indexOf(reg);
    float floatVersion = reg.capturedTexts()[0].toFloat();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Found (" << isFound()
                                 << ") :: Version : " << version()
                                 << "(" << floatVersion
                                 << ")  [" << customVersion << "]";

    return (!version().isNull() &&
            isFound()           &&
            floatVersion >= customVersion);
}

QString DBinaryIface::findHeader(const QStringList& output, const QString& header) const
{
    foreach(const QString& s, output)
    {
        if (s.startsWith(header))
            return s;
    }

    return QString();
}

bool DBinaryIface::parseHeader(const QString& output)
{
    QString firstLine = output.section(QLatin1Char('\n'), m_headerLine, m_headerLine);
    qCDebug(DIGIKAM_GENERAL_LOG) << path() << " help header line: \n" << firstLine;

    if (firstLine.startsWith(m_headerStarts))
    {
        QString version = firstLine.remove(0, m_headerStarts.length());

        if (version.startsWith(QLatin1String("Pre-Release ")))
        {
            version.remove(QLatin1String("Pre-Release "));            // Special case with Hugin beta.
            m_developmentVersion = true;
        }

        setVersion(version);
        return true;
    }

    return false;
}

void DBinaryIface::setVersion(QString& version)
{
    QRegExp versionRegExp(QLatin1String("\\d*(\\.\\d+)*"));
    version.indexOf(versionRegExp);
    m_version = versionRegExp.capturedTexts()[0];
}

void DBinaryIface::slotNavigateAndCheck()
{
    QUrl start;

    if (isValid() && !m_pathDir.isEmpty())
    {
        start = QUrl::fromLocalFile(m_pathDir);
    }
    else
    {
#if defined Q_OS_OSX
        start = QUrl::fromLocalFile(QLatin1String("/Applications/"));
#elif defined Q_OS_WIN
        start = QUrl::fromLocalFile(QLatin1String("C:/Program Files/"));
#else
        start = QUrl::fromLocalFile(QLatin1String("/usr/bin/"));
#endif
    }

    QString f = DFileDialog::getOpenFileName(0, i18n("Navigate to %1", m_binaryBaseName),
                                             start.toLocalFile(),
                                             m_binaryBaseName);

    QString dir = QUrl::fromLocalFile(f).adjusted(QUrl::RemoveFilename).toLocalFile();
    m_searchPaths << dir;

    if (checkDir(dir))
    {
        emit signalSearchDirectoryAdded(dir);
    }
}

void DBinaryIface::slotAddPossibleSearchDirectory(const QString& dir)
{
    if (!isValid())
    {
        m_searchPaths << dir;
        checkDir(dir);
    }
    else
    {
        m_searchPaths << dir;
    }
}

void DBinaryIface::slotAddSearchDirectory(const QString& dir)
{
    m_searchPaths << dir;
    checkDir(dir);       // Forces the use of that directory
}

QString DBinaryIface::readConfig()
{
    if (m_configGroup.isEmpty())
        return QLatin1String("");

    KConfig config;
    KConfigGroup group = config.group(m_configGroup);
    return group.readPathEntry(QString::fromUtf8("%1Binary").arg(m_binaryBaseName), QLatin1String(""));
}

void DBinaryIface::writeConfig()
{
    if (m_configGroup.isEmpty())
        return;

    KConfig config;
    KConfigGroup group = config.group(m_configGroup);
    group.writePathEntry(QString::fromUtf8("%1Binary").arg(m_binaryBaseName), m_pathDir);
}

QString DBinaryIface::path(const QString& dir) const
{
    if (dir.isEmpty())
    {
        return baseName();
    }

    if (dir.endsWith(QLatin1Char('/')))
    {
        return QString::fromUtf8("%1%2").arg(dir).arg(baseName());
    }

    return QString::fromUtf8("%1%2%3").arg(dir).arg(QLatin1Char('/')).arg(baseName());
}

void DBinaryIface::setup(const QString& prev)
{
    QString previousDir = prev;

    if (!previousDir.isEmpty())
    {
        m_searchPaths << previousDir;
        checkDir(previousDir);
        return;
    }

    previousDir = readConfig();
    m_searchPaths << previousDir;
    checkDir(previousDir);

    if ((!previousDir.isEmpty()) && !isValid())
    {
        m_searchPaths << QLatin1String("");
        checkDir(QLatin1String(""));
    }
}

bool DBinaryIface::checkDir(const QString& possibleDir)
{
    bool ret             = false;
    QString possiblePath = path(possibleDir);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Testing " << possiblePath << "...";
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setProcessEnvironment(adjustedEnvironmentForAppImage());
    process.start(possiblePath, m_binaryArguments);

    bool val = process.waitForFinished();

    if (val && (process.error() != QProcess::FailedToStart))
    {
        m_isFound = true;

        if (m_checkVersion)
        {
            QString stdOut = QString::fromUtf8(process.readAllStandardOutput());

            if (parseHeader(stdOut))
            {
                m_pathDir = possibleDir;
                writeConfig();

                qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path() << " version: " << version();
                ret = true;
            }
            else
            {
                // TODO: do something if the version is not right or not found
            }
        }
        else
        {
            m_pathDir = possibleDir;
            writeConfig();

            qCDebug(DIGIKAM_GENERAL_LOG) << "Found " << path();
            ret = true;
        }
    }

    emit signalBinaryValid();
    return ret;
}

bool DBinaryIface::recheckDirectories()
{
    if (isValid())
    {
        // No need for recheck if it is already valid...
        return true;
    }

    foreach(const QString& dir, m_searchPaths)
    {
        checkDir(dir);

        if (isValid())
        {
            return true;
        }
    }

    return false;
}

QString DBinaryIface::goodBaseName(const QString& b)
{
#ifdef Q_OS_WIN
    return b + QLatin1String(".exe");
#else
    return b;
#endif // Q_OS_WIN
}

} // namespace Digikam
