/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tageditdlg.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QTreeWidget>

// KDE includes

#include <klineedit.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kkeysequencewidget.h>

// Local includes

#include "album.h"
#include "syncjob.h"
#include "searchtextbar.h"
#include "tagsactionmngr.h"
#include "databaseconstants.h"

namespace Digikam
{

class TagsListCreationErrorDialog : public KDialog
{

public:

    TagsListCreationErrorDialog(QWidget* const parent, const QMap<QString, QString>& errMap);
    ~TagsListCreationErrorDialog() {};
};

// ------------------------------------------------------------------------------

class TagEditDlg::Private
{
public:

    Private()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        mainRootAlbum   = 0;
        topLabel        = 0;
        keySeqWidget    = 0;
        create          = false;
    }

    bool                create;

    QLabel*             topLabel;

    QString             icon;

    QPushButton*        iconButton;
    QPushButton*        resetIconButton;

    KKeySequenceWidget* keySeqWidget;

    TAlbum*             mainRootAlbum;
    SearchTextBar*      titleEdit;
};

TagEditDlg::TagEditDlg(QWidget* const parent, TAlbum* const album, bool create)
    : KDialog(parent), d(new Private)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");

    if (create)
    {
        setCaption(i18n("New Tag"));
    }
    else
    {
        setCaption(i18n("Edit Tag"));
    }

    d->mainRootAlbum    = album;
    d->create           = create;
    QWidget* const page = new QWidget(this);
    setMainWidget(page);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel             = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator* const line  = new KSeparator(Qt::Horizontal, page);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit             = new SearchTextBar(page, "TagEditDlgTitleEdit", i18n("Enter tag name here..."));
    d->titleEdit->setCaseSensitive(false);
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const tipLabel   = new QLabel(page);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("<p>To create new tags, you can use the following rules:</p>"
                           "<p><ul><li>'/' can be used to create a tags hierarchy.<br/>"
                           "Ex.: <i>\"Country/City/Paris\"</i></li>"
                           "<li>',' can be used to create more than one tags hierarchy at the same time.<br/>"
                           "Ex.: <i>\"City/Paris, Monument/Notre-Dame\"</i></li>"
                           "<li>If a tag hierarchy starts with '/', root tag album is used as parent.</li></ul></p>"
                          ));

    if (d->create)
    {
        AlbumList tList = AlbumManager::instance()->allTAlbums();

        for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
        {
            TAlbum* const tag = static_cast<TAlbum*>(*it);
            d->titleEdit->completionObject()->addItem(tag->tagPath());
        }
    }
    else
    {
        d->titleEdit->setText(d->mainRootAlbum->title());
        tipLabel->hide();
    }

    // --------------------------------------------------------

    QLabel* const iconTextLabel = new QLabel(page);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton         = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    d->icon = album->icon();

    d->iconButton->setIcon(SyncJob::getTagThumbnail(album));

    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), page);

    if (create)
    {
        d->resetIconButton->hide();
    }

    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(page);
    kscTextLabel->setText(i18n("&Shortcut:"));

    d->keySeqWidget = new KKeySequenceWidget(page);
    kscTextLabel->setBuddy(d->keySeqWidget);

    if (!create)
    {
        QString Seq = album->property(TagPropertyName::tagKeyboardShortcut());
        d->keySeqWidget->setKeySequence(Seq);
    }
    else
    {
        // Do not inherit tag shortcut, it creates a conflict shortcut, see bug 309558.
        d->keySeqWidget->setCheckActionCollections(TagsActionMngr::defaultManager()->actionCollections());
    }

    QLabel* const tipLabel2 = new QLabel(page);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: this shortcut can be used to assign or unassign tag to items.</p>"));

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 1, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 0, 1, 4);
    grid->addWidget(tipLabel,           2, 0, 1, 4);
    grid->addWidget(titleLabel,         3, 0, 1, 1);
    grid->addWidget(d->titleEdit,       3, 1, 1, 3);
    grid->addWidget(iconTextLabel,      4, 0, 1, 1);
    grid->addWidget(d->iconButton,      4, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 2, 1, 1);
    grid->addWidget(kscTextLabel,       5, 0, 1, 1);
    grid->addWidget(d->keySeqWidget,    5, 1, 1, 3);
    grid->addWidget(tipLabel2,          6, 0, 1, 4);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->titleEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    // --------------------------------------------------------

    slotTitleChanged(d->titleEdit->text());
    d->titleEdit->setFocus();
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

QKeySequence TagEditDlg::shortcut() const
{
    return d->keySeqWidget->keySequence();
}

void TagEditDlg::slotIconResetClicked()
{
    d->icon = QString("tag");
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotIconChanged()
{
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();

    if (icon.isEmpty() || icon == d->icon)
    {
        return;
    }

    d->icon = icon;
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    QString tagName = d->mainRootAlbum->tagPath();

    if (tagName.endsWith('/') && !d->mainRootAlbum->isRoot())
    {
        tagName.truncate(tagName.length()-1);
    }

    if (d->create)
    {
        if (d->titleEdit->text().startsWith('/'))
        {
            d->topLabel->setText(i18n("<b>Create New Tag</b>"));
        }
        else
        {
            d->topLabel->setText(i18n("<b>Create New Tag in<br/>"
                                      "\"%1\"</b>", tagName));
        }
    }
    else
    {
        d->topLabel->setText(i18n("<b>Properties of Tag<br/>"
                                  "\"%1\"</b>", tagName));
    }

    QRegExp emptyTitle = QRegExp("^\\s*$");
    bool enable        = (!emptyTitle.exactMatch(newtitle) && !newtitle.isEmpty());
    enableButtonOk(enable);
}

bool TagEditDlg::tagEdit(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks)
{
    QPointer<TagEditDlg> dlg = new TagEditDlg(parent, album);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        title = dlg->title();
        icon  = dlg->icon();
        ks    = dlg->shortcut();
    }

    delete dlg;
    return valRet;
}

bool TagEditDlg::tagCreate(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks)
{
    QPointer<TagEditDlg> dlg = new TagEditDlg(parent, album, true);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        title = dlg->title();
        icon  = dlg->icon();
        ks    = dlg->shortcut();
    }

    delete dlg;
    return valRet;
}

AlbumList TagEditDlg::createTAlbum(TAlbum* const mainRootAlbum, const QString& tagStr, const QString& icon,
                                   const QKeySequence& ks, QMap<QString, QString>& errMap)
{
    errMap.clear();
    AlbumList createdTagsList;
    TAlbum* root = 0;

    // Check if new tags are include in a list of tags hierarchy separated by ','.
    // Ex: /Country/France/people,/City/France/Paris

    const QStringList tagsHierarchies = tagStr.split(',', QString::SkipEmptyParts);

    if (tagsHierarchies.isEmpty())
    {
        return createdTagsList;
    }

    for (QStringList::const_iterator it = tagsHierarchies.constBegin();
         it != tagsHierarchies.constEnd(); ++it)
    {
        QString hierarchy = (*it).trimmed();

        if (!hierarchy.isEmpty())
        {
            // Check if new tags is a hierarchy of tags separated by '/'.

            root = 0;

            if (hierarchy.startsWith('/') || !mainRootAlbum)
            {
                root = AlbumManager::instance()->findTAlbum(0);
            }
            else
            {
                root = mainRootAlbum;
            }

            QStringList tagsList = hierarchy.split('/', QString::SkipEmptyParts);
            kDebug() << tagsList;

            if (!tagsList.isEmpty())
            {
                for (QStringList::const_iterator it2 = tagsList.constBegin();
                     it2 != tagsList.constEnd(); ++it2)
                {
                    QString tagPath, errMsg;
                    QString tag = (*it2).trimmed();

                    if (root->isRoot())
                    {
                        tagPath = QString("/%1").arg(tag);
                    }
                    else
                    {
                        tagPath = QString("%1/%2").arg(root->tagPath()).arg(tag);
                    }

                    kDebug() << tag << " :: " << tagPath;

                    if (!tag.isEmpty())
                    {
                        // Tag already exist ?
                        TAlbum* const album = AlbumManager::instance()->findTAlbum(tagPath);

                        if (!album)
                        {
                            root = AlbumManager::instance()->createTAlbum(root, tag, icon, errMsg);
                        }
                        else
                        {
                            root = album;

                            if (*it2 == tagsList.last())
                            {
                                errMap.insert(tagPath, i18n("Tag name already exists"));
                            }
                        }

                        if (root)
                        {
                            createdTagsList.append(root);
                        }
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

    // Assign the keyboard shortcut to the last tag created from the hierarchy.
    if (root && !ks.isEmpty())
    {
        TagsActionMngr::defaultManager()->updateTagShortcut(root->id(), ks);
    }

    return createdTagsList;
}

void TagEditDlg::showtagsListCreationError(QWidget* const parent, const QMap<QString, QString>& errMap)
{
    if (!errMap.isEmpty())
    {
        QPointer<TagsListCreationErrorDialog> dlg = new TagsListCreationErrorDialog(parent, errMap);
        dlg->exec();
        delete dlg;
    }
}

// ------------------------------------------------------------------------------

TagsListCreationErrorDialog::TagsListCreationErrorDialog(QWidget* const parent, const QMap<QString, QString>& errMap)
    : KDialog(parent)
{
    setButtons(Help|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");
    setCaption(i18n("Tag creation Error"));

    QWidget* const box          = new QWidget(this);
    setMainWidget(box);
    QVBoxLayout* const vLay     = new QVBoxLayout(box);

    QLabel* const label         = new QLabel(i18n("An error occurred during tag creation:"), box);
    QTreeWidget* const listView = new QTreeWidget(box);
    listView->setHeaderLabels(QStringList() << i18n("Tag Path") << i18n("Error"));
    listView->setRootIsDecorated(false);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    vLay->addWidget(label);
    vLay->addWidget(listView);
    vLay->setMargin(KDialog::marginHint());
    vLay->setSpacing(KDialog::spacingHint());

    for (QMap<QString, QString>::const_iterator it = errMap.constBegin(); it != errMap.constEnd(); ++it)
    {
        new QTreeWidgetItem(listView, QStringList() << it.key() << it.value());
    }

    adjustSize();
}

}  // namespace Digikam
