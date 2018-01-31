/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a tool to insert a text over an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "inserttexttool.h"

// Qt includes

#include <QBrush>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>
#include <QComboBox>
#include <QApplication>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "inserttextwidget.h"
#include "dfontproperties.h"
#include "dcolorselector.h"
#include "dnuminput.h"

namespace Digikam
{

class InsertTextTool::Private
{
public:

    Private() :
        alignTextMode(0),
        defaultSizeFont(0),
        borderText(0),
        transparentText(0),
        alignButtonGroup(0),
        textRotation(0),
        textOpacity(0),
        fontColorButton(0),
        fontChooserWidget(0),
        textEdit(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configTextRotationEntry;
    static const QString configFontColorEntry;
    static const QString configTextOpacity;
    static const QString configTextStringEntry;
    static const QString configFontPropertiesEntry;
    static const QString configTextAlignmentEntry;
    static const QString configBorderTextEntry;
    static const QString configTransparentTextEntry;
    static const QString configPositionHintEntry;

    int                  alignTextMode;
    int                  defaultSizeFont;

    QCheckBox*           borderText;
    QCheckBox*           transparentText;

    QButtonGroup*        alignButtonGroup;
    QFont                textFont;

    QComboBox*           textRotation;
    DIntNumInput*        textOpacity;
    DColorSelector*      fontColorButton;
    DFontProperties*     fontChooserWidget;
    QTextEdit*           textEdit;

    InsertTextWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString InsertTextTool::Private::configGroupName(QLatin1String("inserttext Tool"));
const QString InsertTextTool::Private::configTextRotationEntry(QLatin1String("Text Rotation"));
const QString InsertTextTool::Private::configFontColorEntry(QLatin1String("Font Color"));
const QString InsertTextTool::Private::configTextOpacity(QLatin1String("Text Opacity"));
const QString InsertTextTool::Private::configTextStringEntry(QLatin1String("Enter your text here."));
const QString InsertTextTool::Private::configFontPropertiesEntry(QLatin1String("Font Properties"));
const QString InsertTextTool::Private::configTextAlignmentEntry(QLatin1String("Text Alignment"));
const QString InsertTextTool::Private::configBorderTextEntry(QLatin1String("Border Text"));
const QString InsertTextTool::Private::configTransparentTextEntry(QLatin1String("Transparent Text"));
const QString InsertTextTool::Private::configPositionHintEntry(QLatin1String("Position Hint"));

// --------------------------------------------------------

InsertTextTool::InsertTextTool(QObject* const parent)
    : EditorTool(parent),
      d(new Private)
{
    setObjectName(QLatin1String("inserttext"));
    setToolName(i18n("Insert Text"));
    setToolIcon(QIcon::fromTheme(QLatin1String("insert-text")));

    // -------------------------------------------------------------

    QFrame* const frame  = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QVBoxLayout* const l = new QVBoxLayout(frame);
    d->previewWidget     = new InsertTextWidget(480, 320, frame);
    l->addWidget(d->previewWidget);
    d->previewWidget->setWhatsThis(i18n("This previews the text inserted in the image. "
                                        "You can use the mouse to move the text to the right location."));
    setToolView(frame);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->textEdit     = new QTextEdit();
    d->textEdit->setWordWrapMode(QTextOption::NoWrap);
    d->textEdit->setWhatsThis( i18n("Here, enter the text you want to insert in your image."));

    // -------------------------------------------------------------

    d->fontChooserWidget = new DFontProperties(0, DFontProperties::NoDisplayFlags);
    d->fontChooserWidget->setSampleBoxVisible(false);
    d->fontChooserWidget->setWhatsThis(i18n("Here you can choose the font to be used."));

    // -------------------------------------------------------------

    QWidget* const alignBox   = new QWidget();
    QHBoxLayout* const hlay   = new QHBoxLayout(alignBox);
    d->alignButtonGroup       = new QButtonGroup(alignBox);
    d->alignButtonGroup->setExclusive(true);

    QToolButton* const alignLeft = new QToolButton(alignBox);
    d->alignButtonGroup->addButton(alignLeft, InsertTextWidget::ALIGN_LEFT);
    alignLeft->setIcon(QIcon::fromTheme(QLatin1String("format-justify-left")));
    alignLeft->setCheckable(true);
    alignLeft->setToolTip(i18n("Align text to the left"));

    QToolButton* const alignRight = new QToolButton(alignBox);
    d->alignButtonGroup->addButton(alignRight, InsertTextWidget::ALIGN_RIGHT);
    alignRight->setIcon(QIcon::fromTheme(QLatin1String("format-justify-right")));
    alignRight->setCheckable(true);
    alignRight->setToolTip(i18n("Align text to the right"));

    QToolButton* const alignCenter = new QToolButton(alignBox);
    d->alignButtonGroup->addButton(alignCenter, InsertTextWidget::ALIGN_CENTER);
    alignCenter->setIcon(QIcon::fromTheme(QLatin1String("format-justify-center")));
    alignCenter->setCheckable(true);
    alignCenter->setToolTip(i18n("Align text to center"));

    QToolButton* alignBlock = new QToolButton(alignBox);
    d->alignButtonGroup->addButton(alignBlock, InsertTextWidget::ALIGN_BLOCK);
    alignBlock->setIcon(QIcon::fromTheme(QLatin1String("format-justify-fill")));
    alignBlock->setCheckable(true);
    alignBlock->setToolTip(i18n("Align text to a block"));

    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);
    hlay->addStretch();
    hlay->addWidget(alignLeft);
    hlay->addWidget(alignRight);
    hlay->addWidget(alignCenter);
    hlay->addWidget(alignBlock);
    hlay->addStretch();

    // -------------------------------------------------------------

    QLabel* const label1 = new QLabel(i18n("Rotation:"));
    d->textRotation      = new QComboBox();
    d->textRotation->addItem(i18nc("no rotation", "None"));
    d->textRotation->addItem(i18n("90 Degrees"));
    d->textRotation->addItem(i18n("180 Degrees"));
    d->textRotation->addItem(i18n("270 Degrees"));
    d->textRotation->setWhatsThis(i18n("Select the text rotation to use here."));

    // -------------------------------------------------------------

    QLabel* const label2 = new QLabel(i18nc("font color", "Color:"));
    d->fontColorButton   = new DColorSelector();
    d->fontColorButton->setColor(Qt::black);
    d->fontColorButton->setWhatsThis(i18n("Set here the font color to use."));

    // -------------------------------------------------------------

    QLabel* const label3 = new QLabel(i18nc("text opacity", "Opacity:"));
    d->textOpacity       = new DIntNumInput();
    d->textOpacity->setRange(0, 100, 1);
    d->textOpacity->setDefaultValue(100);
    d->textOpacity->setSuffix(QLatin1String("%"));
    d->textOpacity->setWhatsThis(i18n("Select the text opacity to use here."));

    // -------------------------------------------------------------

    d->borderText      = new QCheckBox(i18n("Add border"));
    d->borderText->setToolTip(i18n("Add a solid border around text using current text color"));

    d->transparentText = new QCheckBox(i18n("Semi-transparent"));
    d->transparentText->setToolTip(i18n("Use semi-transparent text background under image"));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(d->textEdit,             0, 0, 3, -1);
    mainLayout->addWidget(d->fontChooserWidget,    3, 0, 1, -1);
    mainLayout->addWidget(alignBox,                4, 0, 1, -1);
    mainLayout->addWidget(label1,                  5, 0, 1,  1);
    mainLayout->addWidget(d->textRotation,         5, 1, 1,  1);
    mainLayout->addWidget(label2,                  6, 0, 1,  1);
    mainLayout->addWidget(d->fontColorButton,      6, 1, 1,  1);
    mainLayout->addWidget(label3,                  7, 0, 1,  1);
    mainLayout->addWidget(d->textOpacity,          7, 1, 1,  1);
    mainLayout->addWidget(d->borderText,           8, 0, 1, -1);
    mainLayout->addWidget(d->transparentText,      9, 0, 1, -1);
    mainLayout->setRowStretch(10, 10);
    mainLayout->setColumnStretch(1, 5);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->fontChooserWidget, SIGNAL(fontSelected(QFont)),
            this, SLOT(slotFontPropertiesChanged(QFont)));

    connect(d->fontColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotUpdatePreview()));

    connect(d->textOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdatePreview()));

    connect(d->textEdit, SIGNAL(textChanged()),
            this, SLOT(slotUpdatePreview()));

    connect(d->alignButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotAlignModeChanged(int)));

    connect(d->borderText, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdatePreview()));

    connect(d->transparentText, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdatePreview()));

    connect(d->textRotation, SIGNAL(activated(int)),
            this, SLOT(slotUpdatePreview()));

    connect(this, SIGNAL(signalUpdatePreview()),
            this, SLOT(slotUpdatePreview()));

    // -------------------------------------------------------------

    slotUpdatePreview();
}

InsertTextTool::~InsertTextTool()
{
    delete d;
}

void InsertTextTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    QColor black(0, 0, 0);
    QFont  defaultFont;

    int orgW = d->previewWidget->imageIface()->originalSize().width();
    int orgH = d->previewWidget->imageIface()->originalSize().height();

    if ( orgW > orgH )
    {
        d->defaultSizeFont = (int)(orgH / 8.0);
    }
    else
    {
        d->defaultSizeFont = (int)(orgW / 8.0);
    }

    defaultFont.setPointSize(d->defaultSizeFont);

    d->textRotation->setCurrentIndex(group.readEntry(d->configTextRotationEntry,  0));
    d->fontColorButton->setColor(group.readEntry(d->configFontColorEntry,         black));
    d->textOpacity->setValue(group.readEntry(d->configTextOpacity,                100));
    d->textEdit->setText(group.readEntry(d->configTextStringEntry,                i18n("Enter your text here.")));
    d->textFont = group.readEntry(d->configFontPropertiesEntry,                   defaultFont);
    d->fontChooserWidget->setFont(d->textFont);
    d->alignTextMode = group.readEntry(d->configTextAlignmentEntry,               (int) InsertTextWidget::ALIGN_LEFT);
    d->borderText->setChecked(group.readEntry(d->configBorderTextEntry,           false));
    d->transparentText->setChecked(group.readEntry(d->configTransparentTextEntry, false));
    d->previewWidget->setPositionHint(group.readEntry(d->configPositionHintEntry, QRect()));

    d->alignButtonGroup->button(d->alignTextMode)->setChecked(true);
    slotAlignModeChanged(d->alignTextMode);
}

void InsertTextTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configTextRotationEntry,    d->textRotation->currentIndex());
    group.writeEntry(d->configFontColorEntry,       d->fontColorButton->color());
    group.writeEntry(d->configTextOpacity,          d->textOpacity->value());
    group.writeEntry(d->configTextStringEntry,      d->textEdit->document()->toPlainText());
    group.writeEntry(d->configFontPropertiesEntry,  d->textFont);
    group.writeEntry(d->configTextAlignmentEntry,   d->alignTextMode);
    group.writeEntry(d->configBorderTextEntry,      d->borderText->isChecked());
    group.writeEntry(d->configTransparentTextEntry, d->transparentText->isChecked());
    group.writeEntry(d->configPositionHintEntry,    d->previewWidget->getPositionHint());

    config->sync();
}

void InsertTextTool::slotResetSettings()
{
    d->fontColorButton->blockSignals(true);
    d->alignButtonGroup->blockSignals(true);
    d->fontChooserWidget->blockSignals(true);

    d->textRotation->setCurrentIndex(0); // No rotation.
    d->fontColorButton->setColor(Qt::black);
    d->textOpacity->slotReset();
    QFont defaultFont;
    d->textFont = defaultFont; // Reset to default KDE font.
    d->textFont.setPointSize(d->defaultSizeFont);
    d->fontChooserWidget->setFont(d->textFont);
    d->borderText->setChecked(false);
    d->transparentText->setChecked(false);
    d->previewWidget->resetEdit();
    d->alignButtonGroup->button(InsertTextWidget::ALIGN_LEFT)->setChecked(true);

    d->fontChooserWidget->blockSignals(false);
    d->fontColorButton->blockSignals(false);
    d->alignButtonGroup->blockSignals(false);
    slotAlignModeChanged(InsertTextWidget::ALIGN_LEFT);
}

void InsertTextTool::slotAlignModeChanged(int mode)
{
    d->alignTextMode = mode;
    d->textEdit->selectAll();

    switch (d->alignTextMode)
    {
        case InsertTextWidget::ALIGN_LEFT:
            d->textEdit->setAlignment(Qt::AlignLeft);
            break;

        case InsertTextWidget::ALIGN_RIGHT:
            d->textEdit->setAlignment(Qt::AlignRight);
            break;

        case InsertTextWidget::ALIGN_CENTER:
            d->textEdit->setAlignment(Qt::AlignHCenter);
            break;

        case InsertTextWidget::ALIGN_BLOCK:
            d->textEdit->setAlignment(Qt::AlignJustify);
            break;
    }

    d->textEdit->textCursor().clearSelection();
    emit signalUpdatePreview();
}

void InsertTextTool::slotFontPropertiesChanged(const QFont& font)
{
    d->textFont = font;
    emit signalUpdatePreview();
}

void InsertTextTool::setBackgroundColor(const QColor& bg)
{
    d->previewWidget->setBackgroundColor(bg);
}

void InsertTextTool::slotUpdatePreview()
{
    d->previewWidget->setText(d->textEdit->document()->toPlainText(), d->textFont, d->fontColorButton->color(),
                              d->textOpacity->value(), d->alignTextMode,
                              d->borderText->isChecked(), d->transparentText->isChecked(),
                              d->textRotation->currentIndex());
}

void InsertTextTool::finalRendering()
{
    qApp->setOverrideCursor( Qt::WaitCursor );

    ImageIface iface;
    DImg dest = d->previewWidget->makeInsertText();

    FilterAction action(QLatin1String("digikam:insertTextTool"), 2);
    action.setDisplayableName(i18n("Insert Text Tool"));

    action.addParameter(QLatin1String("text"),              d->textEdit->toPlainText());
    action.addParameter(QLatin1String("textRotationIndex"), d->textRotation->currentIndex());
    action.addParameter(QLatin1String("textFont"),          d->textFont.toString());
    action.addParameter(QLatin1String("colorR"),            d->fontColorButton->color().red());
    action.addParameter(QLatin1String("colorG"),            d->fontColorButton->color().green());
    action.addParameter(QLatin1String("colorB"),            d->fontColorButton->color().blue());
    action.addParameter(QLatin1String("colorA"),            d->fontColorButton->color().alpha());
    action.addParameter(QLatin1String("textOpacity"),       d->textOpacity->value());
    action.addParameter(QLatin1String("borderText"),        d->borderText->isChecked());
    action.addParameter(QLatin1String("transparentText"),   d->transparentText->isChecked());

    iface.setOriginal(i18n("Insert Text"), action, dest);

    qApp->restoreOverrideCursor();
}

}  // namespace Digikam
