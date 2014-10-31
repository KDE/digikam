/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "infodlg.moc"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

// Local includes

#include "daboutdata.h"

namespace Digikam
{

class InfoDlg::Private
{
public:

    Private() :
        listView(0)
    {
    }

    QTreeWidget* listView;
};

InfoDlg::InfoDlg(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    setButtons(Help|User1|Ok);
    setDefaultButton(Ok);
    setButtonFocus(Ok);
    setModal(false);
    setHelp("digikam");
    setCaption(i18n("Shared Libraries and Components Information"));
    setButtonText(User1, i18n("Copy to Clipboard"));

    QWidget* const page     = new QWidget(this);
    setMainWidget(page);
    QGridLayout* const grid = new QGridLayout(page);

    // --------------------------------------------------------

    QLabel* const logo      = new QLabel(page);

    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
    {
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                        .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                        .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // --------------------------------------------------------

    QLabel* const header    = new QLabel(page);
    header->setWordWrap(true);
    header->setText(i18n("<font size=\"5\">%1</font><br/><b>Version %2</b>"
                         "<p>%3</p>",
                         KGlobal::mainComponent().aboutData()->programName(),
                         KGlobal::mainComponent().aboutData()->version(),
                         DAboutData::digiKamSlogan().toString()));

    // --------------------------------------------------------

    d->listView = new QTreeWidget(page);
    d->listView->setSortingEnabled(false);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(2);
    d->listView->header()->setResizeMode(QHeaderView::Stretch);

    // --------------------------------------------------------

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(header,      0, 1, 1, 1);
    // row 1 can be expanded by custom widgets in the subclassed dialog
    grid->addWidget(d->listView, 2, 0, 1, -1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotCopy2ClipBoard()));

    resize(400, 500);
}

InfoDlg::~InfoDlg()
{
    delete d;
}

QTreeWidget* InfoDlg::listView() const
{
    return d->listView;
}

void InfoDlg::setInfoMap(const QMap<QString, QString>& list)
{
    for (QMap<QString, QString>::const_iterator it = list.constBegin(); it != list.constEnd() ; ++it)
    {
        new QTreeWidgetItem(d->listView, QStringList() << it.key() << it.value());
    }
}

void InfoDlg::slotCopy2ClipBoard()
{
    QString textInfo;

    textInfo.append(KGlobal::mainComponent().aboutData()->programName());
    textInfo.append(" version ");
    textInfo.append(KGlobal::mainComponent().aboutData()->version());
    textInfo.append("\n");

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        textInfo.append((*it)->text(0));
        textInfo.append(": ");
        textInfo.append((*it)->text(1));
        textInfo.append("\n");
        ++it;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

}  // namespace Digikam
