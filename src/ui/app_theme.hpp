/**
 * @file app_theme.hpp
 * @brief Built-in Qt Widgets themes and palette application (DD §4.17, FR-033, NFR-009, NFR-016).
 *
 * @note Does not read or write work-folder or extraction artifacts — user prefs only.
 */

#pragma once

#include <QString>
#include <QVector>

class QApplication;

namespace pte::ui {

struct ThemeDescriptor {
    QString themeId;
    QString displayName;
    /** DD theme descriptor: light | dark | high-contrast | sepia | system */
    QString baseMode;
};

/** Stable IDs for built-in themes (persisted in QSettings). */
QString themeIdLight();
QString themeIdDark();
QString themeIdSepia();

QVector<ThemeDescriptor> listBuiltInThemes();

QString defaultThemeId();

QString loadSavedThemeId();

void saveThemeId(const QString& themeId);

bool isKnownBuiltInThemeId(const QString& themeId);

/**
 * @brief Applies Fusion + palette for the given built-in theme id.
 *
 * @post PDF preview pixmap colors unchanged except as inherited from widget chrome around them.
 */
void applyTheme(QApplication* app, const QString& themeId);

/** Loads saved theme or default when missing/invalid. */
void applySavedThemeOrDefault(QApplication* app);

/**
 * @brief Checks text/background pairs for WCAG-style contrast (NFR-016 sanity).
 *
 * @return Safe summary line for logs/status; never includes user content.
 */
QString validateBuiltInThemesContrastHint();

} // namespace pte::ui
