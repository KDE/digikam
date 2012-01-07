/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-10
 * Description : a tool bar for preview mode
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewtoolbar.moc"

// Qt includes

#include <QButtonGroup>
#include <QLayout>
#include <QToolButton>
#include <QAbstractButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace Digikam
{

class PreviewToolBar::PreviewToolBarPriv
{

public:

    PreviewToolBarPriv() :
        previewOriginalButton(0),
        previewBothButtonVert(0),
        previewBothButtonHorz(0),
        previewDuplicateBothButtonVert(0),
        previewDupplicateBothButtonHorz(0),
        previewtargetButton(0),
        previewToggleMouseOverButton(0),
        previewButtons(0)
    {
    }

    QToolButton*  previewOriginalButton;
    QToolButton*  previewBothButtonVert;
    QToolButton*  previewBothButtonHorz;
    QToolButton*  previewDuplicateBothButtonVert;
    QToolButton*  previewDupplicateBothButtonHorz;
    QToolButton*  previewtargetButton;
    QToolButton*  previewToggleMouseOverButton;

    QButtonGroup* previewButtons;
};

PreviewToolBar::PreviewToolBar(QWidget* parent)
    : QWidget(parent), d(new PreviewToolBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout* hlay = new QHBoxLayout(this);
    d->previewButtons = new QButtonGroup(this);
    d->previewButtons->setExclusive(true);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    d->previewOriginalButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewOriginalButton, PreviewOriginalImage);
    hlay->addWidget(d->previewOriginalButton);
    d->previewOriginalButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/original.png")));
    d->previewOriginalButton->setCheckable(true);
    d->previewOriginalButton->setWhatsThis(i18n("If this option is enabled, the original image will be shown."));
    d->previewOriginalButton->setToolTip(i18n("Original image"));

    d->previewBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(d->previewBothButtonVert, PreviewBothImagesVertCont);
    hlay->addWidget(d->previewBothButtonVert);
    d->previewBothButtonVert->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothvert.png")));
    d->previewBothButtonVert->setCheckable(true);
    d->previewBothButtonVert->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                "split vertically. "
                                                "A contiguous area of the image will be shown, "
                                                "with one half from the original image, "
                                                "the other half from the target image."));
    d->previewBothButtonVert->setToolTip(i18n("Vertical split with contiguous image"));

    d->previewBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(d->previewBothButtonHorz, PreviewBothImagesHorzCont);
    hlay->addWidget(d->previewBothButtonHorz);
    d->previewBothButtonHorz->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothhorz.png")));
    d->previewBothButtonHorz->setCheckable(true);
    d->previewBothButtonHorz->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                "split horizontally. "
                                                "A contiguous area of the image will be shown, "
                                                "with one half from the original image, "
                                                "the other half from the target image."));
    d->previewBothButtonHorz->setToolTip(i18n("Horizontal split with contiguous image"));

    d->previewDuplicateBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(d->previewDuplicateBothButtonVert, PreviewBothImagesVert);
    hlay->addWidget(d->previewDuplicateBothButtonVert);
    d->previewDuplicateBothButtonVert->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothvert.png")));
    d->previewDuplicateBothButtonVert->setCheckable(true);
    d->previewDuplicateBothButtonVert->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                         "split vertically. "
                                                         "The same part of the original and the target image "
                                                         "will be shown side by side."));
    d->previewDuplicateBothButtonVert->setToolTip(i18n("Vertical split with same image region"));

    d->previewDupplicateBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(d->previewDupplicateBothButtonHorz, PreviewBothImagesHorz);
    hlay->addWidget(d->previewDupplicateBothButtonHorz);
    d->previewDupplicateBothButtonHorz->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothhorz.png")));
    d->previewDupplicateBothButtonHorz->setCheckable(true);
    d->previewDupplicateBothButtonHorz->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                          "split horizontally. "
                                                          "The same part of the original and the target image "
                                                          "will be shown side by side."));
    d->previewDupplicateBothButtonHorz->setToolTip(i18n("Horizontal split with same image region"));

    d->previewtargetButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewtargetButton, PreviewTargetImage);
    hlay->addWidget(d->previewtargetButton);
    d->previewtargetButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/target.png")));
    d->previewtargetButton->setCheckable(true);
    d->previewtargetButton->setWhatsThis(i18n("If this option is enabled, the target image will be shown."));
    d->previewtargetButton->setToolTip(i18n("Target image"));

    d->previewToggleMouseOverButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewToggleMouseOverButton, PreviewToggleOnMouseOver);
    hlay->addWidget(d->previewToggleMouseOverButton);
    d->previewToggleMouseOverButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/togglemouseover.png")));
    d->previewToggleMouseOverButton->setCheckable(true);
    d->previewToggleMouseOverButton->setWhatsThis(i18n("If this option is enabled, the original image will "
                                                       "be shown when the mouse is over image area; otherwise, "
                                                       "the target image will be shown."));
    d->previewToggleMouseOverButton->setToolTip(i18n("Mouse-over mode"));

    connect(d->previewButtons, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPreviewModeChanged(int)));
}

PreviewToolBar::~PreviewToolBar()
{
    delete d;
}

void PreviewToolBar::setPreviewModeMask(int mask)
{
    if (mask == NoPreviewMode)
    {
        setDisabled(true);
        return;
    }

    setDisabled(false);

    d->previewOriginalButton->setEnabled(mask           & PreviewOriginalImage);
    d->previewBothButtonVert->setEnabled(mask           & PreviewBothImagesHorz);
    d->previewBothButtonHorz->setEnabled(mask           & PreviewBothImagesVert);
    d->previewDuplicateBothButtonVert->setEnabled(mask  & PreviewBothImagesHorzCont);
    d->previewDupplicateBothButtonHorz->setEnabled(mask & PreviewBothImagesVertCont);
    d->previewtargetButton->setEnabled(mask             & PreviewTargetImage);
    d->previewToggleMouseOverButton->setEnabled(mask    & PreviewToggleOnMouseOver);

    // When we switch to another mask, check if current mode is valid.
    PreviewToolBar::PreviewMode mode = previewMode();

    if (d->previewButtons->button(mode))
    {
        if (!d->previewButtons->button(mode)->isEnabled())
        {
            QList<QAbstractButton*> btns = d->previewButtons->buttons();
            foreach(QAbstractButton* btn, btns)
            {
                if (btn && btn->isEnabled())
                {
                    btn->setChecked(true);
                    return;
                }
            }
        }
    }
}

void PreviewToolBar::setPreviewMode(PreviewMode mode)
{
    if (d->previewButtons->button(mode))
    {
        d->previewButtons->button(mode)->setChecked(true);
    }
}

PreviewToolBar::PreviewMode PreviewToolBar::previewMode() const
{
    if (!isEnabled())
    {
        return PreviewToolBar::NoPreviewMode;
    }

    return ((PreviewMode)d->previewButtons->checkedId());
}

void PreviewToolBar::readSettings(KConfigGroup& group)
{
    int mode = group.readEntry("PreviewMode", (int)PreviewBothImagesVertCont);
    mode     = qMax((int)PreviewOriginalImage, mode);
    mode     = qMin((int)PreviewToggleOnMouseOver, mode);
    setPreviewMode((PreviewMode)mode);
}

void PreviewToolBar::writeSettings(KConfigGroup& group)
{
    group.writeEntry("PreviewMode", (int)previewMode());
}

}  // namespace Digikam
