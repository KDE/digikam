/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qlabel.h>
#include <qlayout.h>

// KDE includes.

#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <klistview.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "searchtextbar.h"
#include "syncjob.h"
#include "tageditdlg.h"
#include "tageditdlg.moc"

namespace Digikam
{

class TagsListCreationErrorDialog : public KDialogBase
{

public:

    TagsListCreationErrorDialog(QWidget* parent, const QMap<QString, QString>& errMap);
    ~TagsListCreationErrorDialog(){};
};

// ------------------------------------------------------------------------------

class TagEditDlgPriv
{
public:

    TagEditDlgPriv()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        mainRootAlbum   = 0;
        topLabel        = 0;
        create          = false;
    }

    bool           create;

    QLabel        *topLabel;

    QString        icon;

    QPushButton   *iconButton;
    QPushButton   *resetIconButton;

    TAlbum        *mainRootAlbum;
    SearchTextBar *titleEdit;
};

TagEditDlg::TagEditDlg(QWidget *parent, TAlbum* album, bool create)
          : KDialogBase(parent, 0, true, 0, Help|Ok|Cancel, Ok, true)
{
    d = new TagEditDlgPriv;
    d->mainRootAlbum = album; 
    d->create        = create;

    setHelp("tagscreation.anchor", "digikam");
    if (d->create) setCaption(i18n("New Tag"));
    else        setCaption(i18n("Edit Tag"));

    QWidget *page     = makeMainWidget();
    QGridLayout* grid = new QGridLayout(page, 5, 4, 0, spacingHint());

    // --------------------------------------------------------

    QLabel *logo            = new QLabel(page);
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 96, KIcon::DefaultState, 0, true));

    d->topLabel = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine);

    KSeparator *line = new KSeparator (Horizontal, page);

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new SearchTextBar(page, "TagEditDlgTitleEdit", i18n("Enter tag name here..."));
    titleLabel->setBuddy(d->titleEdit);

    QLabel *tipLabel = new QLabel(page);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setText(i18n("<qt><p>To create new tags, you can use the following rules:</p>"
                           "<p><ul><li>'/' can be used to create a tags hierarchy.<br>"
                           "Ex.: <i>\"Country/City/Paris\"</i></li>"
                           "<li>',' can be used to create more than one tags hierarchy at the same time.<br>"
                           "Ex.: <i>\"City/Paris, Monument/Notre-Dame\"</i></li>"
                           "<li>If a tag hierarchy starts with '/', root tag album is used as parent.</li></ul></p></qt>"
                           ));

    if (d->create) 
    {
        AlbumList tList = AlbumManager::instance()->allTAlbums();
        for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
        {
            TAlbum *tag = dynamic_cast<TAlbum*>(*it);
            d->titleEdit->lineEdit()->completionObject()->addItem(tag->tagPath());
        }
    }
    else
    {
        d->titleEdit->setText(d->mainRootAlbum->title());
        tipLabel->hide();
    }

    QLabel *iconTextLabel = new QLabel(page);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    if (d->create && !d->mainRootAlbum->isRoot())
        d->icon = d->mainRootAlbum->icon();
    else
        d->icon = d->mainRootAlbum->icon();

    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));

    d->resetIconButton = new QPushButton(SmallIcon("reload_page"), i18n("Reset"), page);
    if (d->create) d->resetIconButton->hide();

    // --------------------------------------------------------

    grid->addMultiCellWidget(logo,               0, 3, 0, 0);
    grid->addMultiCellWidget(d->topLabel,        0, 0, 1, 4);
    grid->addMultiCellWidget(line,               1, 1, 1, 4);
    grid->addMultiCellWidget(tipLabel,           2, 2, 1, 4);
    grid->addMultiCellWidget(titleLabel,         3, 3, 1, 1);
    grid->addMultiCellWidget(d->titleEdit,       3, 3, 2, 4);
    grid->addMultiCellWidget(iconTextLabel,      4, 4, 1, 1);
    grid->addMultiCellWidget(d->iconButton,      4, 4, 2, 2);
    grid->addMultiCellWidget(d->resetIconButton, 4, 4, 3, 3);
    grid->setColStretch(4, 10);
    grid->setRowStretch(5, 10);

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->titleEdit->lineEdit(), SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    // --------------------------------------------------------

    slotTitleChanged(d->titleEdit->text());
    d->titleEdit->lineEdit()->setFocus();
    adjustSize();
}

TagEditDlg::~TagEditDlg()
{
    delete d;
}

QString TagEditDlg::title() const
{
    return d->titleEdit->text();
}

QString TagEditDlg::icon() const
{
    return d->icon;
}

void TagEditDlg::slotIconResetClicked()
{
    d->icon = QString("tag");
    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));
}

void TagEditDlg::slotIconChanged()
{
#if KDE_IS_VERSION(3,3,0)
    KIconDialog dlg(this);
    dlg.setup(KIcon::NoGroup, KIcon::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();
#else
    QString icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 20);
    if (icon.startsWith("/"))
        return;
#endif

    if (icon.isEmpty() || icon == d->icon)
        return;

    d->icon = icon;
    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    QString tagName = d->mainRootAlbum->tagPath();
    if (tagName.endsWith("/") && !d->mainRootAlbum->isRoot()) 
        tagName.truncate(tagName.length()-1);

    if (d->create)
    {
        if (d->titleEdit->text().startsWith("/"))
        {
            d->topLabel->setText(i18n("<qt><b>Create New Tag</b></qt>"));
        }
        else
        {
            d->topLabel->setText(i18n("<qt><b>Create New Tag in<br>"
                                      "<i>\"%1\"</i></b></qt>").arg(tagName));
        }
    }
    else
    {
        d->topLabel->setText(i18n("<qt><b>Properties of Tag<br>"
                                  "<i>\"%1\"</i></b></qt>").arg(tagName));
    }

    enableButtonOK(!newtitle.isEmpty());
}

bool TagEditDlg::tagEdit(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

bool TagEditDlg::tagCreate(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album, true);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

AlbumList TagEditDlg::createTAlbum(TAlbum *mainRootAlbum, const QString& tagStr, const QString& icon,
                                   QMap<QString, QString>& errMap)
{
    errMap.clear();
    AlbumList createdTagsList;

    // Check if new tags are include in a list of tags hierarchy separated by ','.
    // Ex: /Country/France/people,/City/France/Paris

    QStringList tagsHierarchies = QStringList::split(",", tagStr);
    if (tagsHierarchies.isEmpty()) 
        return createdTagsList;

    for (QStringList::const_iterator it = tagsHierarchies.begin(); it != tagsHierarchies.end(); ++it)
    {
        QString hierarchy = (*it).stripWhiteSpace();
        if (!hierarchy.isEmpty())
        {
            // Check if new tags is a hierarchy of tags separated by '/'.

            TAlbum *root = 0;

            if (hierarchy.startsWith("/") || !mainRootAlbum)
                root = AlbumManager::instance()->findTAlbum(0);
            else
                root = mainRootAlbum;

            QStringList tagsList = QStringList::split("/", hierarchy);
            DDebug() << tagsList << endl;

            if (!tagsList.isEmpty())
            {
                for (QStringList::iterator it2 = tagsList.begin(); it2 != tagsList.end(); ++it2)
                {
                    QString tagPath, errMsg;
                    QString tag = (*it2).stripWhiteSpace();
                    if (root->isRoot())
                        tagPath = QString("/%1").arg(tag);
                    else
                        tagPath = QString("%1/%2").arg(root->tagPath()).arg(tag);

                    DDebug() << tag << " :: " << tagPath << endl;

                    if (!tag.isEmpty())
                    {
                        // Tag already exist ?
                        TAlbum* album = AlbumManager::instance()->findTAlbum(tagPath);
                        if (!album)
                        {
                            root = AlbumManager::instance()->createTAlbum(root, tag, icon, errMsg);
                        }
                        else
                        {
                            root = album;
                            if (*it2 == tagsList.last())
                                errMap.insert(tagPath, i18n("Tag name already exists"));
                        }

                        if (root)
                            createdTagsList.append(root);
                    }

                    // Sanity check if tag creation failed.
                    if (!root)
                    {
                        errMap.insert(tagPath, errMsg);
                        break;
                    }
                }
            }
        }
    }

    return createdTagsList;
}

void TagEditDlg::showtagsListCreationError(QWidget* parent, const QMap<QString, QString>& errMap)
{
    if (!errMap.isEmpty())
    {
        TagsListCreationErrorDialog dlg(parent, errMap);
        dlg.exec();
    }
}

// ------------------------------------------------------------------------------

TagsListCreationErrorDialog::TagsListCreationErrorDialog(QWidget* parent, const QMap<QString, QString>& errMap)
                           : KDialogBase(parent, 0, true, 0, Help|Ok, Ok, false)
{
    setHelp("tagscreation.anchor", "digikam");
    setCaption(i18n("Tag creation Error"));

    QWidget* box      = makeMainWidget();
    QVBoxLayout* vLay = new QVBoxLayout(box);

    QLabel *label       = new QLabel(i18n("Error been occured during Tag creation:"), box);
    KListView *listView = new KListView(box);
    listView->addColumn(i18n("Tag Path"));
    listView->addColumn(i18n("Error"));
    listView->setResizeMode(QListView::LastColumn);

    vLay->addWidget(label);
    vLay->addWidget(listView);
    vLay->setMargin(0);
    vLay->setSpacing(0);

    for (QMap<QString, QString>::const_iterator it = errMap.begin() ; it != errMap.end() ; ++it)
        new KListViewItem(listView, it.key(), it.data());

    adjustSize();
}

}  // namespace Digikam
