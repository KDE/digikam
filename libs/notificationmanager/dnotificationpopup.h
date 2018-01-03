/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-03
 * Description : dialog-like popup that displays messages without interrupting the user
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2001-2006 by Richard Moore <rich at kde dot org>
 * Copyright (C) 2004-2005 by Sascha Cunz <sascha.cunz at tiscali dot de>
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

#ifndef DNOTIFICATION_POPUP_H
#define DNOTIFICATION_POPUP_H

// Qt includes

#include <QFrame>

// Local includes

#include "digikam_export.h"

class QSystemTrayIcon;

namespace Digikam
{

/**
 * @short A dialog-like popup that displays messages without interrupting the user.
 *
 * The simplest uses of DNotificationPopup are by using the various message() static
 * methods. The position the popup appears at depends on the type of the parent window:
 *
 */
class DIGIKAM_EXPORT DNotificationPopup : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool autoDelete READ autoDelete WRITE setAutoDelete)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)

public:

    /**
     * Styles that a DNotificationPopup can have.
     */
    enum PopupStyle
    {
        Boxed,             ///< Information will appear in a framed box (default)
        Balloon,           ///< Information will appear in a comic-alike balloon
    };

public:

    /**
     * Creates a popup for the specified widget.
     */
    explicit DNotificationPopup(QWidget* parent = 0, Qt::WindowFlags f = 0);

    /**
     * Creates a popup for the specified window.
     */
    explicit DNotificationPopup(WId parent);

    /**
     * Cleans up.
     */
    virtual ~DNotificationPopup();

    /**
     * Sets the main view to be the specified widget (which must be a child of the popup).
     */
    void setView(QWidget* child);

    /**
     * Creates a standard view then calls setView(QWidget*) .
     */
    void setView(const QString& caption, const QString& text = QString());

    /**
     * Creates a standard view then calls setView(QWidget*) .
     */
    virtual void setView(const QString& caption, const QString& text, const QPixmap& icon);

    /**
     * Returns a widget that is used as standard view if one of the
     * setView() methods taking the QString arguments is used.
     * You can use the returned widget to customize the passivepopup while
     * keeping the look similar to the "standard" passivepopups.
     *
     * After customizing the widget, pass it to setView( QWidget* )
     *
     * @param caption The window caption (title) on the popup
     * @param text The text for the popup
     * @param icon The icon to use for the popup
     * @param parent The parent widget used for the returned widget. If left 0,
     * then "this", i.e. the passive popup object will be used.
     *
     * @return a QWidget containing the given arguments, looking like the
     * standard passivepopups. The returned widget contains a QVBoxLayout,
     * which is accessible through layout().
     * @see setView( QWidget * )
     * @see setView( const QString&, const QString& )
     * @see setView( const QString&, const QString&, const QPixmap& )
     */
    QWidget* standardView(const QString& caption, const QString& text,
                          const QPixmap& icon, QWidget* parent = 0L);

    /**
     * Returns the main view.
     */
    QWidget* view() const;

    /**
     * Returns the delay before the popup is removed automatically.
     */
    int timeout() const;

    /**
     * Sets whether the popup will be deleted when it is hidden.
     *
     * The default is false (unless created by one of the static
     * message() overloads).
     */
    virtual void setAutoDelete(bool autoDelete);

    /**
     * Returns whether the popup will be deleted when it is hidden.
     *
     * @see setAutoDelete
     */
    bool autoDelete() const;

    /**
     * Returns the position to which this popup is anchored.
     */
    QPoint anchor() const;

    /**
     * Sets the anchor of this popup.
     *
     * The popup is placed near to the anchor.
     */
    void setAnchor(const QPoint& anchor);

    /**
     * Convenience method that displays popup with the specified  message  beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& text, QWidget* parent,
                                       const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified  message  beside the
     * icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& text, QSystemTrayIcon* parent);

    /**
     * Convenience method that displays popup with the specified caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& caption, const QString& text,
                                       QWidget* parent, const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified caption and message
     * beside the icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& caption, const QString& text,
                                       QSystemTrayIcon* parent);

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& caption, const QString& text,
                                       const QPixmap& icon, QWidget* parent, int timeout = -1,
                                       const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& caption, const QString& text,
                                       const QPixmap& icon, QSystemTrayIcon* parent, int timeout = -1);

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(const QString& caption, const QString& text,
                                       const QPixmap& icon, WId parent,
                                       int timeout = -1, const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified popup-style and message beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& text, QWidget* parent, const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified popup-style and message beside the
     * icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& text, QSystemTrayIcon* parent);

    /**
     * Convenience method that displays popup with the specified popup-style, caption and message
     * beside the icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& caption, const QString& text,
                                       QSystemTrayIcon* parent);

    /**
     * Convenience method that displays popup with the specified popup-style, caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& caption, const QString& text,
                                       QWidget* parent, const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& caption, const QString& text,
                                       const QPixmap& icon, QWidget* parent, int timeout = -1,
                                       const QPoint& p = QPoint());

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified QSystemTrayIcon.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& caption, const QString& text,
                                       const QPixmap& icon, QSystemTrayIcon* parent, int timeout = -1);

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static DNotificationPopup* message(int popupStyle, const QString& caption, const QString& text,
                                       const QPixmap& icon, WId parent, int timeout = -1,
                                       const QPoint& p = QPoint());

    // we create an overloaded version of show()
    using QFrame::show;

public Q_SLOTS:

    /**
     * Sets the delay for the popup is removed automatically. Setting the delay to 0
     * disables the timeout, if you're doing this, you may want to connect the
     * clicked() signal to the hide() slot.
     * Setting the delay to -1 makes it use the default value.
     *
     * @see timeout
     */
    void setTimeout(int delay);

    /**
     * Sets the visual appearance of the popup.
     * @see PopupStyle
     */
    void setPopupStyle(int popupstyle);

    /**
     * Shows the popup in the given point
     */
    void show(const QPoint& p);

    /** @reimp */
    void setVisible(bool visible) Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
     * Emitted when the popup is clicked.
     */
    void clicked();

    /**
     * Emitted when the popup is clicked.
     */
    void clicked(const QPoint& pos);

protected:

    /**
     * Positions the popup.
     *
     * The default implementation attempts to place it by the taskbar
     * entry; failing that it places it by the window of the associated
     * widget; failing that it places it at the location given by
     * defaultLocation().
     *
     * @see moveNear()
     */
    virtual void positionSelf();

    /**
     * Returns a default location for popups when a better placement
     * cannot be found.
     *
     * The default implementation returns the top-left corner of the
     * available work area of the desktop (ie: minus panels, etc).
     */
    virtual QPoint defaultLocation() const;

    /**
     * Moves the popup to be adjacent to @p target.
     *
     * The popup will be placed adjacent to, but outside of, @p target,
     * without going off the current desktop.
     *
     * Reimplementations of positionSelf() can use this to actually
     * position the popup.
     */
    void moveNear(const QRect& target);

    /** @reimp */
    void hideEvent(QHideEvent*) Q_DECL_OVERRIDE;

    /** @reimp */
    void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;

    /** @reimp */
    void paintEvent(QPaintEvent* pe) Q_DECL_OVERRIDE;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DNOTIFICATION_POPUP_H
