#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// =============================================================================
// LLMClient — универсальный клиент для любого OpenAI-совместимого API
// Работает с DeepSeek, OpenRouter, OpenAI, freetheai и другими.
// =============================================================================

class LLMClient : public QObject {
    Q_OBJECT
public:
    explicit LLMClient(QObject* parent = nullptr);

    void setApiKey(const QString& key);
    void setApiUrl(const QString& url);
    void setModel (const QString& model);

    void analyze(const QString& input, const QString& method = QString());

signals:
    void ready(const QJsonObject& result);
    void error(const QString& msg);
    void status(const QString& msg);

private slots:
    void onFinished(QNetworkReply* reply);

private:
    QString buildPrompt(const QString& input, const QString& method) const;
    static QString fixJsonStrings(const QString& s);
    QJsonObject extractJSON(const QString& raw) const;

    QNetworkAccessManager* m_manager;
    QString m_apiKey;
    QString m_apiUrl;
    QString m_model;
};
