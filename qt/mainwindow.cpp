#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "graphwidget.h"
#include "explanation.h"
#include "highlighter.h"
#include "llmclient.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_client(new LLMClient(this))
    , m_statusLabel(new QLabel("Готово. Введите уравнение и нажмите Анализировать."))
    , m_settings("EquationLearner", "App")
{
    m_ui->setupUi(this);

    // Добавляем кастомные виджеты программно (они не в .ui, чтобы не было зависимостей)
    m_graph = new GraphWidget(m_ui->graphTab);
    m_ui->graphLayout->addWidget(m_graph);

    m_explanation = new ExplanationWidget();
    m_ui->scrollArea->setWidget(m_explanation);

    // Подсветка синтаксиса C++
    m_ui->codeText->setFont(QFont("Monospace", 10));
    new CodeHighlighter(m_ui->codeText->document());

    // Статусная строка
    statusBar()->addWidget(m_statusLabel, 1);

    // Сигналы от AI-клиента
    connect(m_client, &LLMClient::ready,  this, &MainWindow::onReady);
    connect(m_client, &LLMClient::error,  this, &MainWindow::onError);
    connect(m_client, &LLMClient::status, this, &MainWindow::onStatus);

    // Сигналы от виджетов UI
    connect(m_ui->analyzeBtn,      &QPushButton::clicked,
            this, &MainWindow::onAnalyzeClicked);
    connect(m_ui->inputEdit,       &QLineEdit::returnPressed,
            this, &MainWindow::onAnalyzeClicked);
    connect(m_ui->keyEdit,         &QLineEdit::textChanged,
            this, &MainWindow::onKeyChanged);
    connect(m_ui->urlEdit,         &QLineEdit::textChanged,
            this, &MainWindow::onUrlChanged);
    connect(m_ui->modelEdit,       &QLineEdit::textChanged,
            this, &MainWindow::onModelChanged);
    connect(m_ui->clearHistoryBtn, &QPushButton::clicked,
            this, &MainWindow::onClearHistory);
    connect(m_ui->historyList,     &QListWidget::itemClicked,
            this, &MainWindow::onHistoryItemClicked);

    applyTheme();

    // Восстанавливаем сохранённые настройки
    QString savedKey = m_settings.value("apiKey").toString();
    if (!savedKey.isEmpty()) { m_ui->keyEdit->setText(savedKey);   m_client->setApiKey(savedKey);   }
    QString savedUrl = m_settings.value("apiUrl").toString();
    if (!savedUrl.isEmpty()) { m_ui->urlEdit->setText(savedUrl);   m_client->setApiUrl(savedUrl);   }
    QString savedModel = m_settings.value("model").toString();
    if (!savedModel.isEmpty()) { m_ui->modelEdit->setText(savedModel); m_client->setModel(savedModel); }

    loadHistory();
}

MainWindow::~MainWindow() { delete m_ui; }

// ---------- Слоты ----------

void MainWindow::onAnalyzeClicked() {
    QString input = m_ui->inputEdit->text().trimmed();
    if (input.isEmpty()) { m_statusLabel->setText("Введите уравнение."); return; }
    m_currentInput = input;
    m_ui->analyzeBtn->setEnabled(false);
    m_ui->analyzeBtn->setText("Анализирую...");
    m_ui->analysisText->clear();
    m_ui->codeText->clear();
    m_graph->clearGraph();
    m_explanation->clearSteps();
    m_ui->tabs->setCurrentIndex(0);
    m_client->analyze(input, m_ui->methodCombo->currentText());
}

void MainWindow::onReady(const QJsonObject& result) {
    m_ui->analyzeBtn->setEnabled(true);
    m_ui->analyzeBtn->setText("Анализировать");

    m_ui->analysisText->setPlainText(result["analysis"].toString());

    m_graph->setExpression(
        result["expression"].toString(),
        result["x_range_min"].toDouble(-10),
        result["x_range_max"].toDouble(10)
    );

    m_explanation->setSteps(result["explanation_steps"].toArray());
    m_ui->scrollArea->widget()->setMinimumHeight(m_explanation->sizeHint().height());

    m_ui->codeText->setPlainText(result["cpp_code"].toString());

    m_statusLabel->setText("Готово. Смотри все четыре вкладки.");

    if (!m_currentInput.isEmpty())
        addToHistory(m_currentInput);
}

void MainWindow::onError(const QString& msg) {
    m_ui->analyzeBtn->setEnabled(true);
    m_ui->analyzeBtn->setText("Анализировать");
    m_statusLabel->setText("Ошибка: " + msg);
    m_ui->analysisText->setPlainText("Ошибка: " + msg);
}

void MainWindow::onStatus(const QString& msg) { m_statusLabel->setText(msg); }

void MainWindow::onKeyChanged(const QString& key) {
    m_client->setApiKey(key.trimmed());
    m_settings.setValue("apiKey", key.trimmed());
}

void MainWindow::onUrlChanged(const QString& url) {
    m_client->setApiUrl(url.trimmed());
    m_settings.setValue("apiUrl", url.trimmed());
}

void MainWindow::onModelChanged(const QString& model) {
    m_client->setModel(model.trimmed());
    m_settings.setValue("model", model.trimmed());
}

void MainWindow::onHistoryItemClicked(QListWidgetItem* item) {
    QString eq = item->data(Qt::UserRole).toString();
    if (!eq.isEmpty()) {
        m_ui->inputEdit->setText(eq);
        m_ui->inputEdit->setFocus();
    }
}

void MainWindow::onClearHistory() {
    m_historyEquations.clear();
    m_ui->historyList->clear();
    m_settings.remove("history");
    m_statusLabel->setText("История очищена.");
}

// ---------- История ----------

void MainWindow::addToHistory(const QString& eq) {
    m_historyEquations.removeAll(eq);
    m_historyEquations.prepend(eq);
    while (m_historyEquations.size() > 50) m_historyEquations.removeLast();
    saveHistory();
    rebuildHistoryList();
}

void MainWindow::loadHistory() {
    QString raw = m_settings.value("history").toString();
    if (raw.isEmpty()) return;
    QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isArray()) return;
    m_historyEquations.clear();
    for (const auto& v : doc.array())
        m_historyEquations.append(v.toString());
    rebuildHistoryList();
}

void MainWindow::saveHistory() {
    QJsonArray arr;
    for (const auto& eq : m_historyEquations) arr.append(eq);
    m_settings.setValue("history",
        QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void MainWindow::rebuildHistoryList() {
    m_ui->historyList->clear();
    for (const auto& eq : m_historyEquations) {
        auto* item = new QListWidgetItem(eq);
        item->setData(Qt::UserRole, eq);
        item->setToolTip(eq);
        m_ui->historyList->addItem(item);
    }
}

// ---------- Тема ----------

void MainWindow::applyTheme() {
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(
        "QMainWindow, QWidget { background: #0f0f1a; color: #d0d0e8; font-size: 13px; }"
        "QGroupBox { border: 1px solid #2a2a42; border-radius: 6px; "
        "            margin-top: 6px; padding: 8px; color: #8080aa; }"
        "QLineEdit { background: #1a1a2e; border: 1px solid #33334a; "
        "            border-radius: 5px; padding: 4px 8px; color: #d0d0e8; }"
        "QLineEdit:focus { border: 1px solid #5555bb; }"
        "QPushButton { background: #3a3aaa; border: none; border-radius: 5px; "
        "              color: white; font-weight: bold; padding: 4px 8px; }"
        "QPushButton:hover    { background: #5050bb; }"
        "QPushButton:pressed  { background: #2a2a88; }"
        "QPushButton:disabled { background: #222238; color: #555570; }"
        "QTabWidget::pane { border: 1px solid #2a2a42; background: #0f0f1a; }"
        "QTabBar::tab { background: #181828; color: #666688; padding: 6px 16px; "
        "               border: 1px solid #2a2a42; border-bottom: none; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #1e1e32; color: #c0c0e0; "
        "                        border-bottom: 2px solid #5050bb; }"
        "QTextEdit { background: #1a1a2e; border: none; color: #d0d0e8; }"
        "QScrollBar:vertical { background: #0f0f1a; width: 8px; }"
        "QScrollBar::handle:vertical { background: #333355; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QStatusBar { background: #0a0a12; color: #606080; }"
        "QComboBox { background: #1a1a2e; border: 1px solid #33334a; border-radius: 5px; "
        "            padding: 4px 8px; color: #d0d0e8; }"
        "QComboBox:focus { border: 1px solid #5555bb; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox QAbstractItemView { background: #1a1a2e; color: #d0d0e8; "
        "                              border: 1px solid #33334a; "
        "                              selection-background-color: #3a3aaa; }"
        "QListWidget { background: #131322; border: 1px solid #2a2a42; "
        "              color: #b0b0cc; font-size: 12px; }"
        "QListWidget::item { padding: 5px 6px; border-bottom: 1px solid #1e1e30; }"
        "QListWidget::item:hover { background: #1e1e34; color: #d0d0f0; }"
        "QListWidget::item:selected { background: #2a2a5a; color: #e0e0ff; }"
    );
}
