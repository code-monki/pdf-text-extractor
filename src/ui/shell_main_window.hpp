#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <memory>

#include "ui/review_session_facade.hpp"

class QAction;
class QLabel;
class QListWidget;
class QResizeEvent;
class QTextEdit;

/**
 * Minimal reviewer shell using Qt Widgets (native dialogs, same ReviewSessionFacade).
 */
class ShellMainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit ShellMainWindow(QWidget* parent = nullptr);
    ~ShellMainWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onOpenPdf();
    void onSavePageText();
    void onPageListRowChanged(int row);
    void onPrevPage();
    void onNextPage();
    void refreshSessionUi();
    void refreshPageUi();
    void refreshPageText();
    void refreshStatus();
    void refreshPreviewColumn();

private:
    void buildUi();
    void wireFacade();
    void applyPreviewScale();

    std::unique_ptr<pte::ui::ReviewSessionFacade> facade_;
    QListWidget* pageList_{};
    QLabel* preview_{};
    QTextEdit* pageText_{};
    QLabel* pageLabel_{};
    QAction* savePageAction_{};
    QPixmap previewSource_;
};
