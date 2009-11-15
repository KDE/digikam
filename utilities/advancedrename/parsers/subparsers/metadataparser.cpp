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

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/version.h>

// Local includes

#include "dmetadata.h"
#include "metadatapanel.h"
#include "metadataselector.h"

namespace Digikam
{

class MetadataParserDialogPriv
{
public:

    MetadataParserDialogPriv() :
        metadataPanel(0),
        separatorLineEdit(0)
    {}

    MetadataPanel* metadataPanel;
    KLineEdit*     separatorLineEdit;
};

// --------------------------------------------------------

MetadataParserDialog::MetadataParserDialog()
                    : KDialog(0), d(new MetadataParserDialogPriv)
{
    setCaption(i18n("Add Metadata Keywords"));

    QWidget* mainWidget  = new QWidget;
    KTabWidget* tab      = new KTabWidget;
    d->metadataPanel     = new MetadataPanel(tab);
    QLabel *customLabel  = new QLabel(i18n("Keyword separator:"));
    d->separatorLineEdit = new KLineEdit;
    d->separatorLineEdit->setText("_");

    // --------------------------------------------------------

    // We only need the "SearchBar" and "ClearBtn" control elements.
    // Also we need to reset the default selections
    foreach (MetadataSelectorView* viewer, d->metadataPanel->viewers())
    {
        viewer->setControlElements(MetadataSelectorView::SearchBar |
                                   MetadataSelectorView::ClearBtn);

        viewer->clearSelection();
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
        {
            makerNotesTabIndex = i;
        }

    }

    if (makerNotesTabIndex != -1)
    {
        tab->removeTab(makerNotesTabIndex);
    }

    // --------------------------------------------------------

    QGroupBox* customGBox     = new QGroupBox(i18n("Settings"));
    QHBoxLayout *customLayout = new QHBoxLayout;
    customLayout->addWidget(customLabel);
    customLayout->addStretch(10);
    customLayout->addWidget(d->separatorLineEdit);
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
    delete d;
}

QStringList MetadataParserDialog::checkedTags() const
{
    return d->metadataPanel->getAllCheckedTags();
}

QString MetadataParserDialog::separator() const
{
    return d->separatorLineEdit->text();
}

// --------------------------------------------------------

MetadataParser::MetadataParser()
              : SubParser(i18n("Metadata..."),
                          i18n("Add metadata fields from Exif, IPTC and XMP"))
{
    // metadataedit icon can be missing if KIPI plugins are not installed, load different icon in this case
    QIcon icon = KIconLoader::global()->loadIcon("metadataedit", KIconLoader::Small, 0,
                                                 KIconLoader::DefaultState, QStringList(), 0L, true);
    if (icon.isNull())
    {
        icon = SmallIcon("editimage");
    }
    setIcon(icon);

    addTokenDescription("[meta:|keycode|]", i18n("Metadata"),
             i18n("Add metadata (use the quick access dialog for keycodes)"));

    setRegExp("\\[meta:\\s*(.*)\\s*\\s*\\]");
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

void MetadataParser::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results)
{
    QRegExp reg = regExp();
    reg.setMinimal(true);

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, reg)
    {
        QString keyword = reg.cap(1);
        tmp = parseMetadata(keyword, info);
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

QString MetadataParser::parseMetadata(const QString& token, ParseInformation& info)
{
    QString tmp;
    QString keyword = token.toLower();
    if (keyword.isEmpty())
    {
        return tmp;
    }

    DMetadata meta(info.filePath);
    if (!meta.isEmpty())
    {
        KExiv2::MetaDataMap dataMap;
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

} // namespace Digikam
