/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005      by Tom Albers <tomalbers@kde.nl>
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

#include "albumpropsedit.moc"

// Qt includes

#include <QCheckBox>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRegExp>
#include <QTextEdit>
#include <QValidator>

// KDE includes

#include <kcombobox.h>
#include <kcursor.h>
#include <kdatepicker.h>
#include <kdeversion.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktextedit.h>
#include <kurl.h>
#include <kseparator.h>

// Local includes

#include "albumdb.h"
#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "databaseaccess.h"

namespace Digikam
{

class DDatePicker : public KDatePicker
{

public:

    explicit DDatePicker(QWidget* const widget)
        : KDatePicker(widget)
    {
    }

    ~DDatePicker()
    {
    }

    void dateLineEnterPressed()
    {
        lineEnterPressed();
    }
};

// --------------------------------------------------------------------------------

class AlbumPropsEdit::Private
{

public:

    Private() :
        categoryCombo(0),
        parentCombo(0),
        titleEdit(0),
        commentsEdit(0),
        datePicker(0),
        album(0)
    {
    }

    KComboBox*   categoryCombo;
    KComboBox*   parentCombo;
    KLineEdit*   titleEdit;
    KTextEdit*   commentsEdit;

    DDatePicker* datePicker;

    PAlbum*      album;
};

AlbumPropsEdit::AlbumPropsEdit(PAlbum* const album, bool create)
    : KDialog(0), d(new Private)
{
    setCaption(create ? i18n("New Album") : i18n("Edit Album"));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("albumpropsedit.anchor", "digikam");

    d->album            = album;
    QWidget* const page = new QWidget(this);
    QLabel* const logo  = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const topLabel = new QLabel(page);

    if (create)
    {
        topLabel->setText(i18n("<qt><b>Create new Album in<br/>\"%1\"</b></qt>", album->title()));
    }
    else
    {
        topLabel->setText(i18n("<qt><b>\"%1\"<br/>Album Properties</b></qt>", album->title()));
    }

    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLabel->setWordWrap(false);

    KSeparator* const topLine = new KSeparator(Qt::Horizontal);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(page);
    d->titleEdit->setClearButtonShown(true);
    titleLabel->setBuddy(d->titleEdit);

    QRegExp titleRx("[^/]+");
    QValidator* const titleValidator = new QRegExpValidator(titleRx, this);
    d->titleEdit->setValidator(titleValidator);

    QLabel* const categoryLabel = new QLabel(page);
    categoryLabel->setText(i18n("Ca&tegory:"));

    d->categoryCombo = new KComboBox(page);
    d->categoryCombo->setEditable(true);
    categoryLabel->setBuddy(d->categoryCombo);

    QLabel* const parentLabel = new QLabel(page);
    parentLabel->setText(i18n("Ch&ild Of:"));

    d->parentCombo = new KComboBox(page);
    parentLabel->setBuddy(d->parentCombo);

    QLabel* const commentsLabel = new QLabel(page);
    commentsLabel->setText(i18n("Ca&ption:"));

    d->commentsEdit = new KTextEdit(page);
    commentsLabel->setBuddy(d->commentsEdit);
    d->commentsEdit->setCheckSpellingEnabled(true);
    d->commentsEdit->setWordWrapMode(QTextOption::WordWrap);

    QLabel* const dateLabel = new QLabel(page);
    dateLabel->setText(i18n("Album &date:"));

    d->datePicker = new DDatePicker(page);
    dateLabel->setBuddy(d->datePicker);

    KHBox* const buttonRow            = new KHBox(page);
    QPushButton* const dateLowButton  = new QPushButton(i18nc("Selects the date of the oldest image",
                                                        "&Oldest"), buttonRow);
    QPushButton* const dateAvgButton  = new QPushButton(i18nc("Calculates the average date",
                                                        "&Average"), buttonRow);
    QPushButton* const dateHighButton = new QPushButton(i18nc("Selects the date of the newest image",
                                                        "Newest"), buttonRow);

    if (create)
    {
        setTabOrder(d->titleEdit, d->categoryCombo);
        setTabOrder(d->categoryCombo, d->parentCombo);
        setTabOrder(d->parentCombo, d->commentsEdit);
        setTabOrder(d->commentsEdit, d->datePicker);
    }
    else
    {
        setTabOrder(d->titleEdit, d->categoryCombo);
        setTabOrder(d->categoryCombo, d->commentsEdit);
        setTabOrder(d->commentsEdit, d->datePicker);
        d->parentCombo->hide();
        parentLabel->hide();
    }

    d->commentsEdit->setTabChangesFocus(true);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
    grid->addWidget(logo,             0, 0, 1, 1);
    grid->addWidget(topLabel,         0, 1, 1, 1);
    grid->addWidget(topLine,          1, 0, 1, 2);
    grid->addWidget(titleLabel,       2, 0, 1, 1);
    grid->addWidget(d->titleEdit,     2, 1, 1, 1);
    grid->addWidget(categoryLabel,    3, 0, 1, 1);
    grid->addWidget(d->categoryCombo, 3, 1, 1, 1);

    if (create)
    {
        grid->addWidget(parentLabel,      4, 0, 1, 1);
        grid->addWidget(d->parentCombo,   4, 1, 1, 1);
        grid->addWidget(commentsLabel,    5, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->commentsEdit,  5, 1, 1, 1);
        grid->addWidget(dateLabel,        6, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->datePicker,    6, 1, 1, 1);
        grid->addWidget(buttonRow,        7, 1, 1, 1);
    }
    else
    {
        grid->addWidget(commentsLabel,    4, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->commentsEdit,  4, 1, 1, 1);
        grid->addWidget(dateLabel,        5, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->datePicker,    5, 1, 1, 1);
        grid->addWidget(buttonRow,        6, 1, 1, 1);
    }

    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    page->setLayout(grid);

    // Initialize ---------------------------------------------

    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        d->categoryCombo->addItem(QString());
        QStringList Categories = settings->getAlbumCategoryNames();
        d->categoryCombo->addItems(Categories);
        int categoryIndex      = Categories.indexOf(album->category());

        if (categoryIndex != -1)
        {
            // + 1 because of the empty item
            d->categoryCombo->setCurrentIndex(categoryIndex + 1);
        }
    }

    if (create)
    {
        d->titleEdit->setText(i18n("New Album"));
        d->datePicker->setDate(QDate::currentDate());
        d->parentCombo->addItem(i18n("Selected Album (Default)"));
        d->parentCombo->addItem(i18nc("top level folder of album","Root of current collection"));
    }
    else
    {
        d->titleEdit->setText(album->title());
        d->commentsEdit->setText(album->caption());
        d->datePicker->setDate(album->date());
    }
    
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    connect(dateLowButton, SIGNAL(clicked()),
            this, SLOT(slotDateLowButtonClicked()));

    connect(dateAvgButton, SIGNAL(clicked()),
            this, SLOT(slotDateAverageButtonClicked()));

    connect(dateHighButton, SIGNAL(clicked()),
            this, SLOT(slotDateHighButtonClicked()));

    // --------------------------------------------------------

    setMainWidget(page);
}

AlbumPropsEdit::~AlbumPropsEdit()
{
    delete d;
}

QString AlbumPropsEdit::title() const
{
    return d->titleEdit->text();
}

QString AlbumPropsEdit::comments() const
{
    return d->commentsEdit->document()->toPlainText();
}

QDate AlbumPropsEdit::date() const
{
    // See bug #267944 : update calendar view if user enter a date in text field.
    d->datePicker->dateLineEnterPressed();

    return d->datePicker->date();
}

int AlbumPropsEdit::parent() const
{
    return d->parentCombo->currentIndex();
}

QString AlbumPropsEdit::category() const
{
    QString name = d->categoryCombo->currentText();

    if (name.isEmpty())
    {
        name = i18n("Uncategorized Album");
    }

    return name;
}

QStringList AlbumPropsEdit::albumCategories() const
{
    QStringList Categories;
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        Categories = settings->getAlbumCategoryNames();
    }

    QString currentCategory = d->categoryCombo->currentText();

    if (Categories.indexOf(currentCategory) == -1)
    {
        Categories.append(currentCategory);
    }

    Categories.sort();

    return Categories;
}

bool AlbumPropsEdit::editProps(PAlbum* const album, QString& title,
                               QString& comments, QDate& date, QString& category,
                               QStringList& albumCategories)
{
    QPointer<AlbumPropsEdit> dlg = new AlbumPropsEdit(album);

    bool ok = (dlg->exec() == QDialog::Accepted);

    title           = dlg->title();
    comments        = dlg->comments();
    date            = dlg->date();
    category        = dlg->category();
    albumCategories = dlg->albumCategories();

    delete dlg;
    return ok;
}

bool AlbumPropsEdit::createNew(PAlbum* const parent, QString& title, QString& comments,
                               QDate& date, QString& category, QStringList& albumCategories,
                               int& parentSelector)
{
    QPointer<AlbumPropsEdit> dlg = new AlbumPropsEdit(parent, true);

    bool ok = (dlg->exec() == QDialog::Accepted);

    title           = dlg->title();
    comments        = dlg->comments();
    date            = dlg->date();
    category        = dlg->category();
    albumCategories = dlg->albumCategories();
    parentSelector  = dlg->parent();

    delete dlg;
    return ok;
}

void AlbumPropsEdit::slotTitleChanged(const QString& newtitle)
{
    QRegExp emptyTitle = QRegExp("^\\s*$");
    bool enable        = (!emptyTitle.exactMatch(newtitle) && !newtitle.isEmpty());
    enableButtonOk(enable);
}

void AlbumPropsEdit::slotDateLowButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate lowDate = DatabaseAccess().db()->getAlbumLowestDate(d->album->id());

    if (lowDate.isValid())
    {
        d->datePicker->setDate(lowDate);
    }

    setCursor(Qt::ArrowCursor);
}

void AlbumPropsEdit::slotDateHighButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate highDate = DatabaseAccess().db()->getAlbumHighestDate(d->album->id());

    if (highDate.isValid())
    {
        d->datePicker->setDate(highDate);
    }

    setCursor(Qt::ArrowCursor);
}

void AlbumPropsEdit::slotDateAverageButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate avDate = DatabaseAccess().db()->getAlbumAverageDate(d->album->id());

    setCursor(Qt::ArrowCursor);

    if (avDate.isValid())
    {
        d->datePicker->setDate(avDate);
    }
    else
    {
        KMessageBox::error(this,
                           i18n("Could not calculate an average."),
                           i18n("Could Not Calculate Average"));
    }
}

}  // namespace Digikam
