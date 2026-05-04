/**
 * @file app_theme.cpp
 * @brief Implements Fusion palettes, QSettings persistence, and WCAG-style contrast hints for pte_shell.
 */

#include "ui/app_theme.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleFactory>

#include <algorithm>
#include <cmath>

namespace pte::ui {

namespace {

constexpr char kSettingsGroupUi[] = "ui";
constexpr char kSettingsKeyTheme[] = "theme";

double linearizeChannel(const double channelSrgb) {
    const double c = channelSrgb / 255.0;
    return (c <= 0.03928) ? (c / 12.92) : std::pow((c + 0.055) / 1.055, 2.4);
}

double relativeLuminance(const QColor& c) {
    return 0.2126 * linearizeChannel(static_cast<double>(c.red()))
        + 0.7152 * linearizeChannel(static_cast<double>(c.green()))
        + 0.0722 * linearizeChannel(static_cast<double>(c.blue()));
}

double contrastRatio(const QColor& a, const QColor& b) {
    const double la = relativeLuminance(a);
    const double lb = relativeLuminance(b);
    const double lighter = std::max(la, lb);
    const double darker = std::min(la, lb);
    return (lighter + 0.05) / (darker + 0.05);
}

void setStandardChrome(QPalette& p, const QColor& window, const QColor& base,
                       const QColor& alternateBase, const QColor& text,
                       const QColor& button, const QColor& buttonText, const QColor& highlight,
                       const QColor& highlightedText, const QColor& tipBg, const QColor& tipText) {
    p.setColor(QPalette::Window, window);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, base);
    p.setColor(QPalette::AlternateBase, alternateBase);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, button);
    p.setColor(QPalette::ButtonText, buttonText);
    p.setColor(QPalette::Highlight, highlight);
    p.setColor(QPalette::HighlightedText, highlightedText);
    p.setColor(QPalette::ToolTipBase, tipBg);
    p.setColor(QPalette::ToolTipText, tipText);
    p.setColor(QPalette::PlaceholderText, text);
    p.setColor(QPalette::BrightText, highlightedText);
}

QPalette lightPalette() {
    QPalette p;
    const QColor window(239, 239, 239);
    const QColor base(255, 255, 255);
    const QColor altBase(247, 247, 247);
    const QColor text(0, 0, 0);
    const QColor button(225, 225, 225);
    const QColor highlight(42, 130, 218);
    const QColor highlightedText(255, 255, 255);
    setStandardChrome(p, window, base, altBase, text, button, text, highlight, highlightedText, base,
                      text);
    return p;
}

QPalette darkPalette() {
    QPalette p;
    const QColor window(45, 45, 48);
    const QColor base(30, 30, 30);
    const QColor altBase(42, 42, 45);
    const QColor text(224, 224, 224);
    const QColor button(55, 55, 58);
    const QColor highlight(61, 174, 233);
    const QColor highlightedText(255, 255, 255);
    const QColor tipBg(60, 60, 63);
    const QColor tipText(240, 240, 240);
    setStandardChrome(p, window, base, altBase, text, button, text, highlight, highlightedText, tipBg,
                      tipText);
    return p;
}

QPalette sepiaPalette() {
    QPalette p;
    const QColor window(242, 232, 213);
    const QColor base(250, 243, 230);
    const QColor altBase(242, 236, 220);
    const QColor text(45, 36, 26);
    const QColor button(232, 220, 198);
    const QColor highlight(139, 105, 20);
    const QColor highlightedText(255, 252, 245);
    const QColor tipBg(255, 248, 235);
    setStandardChrome(p, window, base, altBase, text, button, text, highlight, highlightedText, tipBg,
                      text);
    return p;
}

bool contrastOk(const QColor& fg, const QColor& bg, double minimum) {
    return contrastRatio(fg, bg) >= minimum;
}

} // namespace

QString themeIdLight() {
    return QStringLiteral("builtin.light");
}

QString themeIdDark() {
    return QStringLiteral("builtin.dark");
}

QString themeIdSepia() {
    return QStringLiteral("builtin.sepia");
}

QVector<ThemeDescriptor> listBuiltInThemes() {
    return {
        {themeIdLight(),
         QCoreApplication::translate("AppTheme", "Light (default)"),
         QStringLiteral("light")},
        {themeIdDark(), QCoreApplication::translate("AppTheme", "Dark"), QStringLiteral("dark")},
        {themeIdSepia(),
         QCoreApplication::translate("AppTheme", "Warm sepia"),
         QStringLiteral("sepia")},
    };
}

QString defaultThemeId() {
    return themeIdLight();
}

QString loadSavedThemeId() {
    QSettings settings;
    settings.beginGroup(QLatin1String(kSettingsGroupUi));
    const QString id = settings.value(QLatin1String(kSettingsKeyTheme), defaultThemeId()).toString();
    settings.endGroup();
    if (!isKnownBuiltInThemeId(id)) {
        return defaultThemeId();
    }
    return id;
}

void saveThemeId(const QString& themeId) {
    QSettings settings;
    settings.beginGroup(QLatin1String(kSettingsGroupUi));
    settings.setValue(QLatin1String(kSettingsKeyTheme), themeId);
    settings.endGroup();
}

bool isKnownBuiltInThemeId(const QString& themeId) {
    return themeId == themeIdLight() || themeId == themeIdDark() || themeId == themeIdSepia();
}

void applyTheme(QApplication* app, const QString& themeId) {
    if (app == nullptr) {
        return;
    }
    app->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QString effective = themeId;
    if (!isKnownBuiltInThemeId(effective)) {
        effective = defaultThemeId();
    }

    QPalette pal;
    if (effective == themeIdDark()) {
        pal = darkPalette();
    } else if (effective == themeIdSepia()) {
        pal = sepiaPalette();
    } else {
        pal = lightPalette();
    }

    app->setPalette(pal);
}

void applySavedThemeOrDefault(QApplication* app) {
    applyTheme(app, loadSavedThemeId());
}

QString validateBuiltInThemesContrastHint() {
    struct Pair {
        QPalette palette;
        const char* label;
    };
    const Pair pairs[] = {
        {lightPalette(), "light"},
        {darkPalette(), "dark"},
        {sepiaPalette(), "sepia"},
    };
    QString issues;
    constexpr double kMinNormal = 4.5;
    constexpr double kMinLarge = 3.0;
    for (const Pair& pair : pairs) {
        const QColor wt = pair.palette.color(QPalette::WindowText);
        const QColor win = pair.palette.color(QPalette::Window);
        const QColor tx = pair.palette.color(QPalette::Text);
        const QColor bs = pair.palette.color(QPalette::Base);
        const QColor ht = pair.palette.color(QPalette::HighlightedText);
        const QColor hi = pair.palette.color(QPalette::Highlight);
        if (!contrastOk(wt, win, kMinLarge)) {
            issues += QStringLiteral("%1: window text vs window below large-text threshold; ")
                          .arg(QLatin1String(pair.label));
        }
        if (!contrastOk(tx, bs, kMinNormal)) {
            issues += QStringLiteral("%1: text vs base below normal threshold; ").arg(QLatin1String(pair.label));
        }
        if (!contrastOk(ht, hi, kMinNormal)) {
            issues += QStringLiteral("%1: highlight contrast marginal; ").arg(QLatin1String(pair.label));
        }
    }
    if (issues.isEmpty()) {
        return QStringLiteral("built-in theme contrast checks OK (WCAG-style ratios)");
    }
    issues.chop(2);
    return issues;
}

} // namespace pte::ui
