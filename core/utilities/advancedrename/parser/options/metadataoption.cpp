/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide metadata information to the parser
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

#include "metadataoption.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "metadatapanel.h"
#include "metadataselector.h"

namespace Digikam
{

MetadataOptionDialog::MetadataOptionDialog(Rule* const parent) :
    RuleDialog(parent),
    metadataPanel(0),
    separatorLineEdit(0)
{
    QWidget* const mainWidget = new QWidget(this);
    QTabWidget* const tab     = new QTabWidget(this);
    metadataPanel             = new MetadataPanel(tab);
    QLabel* const customLabel = new QLabel(i18n("Keyword separator:"));
    separatorLineEdit         = new QLineEdit(this);
    separatorLineEdit->setText(QLatin1String("_"));

    // --------------------------------------------------------

    // We only need the "SearchBar" control element.
    // We also need to reset the default selections.
    foreach(MetadataSelectorView* const viewer, metadataPanel->viewers())
    {
        viewer->setControlElements(MetadataSelectorView::SearchBar);
        viewer->clearSelection();
    }

    // --------------------------------------------------------

    // remove "Viewer" string from tabs
    int tabs = tab->count();

    for (int i = 0; i < tabs; ++i)
    {
        QString text = tab->tabText(i);
        text.remove(QLatin1String("viewer"), Qt::CaseInsensitive);
        tab->setTabText(i, text.simplified());
    }

    // --------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(this);
    mainLayout->addWidget(customLabel,       0, 0, 1, 1);
    mainLayout->addWidget(separatorLineEdit, 0, 1, 1, 1);
    mainLayout->addWidget(tab,               1, 0, 1, -1);
    mainWidget->setLayout(mainLayout);

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
    resize(450, 450);
}

MetadataOptionDialog::~MetadataOptionDialog()
{
}

// --------------------------------------------------------

MetadataOption::MetadataOption()
    : Option(i18n("Metadata..."),
             i18n("Add metadata information"))
{
    QString iconName(QLatin1String("format-text-code"));
    QPixmap icon = QIcon::fromTheme(iconName).pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
    setIcon(iconName);

    // --------------------------------------------------------

    addToken(QLatin1String("[meta:||key||]"), description());

    QRegExp reg(QLatin1String("\\[meta(:(.*))\\]"));
    reg.setMinimal(true);
    setRegExp(reg);
}

void MetadataOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList tags;

    QPointer<MetadataOptionDialog> dlg = new MetadataOptionDialog(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        QStringList checkedTags = dlg->metadataPanel->getAllCheckedTags();

        foreach(const QString& tag, checkedTags)
        {
            tags << QString::fromUtf8("[meta:%1]").arg(tag);
        }
    }

    if (!tags.isEmpty())
    {
        QString tokenStr = tags.join(dlg->separatorLineEdit->text());
        emit signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

QString MetadataOption::parseOperation(ParseSettings& settings)
{
    const QRegExp& reg = regExp();
    QString keyword    = reg.cap(2);
    QString result     = parseMetadata(keyword, settings);
    return result;
}

QString MetadataOption::parseMetadata(const QString& token, ParseSettings& settings)
{
    QString result;

    if (settings.fileUrl.isEmpty())
    {
        return result;
    }

    QString keyword = token.toLower();

    if (keyword.isEmpty())
    {
        return result;
    }

    DMetadata meta(settings.fileUrl.toLocalFile());

    if (!meta.isEmpty())
    {
        MetaEngine::MetaDataMap dataMap;

        if (keyword.startsWith(QLatin1String("exif.")))
        {
            dataMap = meta.getExifTagsDataList(QStringList(), true);
        }
        else if (keyword.startsWith(QLatin1String("iptc.")))
        {
            dataMap = meta.getIptcTagsDataList(QStringList(), true);
        }
        else if (keyword.startsWith(QLatin1String("xmp.")))
        {
            dataMap = meta.getXmpTagsDataList(QStringList(), true);
        }

        foreach(const QString& key, dataMap.keys())
        {
            if (key.toLower().contains(keyword))
            {
                result = dataMap[key];
                break;
            }
        }
    }

    result.replace(QLatin1Char('/'), QLatin1Char('|'));

    return result;
}

} // namespace Digikam
