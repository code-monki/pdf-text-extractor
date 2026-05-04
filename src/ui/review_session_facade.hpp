#pragma once

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <filesystem>
#include <memory>

namespace pte::ui {

class ReviewSessionFacade : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString sourcePdfPath READ sourcePdfPath NOTIFY sessionChanged)
    Q_PROPERTY(QString workFolderPath READ workFolderPath NOTIFY sessionChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY sessionChanged)
    Q_PROPERTY(QStringList pageIds READ pageIds NOTIFY sessionChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageId READ currentPageId NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageText READ currentPageText NOTIFY currentPageTextChanged)

public:
    explicit ReviewSessionFacade(QObject* parent = nullptr);

    QString statusMessage() const;
    QString sourcePdfPath() const;
    QString workFolderPath() const;
    int pageCount() const;
    QStringList pageIds() const;
    int currentPageIndex() const;
    QString currentPageId() const;
    QString currentPageText() const;
    QPixmap currentPreviewPixmap() const;

    Q_INVOKABLE bool openPdf(const QString& pdfPath, const QString& requestedWorkFolder);
    Q_INVOKABLE bool selectPage(int index);
    Q_INVOKABLE bool saveCurrentPageText(const QString& text);

signals:
    void statusMessageChanged();
    void sessionChanged();
    void currentPageChanged();
    void currentPageTextChanged();
    void previewPixmapChanged();

private:
    std::filesystem::path currentPagePath() const;
    bool reloadCurrentPageText();
    void refreshPreviewRaster();
    static std::string makeVolumeId(const std::string& filenameStem);
    static std::filesystem::path defaultWorkFolderForPdf(const std::filesystem::path& pdfPath);

    QString statusMessage_;
    QString sourcePdfPath_;
    QString workFolderPath_;
    QStringList pageIds_;
    int currentPageIndex_ = -1;
    QString currentPageText_;
    QPixmap previewPixmap_;
    std::unique_ptr<QTemporaryDir> previewTempDir_;
};

} // namespace pte::ui
