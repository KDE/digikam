/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012 Dodon Victor <dodonvictor at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMAGESHACK_WIDGET_H
#define IMAGESHACK_WIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "dprogresswdg.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"

class QRadioButton;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QGroupBox;
class QComboBox;
class QPushButton;

namespace Digikam
{

class ImageShackSession;

class ImageShackWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit ImageShackWidget(QWidget* const parent,
                              ImageShackSession* const session,
                              DInfoInterface* const iface,
                              const QString& toolName);
    ~ImageShackWidget();

Q_SIGNALS:

    void signalReloadGalleries();

private:

    void updateLabels(const QString& name = QString(), const QString& url = QString()) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotGetGalleries(const QStringList& gTexts, const QStringList& gNames);
    void slotReloadGalleries();

private:

    DImagesList*                   m_imgList;
    DInfoInterface*                m_iface;
    ImageShackSession*             m_session;

    QLabel*                        m_headerLbl;
    QLabel*                        m_accountNameLbl;

    QLineEdit*                     m_tagsFld;

    QCheckBox*                     m_privateImagesChb;
    QCheckBox*                     m_remBarChb;

    QPushButton*                   m_chgRegCodeBtn;
    QPushButton*                   m_reloadGalleriesBtn;

    QComboBox*                     m_galleriesCob;

    DProgressWdg*                  m_progressBar;
/*
    QLabel*                        m_accountEmailLbl;
    QLineEdit*                     m_newGalleryName;
    QRadioButton*                  m_noResizeRdb;
    QRadioButton*                  m_predefSizeRdb;
    QRadioButton*                  m_customSizeRdb;
    QCheckBox*                     m_useGalleriesChb;
    QComboBox*                     m_resizeOptsCob;
    QSpinBox*                      m_widthSpb;
    QSpinBox*                      m_heightSpb;
    QGroupBox*                     m_galleriesBox;
    QWidget*                       m_galleriesWidget;
*/
    friend class ImageShackWindow;
};

}  // namespace Digikam

#endif // IMAGESHACK_WIDGET_H
