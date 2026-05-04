/**
 * @file main.cpp
 * @brief Qt Widgets entry point for `pte_shell` (FR-028, NFR-009).
 *
 * @details Applies saved theme, optional contrast sanity check in debug builds, system UI font,
 *          and displays ShellMainWindow. Domain logic is not started here beyond QApplication setup.
 *
 * @see ShellMainWindow
 * @see pte::ui::applySavedThemeOrDefault
 */

#include <QApplication>
#include <QFontDatabase>
#include <QtGlobal>

#include "ui/app_theme.hpp"
#include "ui/shell_main_window.hpp"

int main(int argc, char* argv[]) {
#if defined(Q_OS_MACOS)
    // Native macOS file sheets can return immediately without UI when activation /
    // bundle context is wrong for a QtWidgets app. Force Qt-drawn dialogs for reliability.
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
#endif
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PDF Text Extractor"));
    QApplication::setOrganizationName(QStringLiteral("pdf-text-extractor"));

    // Use the platform UI font (e.g. San Francisco on macOS) instead of Fusion’s generic default.
    QApplication::setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    pte::ui::applySavedThemeOrDefault(&app);
#if defined(QT_DEBUG)
    {
        const QString contrast = pte::ui::validateBuiltInThemesContrastHint();
        if (!contrast.contains(QLatin1String("OK"), Qt::CaseInsensitive)) {
            qWarning("%s", qPrintable(contrast));
        }
    }
#endif

    ShellMainWindow window;
    window.show();
    return app.exec();
}
