/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a date parser class
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

#include "dateparser.h"
#include "dateparser.moc"

// Qt includes

#include <QDateTime>
#include <QTimer>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "ui_dateparserdialogwidget.h"

namespace Digikam
{
namespace ManualRename
{

const QString dateFormatLink = QString("<a href='http://doc.trolltech.com/latest/qdatetime.html#toString'>"
                  "format settings"
                  "</a>");

// --------------------------------------------------------

DateFormat::DateFormat()
{
    m_map.clear();

    m_map.insert(Standard, DateFormatDescriptor(QString("Standard"), QString("yyyyMMddThhmmss")));
    m_map.insert(ISO,      DateFormatDescriptor(QString("ISO"),      Qt::ISODate));
    m_map.insert(FullText, DateFormatDescriptor(QString("Text"),     Qt::TextDate));
    m_map.insert(Locale,   DateFormatDescriptor(QString("Locale"),   Qt::SystemLocaleShortDate));
    m_map.insert(Custom,   DateFormatDescriptor(QString("Custom"),   QString("")));
}

QString DateFormat::identifier(Type type)
{
    DateFormatDescriptor desc = m_map.at((int)type);
    return desc.first;
}

QVariant DateFormat::formatType(Type type)
{
    DateFormatDescriptor desc = m_map.at((int)type);
    return desc.second;
}

QVariant DateFormat::formatType(QString identifier)
{
    QVariant v;
    foreach (const DateFormatDescriptor& desc, m_map)
    {
        if (desc.first == identifier)
        {
            v = desc.second;
            break;
        }
    }

    if (identifier.isEmpty())
        return m_map.at(Standard).second;

    return v;
}

// --------------------------------------------------------

DateParserDialog::DateParserDialog(QWidget* parent)
                : KDialog(parent), ui(new Ui::DateParserDialogWidget)
{
    ui->setupUi(mainWidget());
    setWindowTitle(i18n("Add Date && Time"));

    // fill the date format combobox
    DateFormat df;
    foreach (const DateFormat::DateFormatDescriptor& desc, df.map())
    {
        ui->dateFormatPicker->addItem(desc.first);
    }
    ui->dateFormatLink->setOpenExternalLinks(true);
    ui->dateFormatLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);
    ui->dateFormatLink->setText(dateFormatLink);

    ui->customFormatInput->setClickMessage(i18n("Enter custom date format"));
    connect(ui->dateFormatPicker, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotDateFormatChanged(int)));

    connect(ui->customFormatInput, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCustomFormatChanged(const QString&)));

    ui->dateFormatPicker->setCurrentIndex(DateFormat::Standard);
    slotDateFormatChanged(ui->dateFormatPicker->currentIndex());
}

DateParserDialog::~DateParserDialog()
{
    delete ui;
}

QString DateParserDialog::formattedDateTime(const QDateTime& date)
{
    if (ui->dateFormatPicker->currentIndex() == DateFormat::Custom)
        return date.toString(ui->customFormatInput->text());

    DateFormat df;
    QVariant v;

    v = df.formatType((DateFormat::Type)ui->dateFormatPicker->currentIndex());
    QString tmp;
    if (v.type() == QVariant::String)
    {
        tmp = date.toString(v.toString());
    }
    else
        tmp = date.toString((Qt::DateFormat)v.toInt());
    return tmp;
}

void DateParserDialog::slotDateFormatChanged(int index)
{
    ui->customFormatInput->setEnabled(index == DateFormat::Custom);

    ui->dateFormatLink->setEnabled(index == DateFormat::Custom);
    ui->dateFormatLink->setVisible(index == DateFormat::Custom);

    updateExampleLabel();
}

void DateParserDialog::slotCustomFormatChanged(const QString&)
{
    updateExampleLabel();
}

void DateParserDialog::updateExampleLabel()
{
    QString tmp = QString("example: %1").arg(formattedDateTime(QDateTime::currentDateTime()));
    ui->exampleLabel->setText(tmp);
}

// --------------------------------------------------------

DateParser::DateParser()
          : Parser(i18n("Date && Time"), SmallIcon("view-pim-calendar"))
{
    useTokenMenu(false);

    addToken("[date]", i18n("Date && Time"),
             i18n("date and time (standard format)"));

    addToken("[date:key]", i18n("Date && Time (key)"),
             i18n("date and time (key = ISO|Text|Locale"));

    addToken("[date:format]", i18n("Date && Time (custom format)"),
             i18n("date and time") + " (" +  dateFormatLink +")");
}

void DateParser::parse(QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    QRegExp regExp("\\[date(:.*)?\\]");
    regExp.setMinimal(true);
    int pos = 0;
    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            QString tmp;
            DateFormat df;

            QString token = regExp.cap(1);
            if (!token.isEmpty())
                token.remove(0, 1);

            QVariant v = df.formatType(token);
            if (v.isNull())
            {
                tmp = info.datetime.toString(token);
            }
            else
            {
                if (v.type() == QVariant::String)
                    tmp = info.datetime.toString(v.toString());
                else
                    tmp = info.datetime.toString((Qt::DateFormat)v.toInt());
            }

            QString result = markResult(tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
        }
    }
}

void DateParser::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tokenStr      = QString("[date:%1]");
    DateParserDialog* dlg = new DateParserDialog;
    QVariant v;
    DateFormat df;
    QString tmp;

    if (dlg->exec() == KDialog::Accepted)
    {
        int index = dlg->ui->dateFormatPicker->currentIndex();

        if (dlg->ui->fixedDateBtn->isChecked())
        {
            QDateTime date;
            date.setDate(dlg->ui->datePicker->date());
            date.setTime(dlg->ui->timePicker->time());

            v = (index == DateFormat::Custom) ? dlg->ui->customFormatInput->text()
                                              : df.formatType((DateFormat::Type)index);

            if (v.type() == QVariant::String)
                tmp = date.toString(v.toString());
            else
                tmp = date.toString((Qt::DateFormat)v.toInt());
        }
        else
        {
            QString identifier = df.identifier((DateFormat::Type)index);
            if (index == DateFormat::Custom)
                tmp = tokenStr.arg(dlg->ui->customFormatInput->text());
            else
            {
                if (identifier.isEmpty())
                    tmp.remove(':');
                tmp = tokenStr.arg(identifier);
            }
        }
    }

    delete dlg;
    emit signalTokenTriggered(tmp);
}

} // namespace ManualRename
} // namespace Digikam
