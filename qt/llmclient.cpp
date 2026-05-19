#include "llmclient.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>

LLMClient::LLMClient(QObject* parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &LLMClient::onFinished);
}

void LLMClient::setApiKey(const QString& key)   { m_apiKey = key; }
void LLMClient::setApiUrl(const QString& url)   { m_apiUrl = url; }
void LLMClient::setModel (const QString& model) { m_model  = model; }

void LLMClient::analyze(const QString& input, const QString& method) {
    if (m_apiKey.isEmpty()) { emit error("API-ключ не задан."); return; }

    QString url   = m_apiUrl.trimmed();
    QString model = m_model.trimmed();
    if (url.isEmpty())   url   = "https://api.deepseek.com/v1/chat/completions";
    if (model.isEmpty()) model = "deepseek-chat";

    emit status(QString("Отправляем запрос (%1)...").arg(model));

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QJsonObject sys, usr, body;
    sys["role"]    = "system";
    sys["content"] = "Ты помощник по математике и C++. "
                     "Отвечай ТОЛЬКО одним валидным JSON-объектом без markdown и без ```json.";
    usr["role"]    = "user";
    usr["content"] = buildPrompt(input, method);

    body["model"]       = model;
    body["temperature"] = 0.2;
    body["max_tokens"]  = 3000;
    body["messages"]    = QJsonArray{ sys, usr };

    m_manager->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
}

void LLMClient::onFinished(QNetworkReply* reply) {
    reply->deleteLater();

    QByteArray    body       = reply->readAll();
    int           httpStatus = reply->attribute(
                                   QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QJsonDocument doc        = QJsonDocument::fromJson(body);

    if (reply->error() != QNetworkReply::NoError) {
        QString detail;
        if (doc.isObject() && doc.object().contains("error"))
            detail = doc.object()["error"].toObject()["message"].toString();
        else
            detail = reply->errorString();
        emit error(QString("Ошибка [HTTP %1]: %2").arg(httpStatus).arg(detail));
        return;
    }

    emit status("Разбираем ответ...");

    if (!doc.isObject()) {
        emit error("Сервер вернул не JSON: " + QString::fromUtf8(body.left(300)));
        return;
    }

    QJsonObject root = doc.object();
    if (root.contains("error")) {
        emit error(QString("Ошибка API [HTTP %1]: %2")
                       .arg(httpStatus)
                       .arg(root["error"].toObject()["message"].toString()));
        return;
    }

    QString content = root["choices"].toArray()
                          .first().toObject()
                          ["message"].toObject()
                          ["content"].toString();

    if (content.isEmpty()) {
        emit error("Модель вернула пустой ответ. Проверь название модели и баланс.");
        return;
    }

    emit status("Готово.");
    emit ready(extractJSON(content));
}

QString LLMClient::buildPrompt(const QString& input, const QString& method) const {
    QString methodHint;
    if (method.isEmpty() || method == "Авто (AI выбирает)") {
        methodHint = "Выбери наиболее подходящий численный или аналитический метод решения самостоятельно.";
    } else {
        methodHint = QString("Используй %1 для решения и объяснения.").arg(method);
    }

    return QString(
        "Пользователь ввёл уравнение: \"%1\"\n\n"
        "Это нелинейное уравнение f(x)=0. %2\n\n"
        "Верни ТОЛЬКО JSON-объект:\n"
        "{\n"
        "  \"analysis\": \"Подробный анализ: тип уравнения, свойства, корни, выбранный метод.\",\n"
        "  \"expression\": \"f(x) как строка. Переменная x, ^ для степени. Пример: x^3-2*x+1\",\n"
        "  \"x_range_min\": -10,\n"
        "  \"x_range_max\": 10,\n"
        "  \"explanation_steps\": [\n"
        "    {\"title\":\"Название шага\",\"detail\":\"Объяснение\"}\n"
        "  ],\n"
        "  \"cpp_code\": \"Полная программа на C++ с комментариями.\"\n"
        "}\n"
        "explanation_steps: 4-6 шагов, объясни метод пошагово. Только сырой JSON без markdown и без ```."
    ).arg(input).arg(methodHint);
}

// Заменяет буквальные \n \r \t внутри JSON-строк на экранированные версии.
QString LLMClient::fixJsonStrings(const QString& s) {
    QString r;
    r.reserve(s.size() + 128);
    bool inStr = false, esc = false;
    for (QChar c : s) {
        if (esc)               { r += c; esc = false; continue; }
        if (c == '\\' && inStr){ r += c; esc = true;  continue; }
        if (c == '"')          { inStr = !inStr; r += c; continue; }
        if (inStr) {
            if      (c == '\n') { r += "\\n";  continue; }
            else if (c == '\r') { r += "\\r";  continue; }
            else if (c == '\t') { r += "\\t";  continue; }
        }
        r += c;
    }
    return r;
}

QJsonObject LLMClient::extractJSON(const QString& raw) const {
    QString s = raw.trimmed();

    // Вытаскиваем содержимое из ```json ... ``` если модель обернула
    QRegularExpression mdFence(R"(```(?:json)?\s*([\s\S]*?)```)",
                               QRegularExpression::CaseInsensitiveOption);
    auto m = mdFence.match(s);
    if (m.hasMatch()) s = m.captured(1).trimmed();

    // Находим границы JSON-объекта
    int a = s.indexOf('{'), b = s.lastIndexOf('}');
    if (a != -1 && b > a) s = s.mid(a, b - a + 1);

    // Первая попытка — парсим как есть
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (doc.isObject()) return doc.object();

    // Вторая попытка — фиксируем неэкранированные символы
    doc = QJsonDocument::fromJson(fixJsonStrings(s).toUtf8(), &err);
    if (doc.isObject()) return doc.object();

    // Заглушка при полном провале
    QJsonObject fb;
    fb["analysis"]          = "Не удалось разобрать ответ:\n" + raw;
    fb["expression"]        = "x";
    fb["x_range_min"]       = -10;
    fb["x_range_max"]       = 10;
    fb["explanation_steps"] = QJsonArray();
    fb["cpp_code"]          = "// Ошибка разбора ответа";
    return fb;
}
