#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QStringList>

class QLabel;
class QListWidgetItem;
class GraphWidget;
class ExplanationWidget;
class LLMClient;

namespace Ui { class MainWindow; }

// =============================================================================
// MainWindow — главное окно приложения
// Визуальная структура описана в qt/mainwindow.ui
// =============================================================================

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAnalyzeClicked();
    void onReady(const QJsonObject& result);
    void onError(const QString& msg);
    void onStatus(const QString& msg);
    void onKeyChanged(const QString& key);
    void onUrlChanged(const QString& url);
    void onModelChanged(const QString& model);
    void onHistoryItemClicked(QListWidgetItem* item);
    void onClearHistory();

private:
    void applyTheme();
    void addToHistory(const QString& eq);
    void loadHistory();
    void saveHistory();
    void rebuildHistoryList();

    Ui::MainWindow*    m_ui;
    GraphWidget*       m_graph       = nullptr;
    ExplanationWidget* m_explanation = nullptr;
    LLMClient*         m_client      = nullptr;
    QLabel*            m_statusLabel = nullptr;
    QSettings          m_settings;
    QString            m_currentInput;
    QStringList        m_historyEquations;
};
