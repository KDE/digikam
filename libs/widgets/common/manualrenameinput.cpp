/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : an input widget that allows manual renaming of files
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "manualrenameinput.h"
#include "manualrenameinput.moc"

// Qt includes

#include <QDateTime>
#include <QFileInfo>
#include <QGridLayout>
#include <QToolButton>

// KDE includes

#include <kicon.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

// Local includes

#include "dcursortracker.h"

namespace Digikam
{

class ManualRenameInputPriv
{
public:

    ManualRenameInputPriv()
    {
        parseStringLineEdit = 0;
        tooltipTracker      = 0;
        tooltipToggleButton = 0;
    }

    QToolButton* tooltipToggleButton;
    KLineEdit*   parseStringLineEdit;
    DTipTracker* tooltipTracker;
};

ManualRenameInput::ManualRenameInput(QWidget* parent)
                 : QWidget(parent), d(new ManualRenameInputPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------

    d->parseStringLineEdit  = new KLineEdit;
    d->tooltipToggleButton  = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->parseStringLineEdit, 1, 1, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton, 1, 2, 1, 1);
    mainLayout->setColumnStretch(1, 10);
    setLayout(mainLayout);

    // --------------------------------------------------------

    QString tooltip   = createToolTip();
    d->tooltipTracker = new DTipTracker(tooltip, d->parseStringLineEdit, Qt::AlignLeft);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);

    d->parseStringLineEdit->setWhatsThis(tooltip);
    d->parseStringLineEdit->setClearButtonShown(true);
    d->parseStringLineEdit->setCompletionMode(KGlobalSettings::CompletionAuto);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->parseStringLineEdit, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->tooltipTracker, SIGNAL(signalClicked()),
            this, SLOT(slotHideToolTipTracker()));
}

ManualRenameInput::~ManualRenameInput()
{
    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;
    delete d;
}

QString ManualRenameInput::text() const
{
    return d->parseStringLineEdit->text();
}

void ManualRenameInput::setText(const QString& text)
{
    d->parseStringLineEdit->setText(text);
}

void ManualRenameInput::setTrackerAlignment(Qt::Alignment alignment)
{
    d->tooltipTracker->setTrackerAlignment(alignment);
}

KLineEdit* ManualRenameInput::input() const
{
    return d->parseStringLineEdit;
}

void ManualRenameInput::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString ManualRenameInput::parser(const QString& parse,
                                  const QString &fileName, const QString &cameraName,
                                  const QDateTime &dateTime, int index)
{
    QFileInfo fi(fileName);
    QRegExp invalidParseString("^\\s*$");

    if (parse.isEmpty() || invalidParseString.exactMatch(parse))
        return fi.baseName();

    QString parsedString = parse;

    // parse sequence number token ----------------------------
    {
        QRegExp regExp("(#+)(\\{\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\})?");
        int pos     = 0;
        int slength = 0;
        int start   = 0;
        int step    = 0;
        int number  = 0;

        while (pos > -1)
        {
            pos = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                slength = regExp.cap(1).length();
                start   = regExp.cap(3).isEmpty() ? 1 : regExp.cap(3).toInt();
                step    = regExp.cap(4).isEmpty() ? 1 : regExp.cap(4).toInt();

                number  = start + ((index-1) * step);
                QString tmp = QString("%1").arg(number, slength, 10, QChar('0'));
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }
    // parse date time token ----------------------------------
    {
        QRegExp regExp("\\[date:(.*)\\]");
        regExp.setMinimal(true);
        int pos = 0;
        while (pos > -1)
        {
            pos  = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                QString tmp = dateTime.toString(regExp.cap(1));
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }
    // parse * token (first letter of each word upper case) ---
    {
        QRegExp regExp("\\*{1}");
        regExp.setMinimal(true);

        int pos = 0;
        while (pos > -1)
        {
            pos  = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                QString tmp = fi.baseName().toLower();
                if( tmp[0].isLetter() )
                    tmp[0] = tmp[0].toUpper();

                for( int i = 0; i < tmp.length(); ++i )
                {
                    if( tmp[i+1].isLetter() && !tmp[i].isLetter() &&
                            tmp[i] != '\'' && tmp[i] != '?' && tmp[i] != '`' )
                    {
                        tmp[i+1] = tmp[i+1].toUpper();
                    }
                }
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }

    // parse camera token
    {
        QRegExp regExp("\\[cam([$%&]*)\\]");
        regExp.setMinimal(true);

        int pos = 0;
        while (pos > -1)
        {
            pos  = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                QString tmp      = cameraName;
                QString optToken = regExp.cap(1);

                if (!optToken.isEmpty())
                {
                    if (regExp.cap(1) == QString('$'))
                        tmp = cameraName;
                    else if (regExp.cap(1) == QString('&'))
                        tmp = cameraName.toUpper();
                    else if (regExp.cap(1) == QString('%'))
                        tmp = cameraName.toLower();
                }
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }

    // parse simple / remaining tokens ------------------------
    {
        parsedString.replace('$', fi.baseName());
        parsedString.replace('&', fi.baseName().toUpper());
        parsedString.replace('%', fi.baseName().toLower());
    }

    return parsedString;
}

QString ManualRenameInput::parse(const QString& fileName, const QString& cameraName,
                                 const QDateTime &dateTime, int index) const
{
    QString parseString = d->parseStringLineEdit->text();
    return (parser(parseString, fileName, cameraName, dateTime, index));
}

QString ManualRenameInput::createToolTip()
{
    typedef QPair<QString, QString> token;
    QList<token> tokenList;

    tokenList << token(QString("$"),              i18n("filename (original)"))
              << token(QString("&"),              i18n("filename (upper case)"))
              << token(QString("%"),              i18n("filename (lower case)"))
              << token(QString("*"),              i18n("filename (first letter of each word upper case)"))
              << token(QString("#"),              i18n("sequence number"))
              << token(QString("#{start,step}"),  i18n("sequence number (custom start + step)"))
              << token(QString("[cam]"),          i18n("camera name"))
              << token(QString("[date:format]"),  i18n("date and time of the file ("
                                                       "<a href='http://doc.trolltech.com/latest/qdatetime.html#toString'>"
                                                           "format settings"
                                                       "</a>)"));

    QString tooltip;
    tooltip += QString("<p><table>");

    foreach (const token& token, tokenList)
    {
        tooltip += QString("<tr><td><b>%1</b></td><td>:</td><td>%2</td></tr>").arg(token.first)
                                                                              .arg(token.second);
    }

    tooltip += QString("</table></p>");

    tooltip += i18n("<p><table>"
                        "<tr><td><i>Example:</i></td><td></td></tr>"
                        "<tr><td></td><td><b>new_$_###</b></td></tr>"
                        "<tr><td>=></td><td>new_MyImageName_001.jpg</td></tr>"
                    "</table></p>");
    return tooltip;
}

void ManualRenameInput::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void ManualRenameInput::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}

}  // namespace Digikam
