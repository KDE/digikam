/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-24
 * Description : user script batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Hubert Law <hhclaw dot eb at gmail dot com>
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

#include "userscript.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QWidget>
#include <QProcess>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dimg.h"
#include "dcombobox.h"
#include "dmetadata.h"
#include "tagscache.h"
#include "dlayoutbox.h"

namespace Digikam
{

class UserScript::Private
{
public:

    enum OutputFileType
    {
        Input = 0,
        JPEG,
        PNG,
        TIFF
    };

public:

    Private() :
        comboBox(0),
        textEdit(0),
        changeSettings(true)
    {
    }

    DComboBox* comboBox;
    QTextEdit* textEdit;

    bool       changeSettings;
};

UserScript::UserScript(QObject* parent)
    : BatchTool(QLatin1String("UserScript"), CustomTool, parent),
      d(new Private)
{
    setToolTitle(i18n("User Shell Script"));
    setToolDescription(i18n("Execute a custom shell script"));
    setToolIconName(QLatin1String("text-x-script"));
}

UserScript::~UserScript()
{
    delete d;
}

void UserScript::registerSettingsWidget()
{
    DVBox* const vbox    = new DVBox;

    QLabel* const label1 = new QLabel(vbox);
    label1->setText(i18n("Output file type:"));

    d->comboBox          = new DComboBox(vbox);
    d->comboBox->insertItem(Private::Input, i18n("Same as input"));
    d->comboBox->insertItem(Private::JPEG,  i18n("JPEG"));
    d->comboBox->insertItem(Private::PNG,   i18n("PNG"));
    d->comboBox->insertItem(Private::TIFF,  i18n("TIFF"));
    d->comboBox->setDefaultIndex(Private::Input);

    QLabel* const label2 = new QLabel(vbox);
    label2->setText(i18n("Shell Script:"));

    d->textEdit          = new QTextEdit(vbox);
    d->textEdit->setAcceptRichText(false);
    d->textEdit->setPlaceholderText(i18n("Enter script for execution. Use $INPUT and $OUTPUT for input / output filenames (with "
                                         "special characters escaped). These would be substituted before shell execution."));

    QLabel* const label3 = new QLabel(i18n("<b>Note:</b> Environment variables TITLE, COMMENTS, COLORLABEL, PICKLABEL, "
                                           "RATING and TAGSPATH (separated by ;) are available."), vbox);
    label3->setWordWrap(true);
    label3->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel* const space  = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings UserScript::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("Output filetype"), d->comboBox->defaultIndex());
    settings.insert(QLatin1String("Script"),          QString());
    return settings;
}

void UserScript::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->comboBox->setCurrentIndex(settings()[QLatin1String("Output filetype")].toInt());

    QString txt       = settings()[QLatin1String("Script")].toString();

    if (d->textEdit->toPlainText() != txt)
    {
        d->textEdit->setPlainText(txt);
    }

    d->changeSettings = true;
}

void UserScript::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Output filetype"), d->comboBox->currentIndex());
        settings.insert(QLatin1String("Script"),          d->textEdit->toPlainText());
        BatchTool::slotSettingsChanged(settings);
    }
}

QString UserScript::outputSuffix () const
{
    int filetype = settings()[QLatin1String("Output filetype")].toInt();

    switch (filetype)
    {
        case Private::JPEG:
            return QLatin1String("jpg");
            break;

        case Private::PNG:
            return QLatin1String("png");
            break;

        case Private::TIFF:
            return QLatin1String("tif");
            break;

        default:
            break;
    }

    // Return "": use original type
    return (BatchTool::outputSuffix());
}

bool UserScript::toolOperations()
{
    QString script = settings()[QLatin1String("Script")].toString();

    if (script.isEmpty())
    {
        setErrorDescription(i18n("User Script: No script."));
        return false;
    }

    // Replace all occurrences of $INPUT and $OUTPUT in script to file names. Case sensitive.
#ifndef Q_OS_WIN
    script.replace(QLatin1String("$INPUT"),  QLatin1Char('"') + inputUrl().toLocalFile()  + QLatin1Char('"'));
    script.replace(QLatin1String("$OUTPUT"), QLatin1Char('"') + outputUrl().toLocalFile() + QLatin1Char('"'));
#else
    script.replace(QLatin1String("$INPUT"),  QLatin1Char('"') + QDir::toNativeSeparators(inputUrl().toLocalFile())  + QLatin1Char('"'));
    script.replace(QLatin1String("$OUTPUT"), QLatin1Char('"') + QDir::toNativeSeparators(outputUrl().toLocalFile()) + QLatin1Char('"'));
#endif // Q_OS_WIN

    // Empties d->image, not to pass it to the next tool in chain
    setImageData(DImg());

    QProcess process(this);

    QProcessEnvironment env = adjustedEnvironmentForAppImage();

    QString tagPath = TagsCache::instance()->tagPaths(imageInfo().tagIds(), TagsCache::NoLeadingSlash,
                                                      TagsCache::NoHiddenTags).join(QLatin1Char(';'));

    // Populate env variables from metadata
    env.insert(QLatin1String("COLORLABEL"), QString::number(imageInfo().colorLabel()));
    env.insert(QLatin1String("PICKLABEL"),  QString::number(imageInfo().pickLabel()));
    env.insert(QLatin1String("RATING"),     QString::number(imageInfo().rating()));
    env.insert(QLatin1String("COMMENTS"),   imageInfo().comment());
    env.insert(QLatin1String("TITLE"),      imageInfo().title());
    env.insert(QLatin1String("TAGSPATH"),   tagPath);

    process.setProcessEnvironment(env);

    // call the shell script
#ifndef Q_OS_WIN
    process.start(QLatin1String("/bin/sh"), QStringList() << QLatin1String("-c") << script);
#else
    script.replace(QLatin1String("\n"), QLatin1String(" & "));

    process.setNativeArguments(QLatin1String("/C ") + script);

    process.start(QLatin1String("cmd.exe"));
#endif // Q_OS_WIN

    if (!process.waitForFinished(60000))
    {
        setErrorDescription(i18n("User Script: Timeout from script."));
        process.kill();
        return false;
    }

    if (process.exitCode() == -2)
    {
        setErrorDescription(i18n("User Script: Failed to start script."));
        return false;
    }
    else if (process.exitCode() == -1)
    {
        setErrorDescription(i18n("User Script: Script process crashed."));
        return false;
    }
    else if (process.exitCode() == 127)
    {
        setErrorDescription(i18n("User Script: Command not found."));
        return false;
    }

    return true;
}

}  // namespace Digikam
