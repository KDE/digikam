/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "ddebug.h"
#include "imagepaniconwidget.h"
#include "editortoolsettings.h"
#include "editortoolsettings.moc"

using namespace KDcrawIface;

namespace Digikam
{

class EditorToolSettingsPriv
{

public:

    EditorToolSettingsPriv()
    {
        okBtn        = 0;
        cancelBtn    = 0;
        tryBtn       = 0;
        defaultBtn   = 0;
        mainVBox     = 0;
        plainPage    = 0;
        btnBox1      = 0;
        btnBox2      = 0;
        btnBox3      = 0;
        saveAsBtn    = 0;
        loadBtn      = 0;
        guideBox     = 0;
        guideColorBt = 0;
        guideSize    = 0;
        panIconView  = 0;
    }

    QHBox              *btnBox1;
    QHBox              *btnBox2;
    QHBox              *btnBox3;
    QHBox              *guideBox;

    QVBox              *mainVBox;
    QWidget            *plainPage;

    KPushButton        *okBtn;
    KPushButton        *cancelBtn;
    KPushButton        *tryBtn;
    KPushButton        *defaultBtn;
    KPushButton        *saveAsBtn;
    KPushButton        *loadBtn;

    KColorButton       *guideColorBt;

    ImagePanIconWidget *panIconView;

    RIntNumInput       *guideSize;
};

EditorToolSettings::EditorToolSettings(int buttonMask, int toolMask, QWidget *parent)
                  : QScrollView(parent)
{
    d = new EditorToolSettingsPriv;

    viewport()->setBackgroundMode(Qt::PaletteBackground);
    setResizePolicy(QScrollView::AutoOneFit);
    setFrameStyle(QFrame::NoFrame);

    d->mainVBox = new QVBox(viewport());
    addChild(d->mainVBox);

    // ---------------------------------------------------------------

    QFrame *frame     = new QFrame(d->mainVBox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* vlay = new QVBoxLayout(frame, 5, 0);
    d->panIconView    = new ImagePanIconWidget(360, 240, frame);
    QWhatsThis::add(d->panIconView, i18n("<p>Here you can see the original image panel "
                                         "which can help you to select the clip preview."
                                         "<p>Click and drag the mouse cursor in the "
                                         "red rectangle to change the clip focus."));
    vlay->addWidget(d->panIconView, 0, Qt::AlignCenter);

    if (!(toolMask & PanIcon))
        frame->hide();

    // ---------------------------------------------------------------

    d->plainPage = new QWidget(d->mainVBox);
    d->guideBox  = new QHBox(d->mainVBox);
    d->btnBox1   = new QHBox(d->mainVBox);
    d->btnBox2   = new QHBox(d->mainVBox);

    // ---------------------------------------------------------------

    new QLabel(i18n("Guide:"), d->guideBox);
    QLabel *space4  = new QLabel(d->guideBox);
    d->guideColorBt = new KColorButton(QColor(Qt::red), d->guideBox);
    QWhatsThis::add(d->guideColorBt, i18n("<p>Set here the color used to draw guides dashed-lines."));
    d->guideSize    = new RIntNumInput(d->guideBox);
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setDefaultValue(1);
    QWhatsThis::add(d->guideSize, i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));

    d->guideBox->setStretchFactor(space4, 10);
    d->guideBox->setSpacing(spacingHint());
    d->guideBox->setMargin(0);

    if (!(toolMask & ColorGuide))
        d->guideBox->hide();

    // ---------------------------------------------------------------

    d->defaultBtn = new KPushButton(d->btnBox1);
    d->defaultBtn->setGuiItem(KStdGuiItem::defaults());
    d->defaultBtn->setIconSet(SmallIconSet("reload_page"));
    QToolTip::add(d->defaultBtn, i18n("<p>Reset all settings to their default values."));
    if (!(buttonMask & Default))
        d->defaultBtn->hide();

    QLabel *space = new QLabel(d->btnBox1);

    d->okBtn = new KPushButton(d->btnBox1);
    d->okBtn->setGuiItem(KStdGuiItem::ok());
    if (!(buttonMask & Ok))
        d->okBtn->hide();

    d->cancelBtn = new KPushButton(d->btnBox1);
    d->cancelBtn->setGuiItem(KStdGuiItem::cancel());
    if (!(buttonMask & Cancel))
        d->cancelBtn->hide();

    d->btnBox1->setStretchFactor(space, 10);
    d->btnBox1->setSpacing(spacingHint());
    d->btnBox1->setMargin(0);

    if (!(buttonMask & Default) && !(buttonMask & Ok) && !(buttonMask & Cancel))
        d->btnBox1->hide();

    // ---------------------------------------------------------------

    d->loadBtn = new KPushButton(d->btnBox2);
    d->loadBtn->setGuiItem(KStdGuiItem::open());
    d->loadBtn->setText(i18n("Load..."));
    QToolTip::add(d->loadBtn, i18n("<p>Load all parameters from settings text file."));
    if (!(buttonMask & Load))
        d->loadBtn->hide();

    d->saveAsBtn = new KPushButton(d->btnBox2);
    d->saveAsBtn->setGuiItem(KStdGuiItem::saveAs());
    QToolTip::add(d->saveAsBtn, i18n("<p>Save all parameters to settings text file."));
    if (!(buttonMask & SaveAs))
        d->saveAsBtn->hide();

    QLabel *space2 = new QLabel(d->btnBox2);

    d->tryBtn = new KPushButton(d->btnBox2);
    d->tryBtn->setGuiItem(KStdGuiItem::apply());
    d->tryBtn->setText(i18n("Try"));
    QToolTip::add(d->tryBtn, i18n("<p>Try all settings."));
    if (!(buttonMask & Try))
        d->tryBtn->hide();

    d->btnBox2->setStretchFactor(space2, 10);
    d->btnBox2->setSpacing(spacingHint());
    d->btnBox2->setMargin(0);

    if (!(buttonMask & Load) && !(buttonMask & SaveAs) && !(buttonMask & Try))
        d->btnBox2->hide();

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SIGNAL(signalDefaultClicked()));

    connect(d->saveAsBtn, SIGNAL(clicked()),
            this, SIGNAL(signalSaveAsClicked()));

    connect(d->loadBtn, SIGNAL(clicked()),
            this, SIGNAL(signalLoadClicked()));

    connect(d->guideColorBt, SIGNAL(changed(const QColor&)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalColorGuideChanged()));
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

QSize EditorToolSettings::minimumSizeHint() const
{
    // Editor Tools usually require a larger horizontal space than other widgets in right side bar
    // Set scroll area to a horizontal minimum size sufficient for the settings.
    // Do not touch vertical size hint.
    // Limit to 40% of the desktop width.
    QSize hint = QScrollView::minimumSizeHint();
    QRect desktopRect = KGlobalSettings::desktopGeometry(d->mainVBox);
    hint.setWidth(QMIN(d->mainVBox->minimumSizeHint().width(), desktopRect.width() * 2 / 5));
    return hint;
}

int EditorToolSettings::marginHint()
{
    return KDialog::marginHint();
}

int EditorToolSettings::spacingHint()
{
    return KDialog::spacingHint();
}

QWidget *EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

ImagePanIconWidget* EditorToolSettings::panIconView() const
{
    return d->panIconView;
}

KPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
        return d->defaultBtn;

    if (buttonCode & Try)
        return d->tryBtn;

    if (buttonCode & Ok)
        return d->okBtn;

    if (buttonCode & Cancel)
        return d->cancelBtn;

    if (buttonCode & Load)
        return d->loadBtn;

    if (buttonCode & SaveAs)
        return d->saveAsBtn;

    return 0;
}

void EditorToolSettings::enableButton(int buttonCode, bool state)
{
    KPushButton *btn = button(buttonCode);
    if (btn) btn->setEnabled(state);
}

QColor EditorToolSettings::guideColor() const
{
    return d->guideColorBt->color();
}

void EditorToolSettings::setGuideColor(const QColor& color)
{
    d->guideColorBt->setColor(color);
}

int EditorToolSettings::guideSize() const
{
    return d->guideSize->value();
}

void EditorToolSettings::setGuideSize(int size)
{
    d->guideSize->setValue(size);
}

} // NameSpace Digikam
