/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide metadata information to the parser
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

#include "metadataoption.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>

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

MetadataOptionDialog::MetadataOptionDialog(ParseObject* parent)
                    : ParseObjectDialog(parent),
                      metadataPanel(0), separatorLineEdit(0)
{
    QWidget* mainWidget  = new QWidget(this);
    KTabWidget* tab      = new KTabWidget(this);
    metadataPanel        = new MetadataPanel(tab);
    QLabel *customLabel  = new QLabel(i18n("Keyword separator:"));
    separatorLineEdit    = new KLineEdit(this);
    separatorLineEdit->setText("_");

    // --------------------------------------------------------

    // We only need the "SearchBar" and "ClearBtn" control elements.
    // Also we need to reset the default selections
    foreach (MetadataSelectorView* viewer, metadataPanel->viewers())
    {
        viewer->setControlElements(MetadataSelectorView::SearchBar |
                                   MetadataSelectorView::ClearBtn);

        viewer->clearSelection();
    }

    // --------------------------------------------------------

    // remove "Viewer" string from tabs
    for (int i = 0; i < tab->count(); ++i)
    {
        QString text = tab->tabText(i);
        text.remove("viewer", Qt::CaseInsensitive);
        tab->setTabText(i, text.simplified());
    }

    // --------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(customLabel,       0, 0, 1, 1);
    mainLayout->addWidget(separatorLineEdit, 0, 1, 1, 1);
    mainLayout->addWidget(tab,               1, 0, 1,-1);
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
              : Option(i18n("Metadata..."), i18n("Add metadata information from Exif, IPTC and XMP"))
{
    // metadataedit icon can be missing if KIPI plugins are not installed, load different icon in this case
    QPixmap icon = KIconLoader::global()->loadIcon("metadataedit", KIconLoader::Small, 0,
                                                   KIconLoader::DefaultState, QStringList(), 0L, true);
    if (icon.isNull())
    {
        icon = SmallIcon("editimage");
    }
    setIcon(icon);

    addToken("[meta:|key|]", i18n("Metadata"), i18n("Add metadata information"));

    QRegExp reg("\\[meta(:(.*))\\]");
    reg.setMinimal(true);
    setRegExp(reg);
}

void MetadataOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList tags;

    QPointer<MetadataOptionDialog> dlg = new MetadataOptionDialog(this);

    if (dlg->exec() == KDialog::Accepted)
    {
        QStringList checkedTags = dlg->metadataPanel->getAllCheckedTags();

        foreach (const QString& tag, checkedTags)
        {
            QString tagStr = QString("[meta:%1]").arg(tag);
            tags << tagStr;
        }
    }

    if (!tags.isEmpty())
    {
        QString tokenStr = tags.join(dlg->separatorLineEdit->text());
        emit signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

void MetadataOption::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results)
{
    QRegExp reg = regExp();

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, reg)
    {
        QString keyword = reg.cap(2);
        tmp = parseMetadata(keyword, info);
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

QString MetadataOption::parseMetadata(const QString& token, ParseInformation& info)
{
    QString tmp;
    QString keyword = token.toLower();
    if (keyword.isEmpty())
    {
        return tmp;
    }

    DMetadata meta(info.fileUrl.toLocalFile());
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
