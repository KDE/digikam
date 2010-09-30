/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-12
 * Description : Widget for assignment and confirmation of names for faces
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "assignnamewidget.moc"

// Qt includes

#include <QGridLayout>
#include <QKeyEvent>
#include <QToolButton>

// KDE includes

#include <kdebug.h>
#include <kstandardguiitem.h>

// libkface includes

#include <libkface/kface.h>

// Local includes

#include "addtagscombobox.h"
#include "addtagscompletionbox.h"
#include "addtagslineedit.h"
#include "album.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "dimg.h"
#include "imageinfo.h"

namespace Digikam
{

class AssignNameWidget::AssignNameWidgetPriv
{
public:

    AssignNameWidgetPriv(AssignNameWidget* q) : q(q)
    {
        mode           = AssignNameWidget::InvalidMode;
        layoutMode     = AssignNameWidget::InvalidLayout;
        bgStyle        = AssignNameWidget::InvalidBackgroundStyle;
        comboBox       = 0;
        confirmButton  = 0;
        rejectButton   = 0;
        layout         = 0;
        tagModel       = 0;
        tagFilterModel = 0;
        tagFilteredModel = 0;
    }

    void         checkWidgets();
    void         clearWidgets();
    void         updateLayout();
    QToolButton* createToolButton(const KGuiItem& item);

public:

    ImageInfo                         info;
    QVariant                          faceIdentifier;

    AssignNameWidget::Mode            mode;
    AssignNameWidget::LayoutMode      layoutMode;
    AssignNameWidget::BackgroundStyle bgStyle;

    AddTagsComboBox*                  comboBox;
    QToolButton*                      confirmButton;
    QToolButton*                      rejectButton;

    TagModel*                         tagModel;
    CheckableAlbumFilterModel*        tagFilterModel;
    TagPropertiesFilterModel*         tagFilteredModel;
    AlbumPointer<TAlbum>              parentTag;

    QGridLayout*                      layout;

    AssignNameWidget* const           q;
};

void AssignNameWidget::AssignNameWidgetPriv::clearWidgets()
{
    delete comboBox;
    comboBox  = 0;
    delete confirmButton;
    confirmButton = 0;
    delete rejectButton;
    rejectButton = 0;
}

QToolButton* AssignNameWidget::AssignNameWidgetPriv::createToolButton(const KGuiItem& gui)
{
    QToolButton* b = new QToolButton;
    b->setIcon(gui.icon());
    b->setText(gui.text());
    b->setToolTip(gui.toolTip());
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    return b;
}

void AssignNameWidget::AssignNameWidgetPriv::checkWidgets()
{
    if (mode == InvalidMode)
    {
        clearWidgets();
        return;
    }

    if (!comboBox)
    {
        comboBox = new AddTagsComboBox(q);
        if (tagModel)
            comboBox->setModel(tagModel, tagFilteredModel, tagFilterModel);

        q->connect(comboBox, SIGNAL(taggingActionActivated(const TaggingAction&)),
                   q, SLOT(actionActivated(const TaggingAction&)));

        if (parentTag)
            comboBox->setParentTag(parentTag);

        comboBox->setMinimumContentsLength(15);
        /*comboBox->lineEdit()->completionBox()->setObjectName("addTagsComboBox-completionBox");
        comboBox->lineEdit()->completionBox()->setFrameStyle(QFrame::StyledPanel);
        comboBox->view()->parentWidget()->setObjectName("addTagsComboBox-viewContainer");*/
   }

    if (!confirmButton)
    {
        confirmButton = createToolButton(KStandardGuiItem::ok());
        confirmButton->setText(i18n("Confirm"));
        //confirmButton->setToolTip(i18n("Confirm that the selected person is shown here"));
        q->connect(confirmButton, SIGNAL(clicked()),
                   q, SLOT(slotConfirm()));
    }

    if (!rejectButton)
    {
        rejectButton  = createToolButton(KStandardGuiItem::remove());
        q->connect(rejectButton, SIGNAL(clicked()),
                   q, SLOT(slotReject()));
    }
}

void AssignNameWidget::AssignNameWidgetPriv::updateLayout()
{
    if (mode == InvalidMode)
        return;

    layout->removeWidget(comboBox);
    layout->removeWidget(confirmButton);
    layout->removeWidget(rejectButton);

    if (layoutMode == AssignNameWidget::FullLine)
    {
        layout->addWidget(comboBox,  0, 0);
        layout->addWidget(confirmButton, 0, 1);
        layout->addWidget(rejectButton, 0, 2);
        layout->setColumnStretch(0, 1);
    }
    else if (layoutMode == AssignNameWidget::TwoLines || layoutMode == AssignNameWidget::Compact)
    {
        layout->addWidget(comboBox,  0, 0, 1, 2);
        layout->addWidget(confirmButton, 1, 0);
        layout->addWidget(rejectButton, 1, 1);
    }
}

AssignNameWidget::AssignNameWidget(QWidget* parent)
                : QFrame(parent), d(new AssignNameWidgetPriv(this))
{
    d->layout = new QGridLayout;
    setLayout(d->layout);

    setLayoutMode(FullLine);
    setBackgroundStyle(StyledFrame);
}

AssignNameWidget::~AssignNameWidget()
{
    delete d;
}

void AssignNameWidget::setTagModel(TagModel* model, TagPropertiesFilterModel *filteredModel, CheckableAlbumFilterModel* filterModel)
{
    d->tagModel = model;
    d->tagFilterModel = filterModel;
    d->tagFilteredModel  = filteredModel;
    if (d->comboBox)
        d->comboBox->setModel(d->tagModel, d->tagFilteredModel, d->tagFilterModel);
}

void AssignNameWidget::setParentTag(TAlbum* album)
{
    d->parentTag = album;
    if (d->comboBox)
        d->comboBox->setParentTag(album);
}

AddTagsComboBox* AssignNameWidget::comboBox() const
{
    return d->comboBox;
}

void AssignNameWidget::setMode(Mode mode)
{
    if (mode == d->mode)
        return;

    d->mode = mode;
    d->checkWidgets();
    d->updateLayout();
}

AssignNameWidget::Mode AssignNameWidget::mode() const
{
    return d->mode;
}

void AssignNameWidget::setLayoutMode(LayoutMode mode)
{
    if (d->layoutMode == mode)
        return;

    d->layoutMode = mode;
    d->updateLayout();
}

AssignNameWidget::LayoutMode AssignNameWidget::layoutMode() const
{
    return d->layoutMode;
}

void AssignNameWidget::setBackgroundStyle(BackgroundStyle style)
{
    if (d->bgStyle == style)
        return;

    if (style == TransparentRound)
    {
        setStyleSheet(
            "QFrame {"
            "  background-color: rgba(0, 0, 0, 66%); "
            "  border: 1px solid rgba(100, 100, 100, 66%); "
            "  border-radius: 8px; "
            "} "

            "QToolButton { "
            "  color: rgba(255,255,255,220); "
            "  padding: 1px; "
            "  background-color: "
            "    qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,100), "
            "                    stop:1 rgba(255,255,255,0)); "
            "  border: 1px solid rgba(255,255,255,127); "
            "  border-radius: 4px; "
            "} "

            "QToolButton:hover { "
            "  border-color: white; "
            "} "

            "QToolButton:pressed { "
            "  background-color: "
            "    qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,0), "
            "                    stop:1 rgba(255,255,255,100)); "
            "  border-color: white; "
            "} "

            "QComboBox { "
            "  color: white; "
            "  background-color: transparent; "
            "} "

            "QComboBox QAbstractItemView, KCompletionBox::item:!selected { "
            "  color: white; "
            "  background-color: rgba(0,0,0,80%); "
            "} "
            /*"KCompletionBox::item:hover, KCompletionBox::item:selected { "
            "  background-color: "
            "     qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(150,150,150,80%), "
            "                     stop:0.5 rgba(25,25,25,100%), stop:1 rgba(150,150,150,80%)); "
            " } "*/
        );
    }
    else if (style == StyledFrame)
    {
        setStyleSheet(QString());
        setFrameStyle(Raised | StyledPanel);
    }
    d->bgStyle = style;
}

AssignNameWidget::BackgroundStyle AssignNameWidget::backgroundStyle() const
{
    return d->bgStyle;
}

void AssignNameWidget::setFace(const ImageInfo& info, const QVariant& faceIdentifier)
{
    d->info           = info;
    d->faceIdentifier = faceIdentifier;
}

ImageInfo AssignNameWidget::info() const
{
    return d->info;
}

QVariant AssignNameWidget::faceIdentifier() const
{
    return d->faceIdentifier;
}

void AssignNameWidget::slotConfirm()
{
    if (d->comboBox)
        emit assigned(d->comboBox->currentTaggingAction(), d->info, d->faceIdentifier);
}

void AssignNameWidget::slotReject()
{
    emit rejected(d->info, d->faceIdentifier);
}

void AssignNameWidget::actionActivated(const TaggingAction& action)
{
    emit assigned(action, d->info, d->faceIdentifier);
}

void AssignNameWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            slotConfirm();
            return;
        case Qt::Key_Escape:
            slotReject();
            return;
    }

    QWidget::keyPressEvent(e);
}

} // namespace Digikam
