/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef INSERTEXTTOOL_H
#define INSERTEXTTOOL_H

// Qt includes.

#include <QColor>
#include <QImage>

// Digikam includes.

#include "editortool.h"

class QFont;
class QButtonGroup;
class QComboBox;
class QCheckBox;

class KTextEdit;
class KColorButton;
class KFontChooser;

namespace DigikamInsertTextImagesPlugin
{

class InsertTextWidget;

class InsertTextTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    InsertTextTool(QObject *parent);
    ~InsertTextTool();

signals:

    void signalUpdatePreview();

private slots:

    void slotResetSettings();
    void slotFontPropertiesChanged(const QFont &font);
    void slotUpdatePreview();
    void slotAlignModeChanged(int mode);

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private:

    int                          m_alignTextMode;
    int                          m_defaultSizeFont;

    QComboBox                   *m_textRotation;

    QCheckBox                   *m_borderText;
    QCheckBox                   *m_transparentText;

    QButtonGroup                *m_alignButtonGroup;

    QFont                        m_textFont;

    KColorButton                *m_fontColorButton;

    KFontChooser                *m_fontChooserWidget;

    KTextEdit                   *m_textEdit;

    InsertTextWidget            *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamInsertTextImagesPlugin

#endif /* INSERTEXTTOOL_H */
