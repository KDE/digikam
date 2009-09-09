/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a metadata parser class
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

#include "metadataparser.h"
#include "metadataparser.moc"

// Qt includes

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktabwidget.h>

// LibKExiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

#include "dmetadata.h"
#include "metadatapanel.h"
#include "metadataselector.h"

namespace Digikam
{
namespace ManualRename
{

MetadataParserDialog::MetadataParserDialog()
                    : KDialog(0)
{
    setCaption(i18n("Add Metadata Keywords"));

    QWidget* mainWidget = new QWidget;
    KTabWidget* tab     = new KTabWidget;
    m_metadataPanel     = new MetadataPanel(tab);
    QLabel *customLabel = new QLabel(i18n("Keyword separator:"));
    m_separatorLineEdit = new KLineEdit;
    m_separatorLineEdit->setText("_");

    // --------------------------------------------------------

    // we only need the "SearchBar" and "ClearBtn" control elements
    foreach (MetadataSelectorView* viewer, m_metadataPanel->viewers())
    {
        viewer->setControlElements(MetadataSelectorView::SearchBar |
                                   MetadataSelectorView::ClearBtn);
    }

    // --------------------------------------------------------

    // remove "Viewer" string from tabs, remove "Makernotes" tab completely for now
    int makerNotesTabIndex = -1;
    for (int i = 0; i < tab->count(); ++i)
    {
        QString text = tab->tabText(i);
        text.remove("viewer", Qt::CaseInsensitive);
        tab->setTabText(i, text.simplified());

        if (text.toLower().contains("makernotes"))
            makerNotesTabIndex = i;
    }
    if (makerNotesTabIndex != -1)
        tab->removeTab(makerNotesTabIndex);

    // --------------------------------------------------------

    QGroupBox* customGBox     = new QGroupBox(i18n("Settings"));
    QHBoxLayout *customLayout = new QHBoxLayout;
    customLayout->addWidget(customLabel);
    customLayout->addStretch(10);
    customLayout->addWidget(m_separatorLineEdit);
    customGBox->setLayout(customLayout);

    // --------------------------------------------------------

    QGroupBox* keywordsGBox    = new QGroupBox(i18n("Metadata Keywords"));
    QVBoxLayout* keywordLayout = new QVBoxLayout;
    keywordLayout->addWidget(tab);
    keywordsGBox->setLayout(keywordLayout);

    // --------------------------------------------------------

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(customGBox);
    mainLayout->addWidget(keywordsGBox);
    mainWidget->setLayout(mainLayout);

    // --------------------------------------------------------

    setMainWidget(mainWidget);
    resize(450, 450);
}

MetadataParserDialog::~MetadataParserDialog()
{
}

QStringList MetadataParserDialog::checkedTags() const
{
    return m_metadataPanel->getAllCheckedTags();
}

QString MetadataParserDialog::separator() const
{
    return m_separatorLineEdit->text();
}

// --------------------------------------------------------

MetadataParser::MetadataParser()
              : Parser(i18n("Metadata..."), SmallIcon("metadataedit"))
{
    addToken("[meta:keycode]", i18n("Metadata"),
             i18n("add metadata (use the quick access dialog for keycodes)"));
}

void MetadataParser::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList tags;

    QPointer<MetadataParserDialog> dlg = new MetadataParserDialog;

    if (dlg->exec() == KDialog::Accepted)
    {
        QStringList checkedTags = dlg->checkedTags();

        foreach (const QString& tag, checkedTags)
        {
            QString tagStr = QString("[meta:%1]").arg(tag);
            tags << tagStr;
        }
    }

    if (!tags.isEmpty())
    {
        QString tokenStr = tags.join(dlg->separator());
        emit signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

void MetadataParser::parseOperation(QString& parseString, const ParseInformation& info)
{
    QRegExp regExp("\\[meta:\\s*(.*)\\s*\\s*\\]");
    regExp.setMinimal(true);
    int pos = 0;
    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            QString keyword = regExp.cap(1);
            QString tmp;

#if KEXIV2_VERSION >= 0x010000
            tmp = parseMetadata(keyword, info);
#else
            Q_UNUSED(info)
#endif
            QString result = markResult(regExp.matchedLength(), tmp);
            parseString.replace(pos, regExp.matchedLength(), result);
        }
    }
}

QString MetadataParser::parseMetadata(const QString& token, const ParseInformation& info)
{
    QString tmp;
    QString keyword = token.toLower();
    if (keyword.isEmpty())
        return tmp;

    DMetadata meta(info.filePath);
    if (!meta.isEmpty())
    {
        KExiv2::MetaDataMap dataMap;
        if (keyword.startsWith(QLatin1String("exif.")))
            dataMap = meta.getExifTagsDataList(QStringList(), true);
        else if (keyword.startsWith(QLatin1String("iptc.")))
            dataMap = meta.getIptcTagsDataList(QStringList(), true);
        else if (keyword.startsWith(QLatin1String("xmp.")))
            dataMap = meta.getXmpTagsDataList(QStringList(), true);

        foreach (const QString& key, dataMap.keys())
        {
            if (key.toLower().contains(keyword))
            {
                tmp = dataMap[key];
                break;
            }
        }
    }
    return tmp;
}

} // namespace ManualRename
} // namespace Digikam
