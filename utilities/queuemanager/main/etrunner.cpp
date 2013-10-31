
#include <QFileInfo>
#include <QTimer>

#include <kcomponentdata.h>
#include <kdebug.h>
#include <ktemporaryfile.h>
#include <klocalizedstring.h>

#include "etrunner.h"


ETRunner::ETRunner(QObject* parent, const QString& tool, const QList<KUrl>& urls)
    : QObject(parent)
    , toolcfg_(ETConfig::config(tool))
    , urls_(urls)
    , process_(new KProcess(this))
{}

ETRunner::~ETRunner()
{}

bool ETRunner::run()
{
    process_->setOutputChannelMode(KProcess::SeparateChannels);

    connect(process_, SIGNAL(error(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)));
    connect(process_, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(process_, SIGNAL(started()), SLOT(onStarted()));

    KTemporaryFile* script = new KTemporaryFile();
    script->setParent(process_);

    if (!script->open() || !script->setPermissions(QFile::ReadOwner | QFile::WriteOwner))
    {
        Q_EMIT error(i18n("Can't execute script"), i18n("Can't create script file"));
        Q_EMIT finished();
        return false;
    }

    if (script->write(toolcfg_->cfg.readEntry(ETConfig::script, QString()).toLocal8Bit()) == -1)
    {
        Q_EMIT error(i18n("Can't execute script"), i18n("Can't write to script file"));
        Q_EMIT finished();
        return false;
    }

    script->flush();

    process_->setProgram(toolcfg_->cfg.readEntry(ETConfig::interpretter, "/bin/sh"), QStringList() << QFileInfo(*script).absoluteFilePath());
    process_->start();
    return true;
}

void ETRunner::terminate()
{
    process_->terminate();
    QTimer::singleShot(5000, process_, SLOT(kill()));
}

void ETRunner::wait()
{
    process_->waitForFinished();
}

inline QString errorString(QProcess::ProcessError error)
{
    QMap<QProcess::ProcessError, QString> map;
    map[QProcess::FailedToStart] = i18n("Failed to start(file not found or resource error)");
    map[QProcess::Crashed] = i18n("Crashed");
    map[QProcess::Timedout] = i18n("Timedout");
    map[QProcess::ReadError] = i18n("Read error");
    map[QProcess::WriteError] = i18n("Write error");
    map[QProcess::UnknownError] = i18n("Unknown error");
    return map[error];
}

void ETRunner::onError(QProcess::ProcessError error)
{
    process_->setProperty("error", errorString(error));

    if (error == QProcess::FailedToStart)
    {
        onFinished(-1, QProcess::CrashExit);
    }
}

void ETRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    QString err = process_->property("error").toString();

    err = (err.isEmpty())
          ? process_->readAllStandardError()
          : QString("%1\n%2").arg(err).arg(QString(process_->readAllStandardError()));

    if (exitStatus == QProcess::NormalExit)
    {
        if (exitCode != 0)
        {
            Q_EMIT error(i18n("Tool exit with error"), err);
        }
    }
    else
    {
        Q_EMIT error(i18n("Tool crashed"), err);
    }

    Q_EMIT finished();
}

void ETRunner::onStarted()
{
    foreach(const KUrl url, urls_)
    {
        process_->write(url.path().toLocal8Bit() + "\n");
    }
    process_->closeWriteChannel();
    Q_EMIT started();    
}
