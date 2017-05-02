/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "wizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

// Local includes

#include "dwizardpage.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "albumselecttabs.h"
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "theme.h"
#include "ui_imagesettingspage.h"
#include "ui_outputpage.h"
#include "ui_themepage.h"
#include "ui_themeparameterspage.h"

namespace Digikam
{

class ThemeListBoxItem : public QListWidgetItem
{
public:

    ThemeListBoxItem(QListWidget* const list, Theme::Ptr theme)
        : QListWidgetItem(theme->name(), list),
          mTheme(theme)
    {
    }

    Theme::Ptr mTheme;
};

// ----------------------------------------------------------------------------------------------

template <class Ui_Class>

class WizardPage : public DWizardPage, public Ui_Class
{
public:

    WizardPage(QWizard* const dialog, const QString& title)
        : DWizardPage(dialog, title)
    {
        this->setupUi(this);
        layout()->setContentsMargins(QMargins());
        setPageWidget(this);
    }
};

// ----------------------------------------------------------------------------------------------

typedef WizardPage<Ui_ThemePage>           ThemePage;
typedef WizardPage<Ui_ThemeParametersPage> ThemeParametersPage;
typedef WizardPage<Ui_OutputPage>          OutputPage;

class ImageSettingsPage : public WizardPage<Ui_ImageSettingsPage>
{
public:

    ImageSettingsPage(QWizard* const dialog, const QString& title)
        : WizardPage<Ui_ImageSettingsPage>(dialog, title)
    {
        InvisibleButtonGroup* const group = new InvisibleButtonGroup(this);
        group->setObjectName(QLatin1String("kcfg_useOriginalImageAsFullImage"));
        group->addButton(mSaveImageButton, int(false));
        group->addButton(mUseOriginalImageButton, int(true));
    }
};

// ----------------------------------------------------------------------------------------------

class Wizard::Private
{
public:

    GalleryInfo*                    mInfo;
    KConfigDialogManager*           mConfigManager;

    AlbumSelectTabs*                mCollectionSelector;
    QWizardPage*                    mCollectionSelectorPage;
    ThemePage*                      mThemePage;
    ThemeParametersPage*            mThemeParametersPage;
    ImageSettingsPage*              mImageSettingsPage;
    OutputPage*                     mOutputPage;

    QMap<QByteArray, QWidget*>      mThemeParameterWidgetFromName;

public:

    void initThemePage()
    {
        QListWidget* const listWidget  = mThemePage->mThemeList;
        Theme::List list               = Theme::getList();
        Theme::List::ConstIterator it  = list.constBegin();
        Theme::List::ConstIterator end = list.constEnd();

        for (; it != end ; ++it)
        {
            Theme::Ptr theme             = *it;
            ThemeListBoxItem* const item = new ThemeListBoxItem(listWidget, theme);

            if (theme->internalName() == mInfo->theme())
            {
                listWidget->setCurrentItem(item);
            }
        }
    }

    void fillThemeParametersPage(Theme::Ptr theme)
    {
        // Create a new content page
        delete mThemeParametersPage->content;
        QWidget* const content        = new QWidget;
        mThemeParametersPage->content = content;
        mThemeParametersPage->scrollArea->setWidget(mThemeParametersPage->content);
        mThemeParameterWidgetFromName.clear();

        // Create layout. We need to recreate it every time, to get rid of
        // spacers
        QGridLayout* const layout     = new QGridLayout(content);
        layout->setContentsMargins(QMargins());
        layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

        // Create widgets
        Theme::ParameterList parameterList      = theme->parameterList();
        QString themeInternalName               = theme->internalName();
        Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
        Theme::ParameterList::ConstIterator end = parameterList.constEnd();

        for (; it != end ; ++it)
        {
            AbstractThemeParameter* const themeParameter = *it;
            QByteArray internalName                      = themeParameter->internalName();
            QString value                                = mInfo->getThemeParameterValue(themeInternalName,
                                                                QString::fromLatin1(internalName),
                                                                themeParameter->defaultValue());

            QString name          = themeParameter->name();
            name                  = i18nc("'%1' is a label for a theme parameter", "%1:", name);

            QLabel* const label   = new QLabel(name, content);
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            QWidget* const widget = themeParameter->createWidget(content, value);
            label->setBuddy(widget);

            int row               = layout->rowCount();
            layout->addWidget(label, row, 0);

            if (widget->sizePolicy().expandingDirections() & Qt::Horizontal)
            {
                // Widget wants full width
                layout->addWidget(widget, row, 1, 1, 2);
            }
            else
            {
                // Widget doesn't like to be stretched, add a spacer next to it
                layout->addWidget(widget, row, 1);
                QSpacerItem* const spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding,
                                                            QSizePolicy::Minimum);
                layout->addItem(spacer, row, 2);
            }

            mThemeParameterWidgetFromName[internalName] = widget;
        }

        // Add spacer at the end, so that widgets aren't spread on the whole
        // parent height
        QSpacerItem* const spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout->addItem(spacer, layout->rowCount(), 0);
    }
};

Wizard::Wizard(QWidget* const parent, GalleryInfo* const info)
    : QWizard(parent),
      d(new Private)
{
    d->mInfo                   = info;

    setWindowTitle(i18n("Export image collections to HTML pages"));

    // ---------------------------------------------------------------

    d->mCollectionSelector     = new AlbumSelectTabs(this);
    d->mCollectionSelectorPage = addPage(d->mCollectionSelector, i18n("Album Selection"));
    updateCollectionSelectorPageValidity();

    connect(d->mCollectionSelector, SIGNAL(signalAlbumSelectionChanged()),
            this, SLOT(updateCollectionSelectorPageValidity()));

    d->mThemePage              = new ThemePage(this, i18n("Theme"));
    d->initThemePage();

    connect(d->mThemePage->mThemeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

    d->mThemeParametersPage    = new ThemeParametersPage(this, i18n("Theme Parameters"));
    d->mImageSettingsPage      = new ImageSettingsPage(this, i18n("Image Settings"));
    d->mOutputPage             = new OutputPage(this, i18n("Output"));
    d->mOutputPage->kcfg_destUrl->setFileDlgMode(QFileDialog::Directory);

    connect(d->mOutputPage->kcfg_destUrl, SIGNAL(textChanged(QString)),
            this, SLOT(updateFinishPageValidity()));

    d->mConfigManager          = new KConfigDialogManager(this, d->mInfo);
    d->mConfigManager->updateWidgets();

    // Set page states
    // Pages can only be disabled after they have *all* been added!
    slotThemeSelectionChanged();
    updateFinishPageValidity();
}

Wizard::~Wizard()
{
    delete d;
}

void Wizard::updateFinishPageValidity()
{
    setValid(d->mOutputPage->page(), !d->mOutputPage->kcfg_destUrl->fileDlgPath().isEmpty());
}

void Wizard::updateCollectionSelectorPageValidity()
{
    setValid(d->mCollectionSelectorPage, !d->mCollectionSelector->selectedAlbums().empty());
}

void Wizard::slotThemeSelectionChanged()
{
    QListWidget* const listWidget = d->mThemePage->mThemeList;
    QTextBrowser* const browser   = d->mThemePage->mThemeInfo;

    if (listWidget->currentItem())
    {
        Theme::Ptr theme=static_cast<ThemeListBoxItem*>(listWidget->currentItem())->mTheme;

        QString url                   = theme->authorUrl();
        QString author                = theme->authorName();
        bool allowNonsquareThumbnails = theme->allowNonsquareThumbnails();

        if (!url.isEmpty())
        {
            author = QString::fromUtf8("<a href='%1'>%2</a>").arg(url).arg(author);
        }

        QString preview               = theme->previewUrl();
        QString image                 = QLatin1String("");

        if (!preview.isEmpty())
        {
            image = QString::fromUtf8("<img src='%1/%2' /><br/><br/>").arg(theme->directory(),
                                                                           theme->previewUrl());
        }

        QString txt = image +
                      QString::fromUtf8("<b>%3</b><br/><br/>%4<br/><br/>")
                          .arg(theme->name(), theme->comment()) + 
                      i18n("Author: %1", author);

        browser->setHtml(txt);
        setValid(d->mThemePage->page(), true);

        // Enable theme parameter page if there is any parameter
        Theme::ParameterList parameterList = theme->parameterList();
        setAppropriate(d->mThemeParametersPage->page(), parameterList.size() > 0);

        d->mImageSettingsPage->kcfg_thumbnailSquare->setEnabled(allowNonsquareThumbnails);

        if (!allowNonsquareThumbnails)
        {
            d->mImageSettingsPage->kcfg_thumbnailSquare->setChecked(true);
        }

        d->fillThemeParametersPage(theme);
    }
    else
    {
        browser->clear();
        setValid(d->mThemePage->page(), false);
    }
}

/**
 * Update mInfo
 */
void Wizard::accept()
{
    d->mInfo->mCollectionList               = d->mCollectionSelector->selectedAlbums();
    Theme::Ptr theme                        = static_cast<ThemeListBoxItem*>(d->mThemePage->mThemeList->currentItem())->mTheme;
    QString themeInternalName               = theme->internalName();
    d->mInfo->setTheme(themeInternalName);

    Theme::ParameterList parameterList      = theme->parameterList();
    Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
    Theme::ParameterList::ConstIterator end = parameterList.constEnd();

    for (; it != end ; ++it)
    {
        AbstractThemeParameter* const themeParameter = *it;
        QByteArray parameterInternalName             = themeParameter->internalName();
        QWidget* const widget                        = d->mThemeParameterWidgetFromName[parameterInternalName];
        QString value                                = themeParameter->valueFromWidget(widget);

        d->mInfo->setThemeParameterValue(themeInternalName,
                                         QString::fromLatin1(parameterInternalName),
                                         value);
    }

    d->mConfigManager->updateSettings();

    QWizard::accept();
}

} // namespace Digikam
