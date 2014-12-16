/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide date information to the parser
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "dateoption.moc"

// Qt includes

#include <QDateTime>
#include <QPointer>
#include <QTimer>
#include <QValidator>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "ui_dateoptiondialogwidget.h"

namespace Digikam
{

static QString getDateFormatLinkText()
{
    const QString dateFormatLink("<a href='http://qt-project.org/doc/qt-5.0/qtcore/qdatetime.html#toString'>%1</a>");
    const QString dateFormatLinkDescr = i18nc("date format settings", "format settings");

    return dateFormatLink.arg(dateFormatLinkDescr);
}

// --------------------------------------------------------

DateFormat::DateFormat()
{
    m_map.insert(Standard,      DateFormatDescriptor(QString("Standard"),       QString("yyyyMMddThhmmss")));
    m_map.insert(ISO,           DateFormatDescriptor(QString("ISO"),            Qt::ISODate));
    m_map.insert(FullText,      DateFormatDescriptor(QString("Text"),           Qt::TextDate));
#if QT_VERSION >= 0x040700
    m_map.insert(UnixTimeStamp, DateFormatDescriptor(QString("UnixTimeStamp"),  QVariant()));
#endif
    m_map.insert(Custom,        DateFormatDescriptor(QString("Custom"),         QVariant()));
}

DateFormat::Type DateFormat::type(const QString& identifier)
{
    if (identifier.isEmpty())
    {
        return Standard;
    }

    for (int i = 0; i < m_map.size(); ++i)
    {
        if (m_map.at(i).first == identifier)
        {
            return (Type)i;
        }
    }

    return Standard;
}

QString DateFormat::identifier(Type type)
{
    return m_map.at((int)type).first;
}

QVariant DateFormat::format(Type type)
{
    return m_map.at((int)type).second;
}

QVariant DateFormat::format(const QString& identifier)
{
    if (identifier.isEmpty())
    {
        return m_map.at(Standard).second;
    }

    foreach(const DateFormatDescriptor& desc, m_map)
    {
        if (desc.first == identifier)
        {
            return desc.second;
        }
    }
    return QVariant();
}

// --------------------------------------------------------

DateOptionDialog::DateOptionDialog(Rule* parent)
    : RuleDialog(parent), ui(new Ui::DateOptionDialogWidget)
{
    QWidget* mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);

    // --------------------------------------------------------

    // fill the date source combobox
    ui->dateSourcePicker->addItem(i18nc("Get date information from the image", "Image"),
                                  QVariant(FromImage));
    //    ui->dateSourcePicker->addItem(i18nc("Get date information from the current date", "Current Date"),
    //                                  QVariant(CurrentDateTime));
    ui->dateSourcePicker->addItem(i18nc("Set a fixed date", "Fixed Date"),
                                  QVariant(FixedDateTime));

    // fill the date format combobox
    DateFormat df;
    foreach(const DateFormat::DateFormatDescriptor& desc, df.map())
    {
        ui->dateFormatPicker->addItem(desc.first);
    }

    // set the datePicker and timePicker to the current local datetime
    QDateTime currentDateTime = QDateTime::currentDateTime();
    ui->datePicker->setDate(currentDateTime.date());
    ui->timePicker->setTime(currentDateTime.time());

    ui->dateFormatLink->setOpenExternalLinks(true);
    ui->dateFormatLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    ui->dateFormatLink->setText(getDateFormatLinkText());

    QRegExp validRegExp("[^/]+");
    QValidator* validator = new QRegExpValidator(validRegExp, this);
    ui->customFormatInput->setValidator(validator);
    ui->customFormatInput->setClickMessage(i18n("Enter custom format"));

    // --------------------------------------------------------

    connect(ui->dateSourcePicker, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotDateSourceChanged(int)));

    connect(ui->dateFormatPicker, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotDateFormatChanged(int)));

    connect(ui->customFormatInput, SIGNAL(textChanged(QString)),
            this, SLOT(slotCustomFormatChanged(QString)));

    // --------------------------------------------------------

    ui->dateFormatPicker->setCurrentIndex(DateFormat::Standard);
    slotDateFormatChanged(ui->dateFormatPicker->currentIndex());

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
}

DateOptionDialog::~DateOptionDialog()
{
    delete ui;
}

DateOptionDialog::DateSource DateOptionDialog::dateSource()
{
    QVariant v = ui->dateSourcePicker->itemData(ui->dateSourcePicker->currentIndex());
    bool ok    = true;
    int choice = v.toInt(&ok);

    return static_cast<DateSource>(choice);
}

QString DateOptionDialog::formattedDateTime(const QDateTime& date)
{
    switch (ui->dateFormatPicker->currentIndex())
    {
        case DateFormat::Custom:
            return date.toString(ui->customFormatInput->text());
            break;
#if QT_VERSION >= 0x040700

        case DateFormat::UnixTimeStamp:
            return QString("%1").arg(date.toMSecsSinceEpoch());
            break;
#endif

        default:
            break;
    }

    DateFormat df;
    QVariant   v;

    v = df.format(static_cast<DateFormat::Type>(ui->dateFormatPicker->currentIndex()));
    QString result;

    if (v.type() == QVariant::String)
    {
        result = date.toString(v.toString());
    }
    else
    {
        result = date.toString((Qt::DateFormat)v.toInt());
    }

    return result;
}

void DateOptionDialog::slotDateSourceChanged(int index)
{
    Q_UNUSED(index)
    ui->fixedDateContainer->setEnabled(dateSource() == FixedDateTime);
}

void DateOptionDialog::slotDateFormatChanged(int index)
{
    bool custom = (index == DateFormat::Custom);
    ui->customFormatInput->setEnabled(custom);
    ui->dateFormatLink->setEnabled(custom);
    ui->dateFormatLink->setVisible(custom);

    updateExampleLabel();
}

void DateOptionDialog::slotCustomFormatChanged(const QString&)
{
    updateExampleLabel();
}

void DateOptionDialog::updateExampleLabel()
{
    QString result = QString("example: %1").arg(formattedDateTime(QDateTime::currentDateTime()));
    ui->exampleLabel->setText(result);
}

// --------------------------------------------------------

DateOption::DateOption()
    : Option(i18n("Date && Time..."),
             i18n("Add date and time information"),
             "view-pim-calendar")
{
    addToken("[date]",            i18n("Date and time (standard format)"));
#if QT_VERSION >= 0x040700
    addToken("[date:||key||]",    i18n("Date and time (||key|| = Standard|ISO|UnixTimeStamp|Text)"));
#else
    addToken("[date:||key||]",    i18n("Date and time (||key|| = Standard|ISO|Text)"));
#endif
    addToken("[date:||format||]", i18n("Date and time") + " (" +  getDateFormatLinkText() + ')');

    QRegExp reg("\\[date(:(.*))?\\]");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString DateOption::parseOperation(ParseSettings& settings)
{
    const QRegExp& reg = regExp();

    QString token = reg.cap(2);

    // search for quoted token parameters (indicates custom formatting)
    const int MIN_TOKEN_SIZE = 2;

    if ((token.size() > MIN_TOKEN_SIZE) &&
        (token.startsWith('"') && token.endsWith('"')))
    {
        token = token.remove(0, 1);
        token.chop(1);
    }

    // check if the datetime was already set in the parseSettings objects (most likely during the camera import)
    QDateTime dateTime;

    if (!(settings.creationTime.isNull()) && (settings.creationTime.isValid()))
    {
        dateTime = settings.creationTime;
    }
    else
    {
        // lets try to re-read the file information
        ImageInfo info = ImageInfo::fromUrl(settings.fileUrl);

        if (!info.isNull())
        {
            dateTime = info.dateTime();
        }

        if (dateTime.isNull() || !dateTime.isValid())
        {
            // still no date info, use Qt file information
            QFileInfo fileInfo(settings.fileUrl.toLocalFile());
            dateTime = fileInfo.created();
        }
    }

    // do we have a valid date?
    if (dateTime.isNull())
    {
        return QString();
    }

    QString    result;
    DateFormat df;
    QVariant   v = df.format(token);

    if (v.isNull())
    {
        // we seem to use custom format settings or UnixTimeStamp here
        switch (df.type(token))
        {
#if QT_VERSION >= 0x040700

            case DateFormat::UnixTimeStamp:
                result = QString("%1").arg(dateTime.toMSecsSinceEpoch());
                break;
#endif

            default:
                result = dateTime.toString(token);
                break;
        }
    }
    else
    {
        if (v.type() == QVariant::String)
        {
            result = dateTime.toString(v.toString());
        }
        else
        {
            result = dateTime.toString((Qt::DateFormat)v.toInt());
        }
    }

    return result;
}

void DateOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QPointer<DateOptionDialog> dlg = new DateOptionDialog(this);

    QString dateString;

    if (dlg->exec() == KDialog::Accepted)
    {
        DateFormat df;
        int index = dlg->ui->dateFormatPicker->currentIndex();

        // use custom date format?
        if (dlg->dateSource() == DateOptionDialog::FixedDateTime)
        {
            QDateTime date;
            date.setDate(dlg->ui->datePicker->date());
            date.setTime(dlg->ui->timePicker->time());

            QVariant v = (index == DateFormat::Custom)
                         ? dlg->ui->customFormatInput->text()
                         : df.format((DateFormat::Type)index);

            if (v.isNull())
            {
                // we seem to use UnixTimeStamp here
                switch (index)
                {
#if QT_VERSION >= 0x040700

                    case DateFormat::UnixTimeStamp:
                        dateString = QString("%1").arg(date.toMSecsSinceEpoch());
                        break;
#endif

                    default:
                        break;
                }
            }
            else
            {
                if (v.type() == QVariant::String)
                {
                    dateString = date.toString(v.toString());
                }
                else
                {
                    dateString = date.toString((Qt::DateFormat)v.toInt());
                }
            }
        }
        // use predefined keywords for date formatting
        else
        {
            QString tokenStr = QString("[date:%1]");

            switch (index)
            {
                case DateFormat::Standard:
                    dateString = tokenStr.arg(QString(""));
                    dateString.remove(':');
                    break;

                case DateFormat::Custom:
                    dateString = tokenStr.arg(QString("\"%1\"").arg(dlg->ui->customFormatInput->text()));
                    break;

                default:
                    QString identifier = df.identifier((DateFormat::Type) index);
                    dateString         = tokenStr.arg(identifier);
                    break;
            }
        }
    }

    delete dlg;
    emit signalTokenTriggered(dateString);
}

} // namespace Digikam
