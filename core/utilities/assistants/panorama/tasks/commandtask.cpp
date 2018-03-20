/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-07
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "commandtask.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

CommandTask::CommandTask(PanoAction action, const QString& workDirPath, const QString& commandPath)
    : PanoTask(action, workDirPath),
      process(0),
      commandPath(commandPath)
{
}

CommandTask::~CommandTask()
{
}

void CommandTask::requestAbort()
{
    PanoTask::requestAbort();
    if (!process.isNull())
        process->kill();
}

void CommandTask::runProcess(QStringList& args)
{
    if (isAbortedFlag)
        return;

    process.reset(new QProcess());
    process->setWorkingDirectory(tmpDir.toLocalFile());
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setProcessEnvironment(adjustedEnvironmentForAppImage());
    process->setProgram(commandPath);
    process->setArguments(args);
    process->start();

    successFlag = process->waitForFinished(-1) && process->exitStatus() == QProcess::NormalExit;
    output      = QString::fromLocal8Bit(process->readAll());

    if (!successFlag)
        errString = getProcessError();
}

QString CommandTask::getProgram()
{
    if (process.isNull())
        return QString();
    return process->program();
}

QString CommandTask::getCommandLine()
{
    if (process.isNull())
        return QString();
    return (process->program() + QChar::fromLatin1(' ') + process->arguments().join(QChar::fromLatin1(' ')));
}

QString CommandTask::getProcessError()
{
    if (isAbortedFlag)
        return i18n("<b>Canceled</b>");
    if (process.isNull())
        return QString();
    return (i18n("<b>Cannot run <i>%1</i>:</b><p>%2</p>",
                 getProgram(),
                 output.toHtmlEscaped().replace(QLatin1String("\n"), QLatin1String("<br />"))));
}

void CommandTask::printDebug(const QString& binaryName)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << binaryName << "command line: " << getCommandLine();
    qCDebug(DIGIKAM_GENERAL_LOG) << binaryName << "output:" << endl << qPrintable(QLatin1String(" >>\t") + output.replace(QLatin1String("\n"), QLatin1String("\n >>\t")));
}

}  // namespace Digikam
