/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to change font properties.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      1996 by Bernd Johannes Wuebben  <wuebben at kde dot org>
 * Copyright (c)      1999 by Preston Brown <pbrown at kde dot org>
 * Copyright (c)      1999 by Mario Weilguni <mweilguni at kde dot org>
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

#ifndef DFONT_PROPERTIES_H
#define DFONT_PROPERTIES_H

// Qt includes

#include <QWidget>
#include <QColor>
#include <QFont>
#include <QStringList>

// Local includes

#include "digikam_export.h"

class QFont;
class QStringList;

namespace Digikam
{

class DIGIKAM_EXPORT DFontProperties : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontSelected USER true)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(Qt::CheckState sizeIsRelative READ sizeIsRelative WRITE setSizeIsRelative)
    Q_PROPERTY(QString sampleText READ sampleText WRITE setSampleText)

public:

    /**
     *  @li @p FamilyList - Identifies the family (leftmost) list.
     *  @li @p StyleList  - Identifies the style (center) list.
     *  @li @p SizeList   - Identifies the size (rightmost) list.
     */
    enum FontColumn
    {
        FamilyList = 0x01,
        StyleList  = 0x02,
        SizeList   = 0x04
    };

    /**
     *  @li @p FontDiffFamily - Identifies a requested change in the font family.
     *  @li @p FontDiffStyle  - Identifies a requested change in the font style.
     *  @li @p FontDiffSize   - Identifies a requested change in the font size.
     */
    enum FontDiff
    {
        NoFontDiffFlags = 0,
        FontDiffFamily  = 1,
        FontDiffStyle   = 2,
        FontDiffSize    = 4,
        AllFontDiffs    = FontDiffFamily | FontDiffStyle | FontDiffSize
    };
    Q_DECLARE_FLAGS(FontDiffFlags, FontDiff)

    /**
     * @li @p FixedFontsOnly only show fixed fonts, excluding proportional fonts
     * @li @p DisplayFrame show a visual frame around the chooser
     * @li @p ShowDifferences display the font differences interfaces
     */
    enum DisplayFlag
    {
        NoDisplayFlags  = 0,
        FixedFontsOnly  = 1,
        DisplayFrame    = 2,
        ShowDifferences = 4
    };
    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    /**
     * The selection criteria for the font families shown in the dialog.
     *  @li @p FixedWidthFont when included only fixed-width fonts are returned.
     *        The fonts where the width of every character is equal.
     *  @li @p ScalableFont when included only scalable fonts are returned;
     *        certain configurations allow bitmap fonts to remain unscaled and
     *        thus these fonts have limited number of sizes.
     *  @li @p SmoothScalableFont when included only return smooth scalable fonts.
     *        this will return only non-bitmap fonts which are scalable to any size requested.
     *        Setting this option to true will mean the "scalable" flag is irrelavant.
     */
    enum FontListCriteria
    {
        FixedWidthFonts     = 0x01,
        ScalableFonts       = 0x02,
        SmoothScalableFonts = 0x04
    };

public:

    /**
     * Constructs a font picker widget.
     * It normally comes up with all font families present on the system; the
     * getFont method below does allow some more fine-tuning of the selection of fonts
     * that will be displayed in the dialog.
     *
     * @param parent The parent widget.
     * @param flags Defines how the font chooser is displayed. @see DisplayFlags
     * @param fontList A list of fonts to display, in XLFD format.
     * @param visibleListSize The minimum number of visible entries in the
     *        fontlists.
     * @param sizeIsRelativeState If not zero the widget will show a
     *        checkbox where the user may choose whether the font size
     *        is to be interpreted as relative size.
     *        Initial state of this checkbox will be set according to
     *        *sizeIsRelativeState, user choice may be retrieved by
     *        calling sizeIsRelative().
     */
    explicit DFontProperties(QWidget* const parent = 0,
                             const DisplayFlags& flags = DisplayFrame,
                             const QStringList& fontList = QStringList(),
                             int visibleListSize = 8,
                             Qt::CheckState* const sizeIsRelativeState = 0);

    /**
     * Destructs the font chooser.
     */
    virtual ~DFontProperties();

    /**
     * Enables or disable a font column in the chooser.
     *
     * Use this
     * function if your application does not need or supports all font
     * properties.
     *
     * @param column Specify the columns. An or'ed combination of
     *        @p FamilyList, @p StyleList and @p SizeList is possible.
     * @param state If @p false the columns are disabled.
     */
    void enableColumn(int column, bool state);

    /**
     * Makes a font column in the chooser visible or invisible.
     *
     * Use this
     * function if your application does not need to show all font
     * properties.
     *
     * @param column Specify the columns. An or'ed combination of
     *        @p FamilyList, @p StyleList and @p SizeList is possible.
     * @param state If @p false the columns are made invisible.
     */
    void makeColumnVisible(int column, bool state);

    /**
     * Sets the currently selected font in the chooser.
     *
     * @param font The font to select.
     * @param onlyFixed Readjust the font list to display only fixed
     *        width fonts if @p true, or vice-versa.
     */


    void setFont(const QFont& font, bool onlyFixed = false);

    /**
     * @return The bitmask corresponding to the attributes the user
     *         wishes to change.
     */
    FontDiffFlags fontDiffFlags() const;

    /**
     * @return The currently selected font in the chooser.
     */
    QFont font() const;

    /**
     * Sets the color to use in the preview.
     */
    void setColor(const QColor& col);

    /**
     * @return The color currently used in the preview (default: the text
     *         color of the active color group)
     */
    QColor color() const;

    /**
     * Sets the background color to use in the preview.
     */
    void setBackgroundColor(const QColor& col);

    /**
     * @return The background color currently used in the preview (default:
     *         the base color of the active colorgroup)
     */
    QColor backgroundColor() const;

    /**
     * Sets the state of the checkbox indicating whether the font size
     * is to be interpreted as relative size.
     * NOTE: If parameter sizeIsRelative was not set in the constructor
     *       of the widget this setting will be ignored.
     */
    void setSizeIsRelative(Qt::CheckState relative);

    /**
     * @return Whether the font size is to be interpreted as relative size
     *         (default: QButton:Off)
     */
    Qt::CheckState sizeIsRelative() const;

    /**
     * @return The current text in the sample text input area.
     */
    QString sampleText() const;

    /**
     * Sets the sample text.
     *
     * Normally you should not change this
     * text, but it can be better to do this if the default text is
     * too large for the edit area when using the default font of your
     * application.
     *
     * @param text The new sample text. The current will be removed.
     */
    void setSampleText(const QString& text);

    /**
     * Shows or hides the sample text box.
     *
     * @param visible Set it to true to show the box, to false to hide it.
     */
    void setSampleBoxVisible(bool visible);

    /**
     * Creates a list of font strings.
     *
     * @param list The list is returned here.
     * @param fontListCriteria should contain all the restrictions for font selection as OR-ed values
     *        @see DFontProperties::FontListCriteria for the individual values
     */
    static void getFontList(QStringList& list, uint fontListCriteria);

    /**
     * Reimplemented for internal reasons.
     */
    QSize sizeHint(void) const Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
     * Emitted whenever the selected font changes.
     */
    void fontSelected(const QFont& font);

private:

    class Private;
    Private* const d;

    Q_DISABLE_COPY(DFontProperties)

    Q_PRIVATE_SLOT(d, void _d_toggled_checkbox())
    Q_PRIVATE_SLOT(d, void _d_family_chosen_slot(const QString&))
    Q_PRIVATE_SLOT(d, void _d_size_chosen_slot(const QString&))
    Q_PRIVATE_SLOT(d, void _d_style_chosen_slot(const QString&))
    Q_PRIVATE_SLOT(d, void _d_displaySample(const QFont& font))
    Q_PRIVATE_SLOT(d, void _d_size_value_slot(double))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFontProperties::DisplayFlags)

} // namespace Digikam

#endif // DFONT_PROPERTIES_H
