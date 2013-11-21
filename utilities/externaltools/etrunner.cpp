
#include <QFileInfo>
#include <QTimer>

#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kshell.h>
#include <ktemporaryfile.h>

#include "etrunner.h"

typedef ETConfig::Type::Executable   Exec;
typedef ETConfig::Type::Script       Script;
typedef ETConfig::Type::SimpleScript SimpleScript;

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

    if (!setupProcess())
    {
        Q_EMIT finished();
        return false;
    }
    
    process_->start();   
    return true;
}

bool ETRunner::setupProcess()
{
    const QString type = toolcfg_->readEntry(ETConfig::type, QString());
    QString     program;
    QStringList arguments;
    
    const QString defprog = "/bin/sh";
    
    if (type == Exec::type)
    {
        KShell::Errors err;
        
        program   = toolcfg_->readTypeEntry(type, Exec::path, defprog);
        arguments = KShell::splitArgs(toolcfg_->readTypeEntry(type, Exec::arguments, QString()), KShell::NoOptions, &err);
        if (err != KShell::NoError)
        {
            Q_EMIT error(i18n("Can't execute program"), i18n("Bad quoting in arguments"));
            return false;
        }
    }
    else if (type == Script::type)
    {
        program   = toolcfg_->readTypeEntry(type, Script::path, defprog);
        arguments << toolcfg_->readTypeEntry(type, Script::script, QString());
    }
    else if (type == SimpleScript::type)
    {
        KTemporaryFile* script = new KTemporaryFile();
        script->setParent(process_);

        if (!script->open() || !script->setPermissions(QFile::ReadOwner | QFile::WriteOwner))
        {
            Q_EMIT error(i18n("Can't execute script"), i18n("Can't create script file"));
            return false;
        }
        
        if (script->write(toolcfg_->readTypeEntry(type, SimpleScript::body, QString()).toLocal8Bit()) == -1)
        {
            Q_EMIT error(i18n("Can't execute script"), i18n("Can't write to script file"));
            return false;
        }

        script->flush();
        program   = toolcfg_->readTypeEntry(type, SimpleScript::path, defprog);
        arguments << QFileInfo(*script).absoluteFilePath();
    }
    else
    {
        return false;
    }
    
    process_->setProgram(program, arguments);
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
