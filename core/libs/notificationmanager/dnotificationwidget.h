/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-03
 * Description : A widget to provide feedback or propose opportunistic interactions
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2011      by Aurelien Gateau <agateau at kde dot org>
 * Copyright (c) 2014      by Dominik Haumann <dhaumann at kde dot org>
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

#ifndef DNOTIFICATION_WIDGET_H
#define DNOTIFICATION_WIDGET_H

// Qt includes

#include <QFrame>
#include <QIcon>
#include <QString>
#include <QAction>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * This widget can be used to provide inline positive or negative
 * feedback, or to implement opportunistic interactions.
 */
class DIGIKAM_EXPORT DNotificationWidget : public QFrame
{
    Q_OBJECT
    Q_ENUMS(MessageType)

    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
    Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible)
    Q_PROPERTY(MessageType messageType READ messageType WRITE setMessageType)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

public:

    /**
     * Available message types.
     * The background colors are chosen depending on the message type.
     */
    enum MessageType
    {
        Positive,
        Information,
        Warning,
        Error
    };

public:

    /**
     * Constructs a DNotificationWidget with the specified @p parent.
     */
    explicit DNotificationWidget(QWidget* parent = 0);

    /**
     * Constructs a DNotificationWidget with the specified @p parent and
     * contents @p text.
     */
    explicit DNotificationWidget(const QString& text, QWidget* parent = 0);

    /**
     * Destructor.
     */
    ~DNotificationWidget();

    /**
     * Get the text of this message widget.
     * @see setText()
     */
    QString text() const;

    /**
     * Check whether word wrap is enabled.
     *
     * If word wrap is enabled, the message widget wraps the displayed text
     * as required to the available width of the widget. This is useful to
     * avoid breaking widget layouts.
     *
     * @see setWordWrap()
     */
    bool wordWrap() const;

    /**
     * Check whether the close button is visible.
     *
     * @see setCloseButtonVisible()
     */
    bool isCloseButtonVisible() const;

    /**
     * Get the type of this message.
     * By default, the type is set to DNotificationWidget::Information.
     *
     * @see DNotificationWidget::MessageType, setMessageType()
     */
    MessageType messageType() const;

    /**
     * Add @p action to the message widget.
     * For each action a button is added to the message widget in the
     * order the actions were added.
     *
     * @param action the action to add
     * @see removeAction(), QWidget::actions()
     */
    void addAction(QAction* action);

    /**
     * Remove @p action from the message widget.
     *
     * @param action the action to remove
     * @see DNotificationWidget::MessageType, addAction(), setMessageType()
     */
    void removeAction(QAction* action);

    /**
     * Returns the preferred size of the message widget.
     */
    QSize sizeHint() const Q_DECL_OVERRIDE;

    /**
     * Returns the minimum size of the message widget.
     */
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    /**
     * Returns the required height for @p width.
     * @param width the width in pixels
     */
    int heightForWidth(int width) const Q_DECL_OVERRIDE;

    /**
     * The icon shown on the left of the text. By default, no icon is shown.
     */
    QIcon icon() const;

    /**
     * Check whether the hide animation started by calling animatedHide()
     * is still running. If animations are disabled, this function always
     * returns @e false.
     *
     * @see animatedHide(), hideAnimationFinished()
     */
    bool isHideAnimationRunning() const;

    /**
     * Check whether the show animation started by calling animatedShow()
     * is still running. If animations are disabled, this function always
     * returns @e false.
     *
     * @see animatedShow(), showAnimationFinished()
     */
    bool isShowAnimationRunning() const;

public Q_SLOTS:

    /**
     * Set the text of the message widget to @p text.
     * If the message widget is already visible, the text changes on the fly.
     *
     * @param text the text to display, rich text is allowed
     * @see text()
     */
    void setText(const QString& text);

    /**
     * Set word wrap to @p wordWrap. If word wrap is enabled, the text()
     * of the message widget is wrapped to fit the available width.
     * If word wrap is disabled, the message widget's minimum size is
     * such that the entire text fits.
     *
     * @param wordWrap disable/enable word wrap
     * @see wordWrap()
     */
    void setWordWrap(bool wordWrap);

    /**
     * Set the visibility of the close button. If @p visible is @e true,
     * a close button is shown that calls animatedHide() if clicked.
     *
     * @see closeButtonVisible(), animatedHide()
     */
    void setCloseButtonVisible(bool visible);

    /**
     * Set the message type to @p type.
     * By default, the message type is set to DNotificationWidget::Information.
     *
     * @see messageType(), DNotificationWidget::MessageType
     */
    void setMessageType(DNotificationWidget::MessageType type);

    /**
     * Show the widget using an animation.
     */
    void animatedShow();

    /**
     * Hide the widget using an animation.
     */
    void animatedHide();

    /**
     * Define an icon to be shown on the left of the text
     */
    void setIcon(const QIcon& icon);

Q_SIGNALS:

    /**
     * This signal is emitted when the user clicks a link in the text label.
     * The URL referred to by the href anchor is passed in contents.
     * @param contents text of the href anchor
     * @see QLabel::linkActivated()
     */
    void linkActivated(const QString& contents);

    /**
     * This signal is emitted when the user hovers over a link in the text label.
     * The URL referred to by the href anchor is passed in contents.
     * @param contents text of the href anchor
     * @see QLabel::linkHovered()
     */
    void linkHovered(const QString& contents);

    /**
     * This signal is emitted when the hide animation is finished, started by
     * calling animatedHide(). If animations are disabled, this signal is
     * emitted immediately after the message widget got hidden.
     *
     * @note This signal is @e not emitted if the widget was hidden by
     *       calling hide(), so this signal is only useful in conjunction
     *       with animatedHide().
     *
     * @see animatedHide()
     */
    void hideAnimationFinished();

    /**
     * This signal is emitted when the show animation is finished, started by
     * calling animatedShow(). If animations are disabled, this signal is
     * emitted immediately after the message widget got shown.
     *
     * @note This signal is @e not emitted if the widget was shown by
     *       calling show(), so this signal is only useful in conjunction
     *       with animatedShow().
     *
     * @see animatedShow()
     */
    void showAnimationFinished();

protected:

    void paintEvent(QPaintEvent* event)   Q_DECL_OVERRIDE;
    bool event(QEvent* event)             Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:

    class Private;
    Private* const d;

    friend class Private;
};

} // namespace Digikam

#endif // DNOTIFICATION_WIDGET_H
