/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "infodlg.h"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "daboutdata.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class InfoDlg::Private
{
public:

    Private() :
        listView(0),
        page(0)
    {
    }

    QTreeWidget* listView;
    QWidget*     page;
};

InfoDlg::InfoDlg(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setModal(false);
    setWindowTitle(i18n("Shared Libraries and Components Information"));

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Apply | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Apply)->setText(i18n("Copy to Clipboard"));

    d->page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(d->page);

    // --------------------------------------------------------

    QLabel* const logo      = new QLabel(d->page);

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));
    }
    else
    {
        logo->setPixmap(QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48,48)));
    }

    // --------------------------------------------------------

    QLabel* const header    = new QLabel(d->page);
    header->setWordWrap(true);
    header->setText(i18n("<font size=\"5\">%1</font><br/><b>Version %2</b>"
                         "<p>%3</p>",
                         QApplication::applicationName(),
                         QApplication::applicationVersion(),
                         DAboutData::digiKamSlogan()));

    // --------------------------------------------------------

    d->listView = new QTreeWidget(d->page);
    d->listView->setSortingEnabled(false);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(2);
    d->listView->header()->setSectionResizeMode(QHeaderView::Stretch);

    // --------------------------------------------------------

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(header,      0, 1, 1, 1);
    // row 1 can be expanded by custom widgets in the subclassed dialog
    grid->addWidget(d->listView, 2, 0, 1, -1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(buttons);
    setLayout(vbx);

    // --------------------------------------------------------

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotCopy2ClipBoard()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

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

QWidget* InfoDlg::mainWidget() const
{
    return d->page;
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

    textInfo.append(QApplication::applicationName());
    textInfo.append(QLatin1String(" version "));
    textInfo.append(QApplication::applicationVersion());
    textInfo.append(QLatin1String("\n"));

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        textInfo.append((*it)->text(0));
        textInfo.append(QLatin1String(": "));
        textInfo.append((*it)->text(1));
        textInfo.append(QLatin1String("\n"));
        ++it;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void InfoDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
