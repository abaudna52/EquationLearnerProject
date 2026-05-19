#pragma once

#include <QWidget>
#include <QJsonArray>
#include <vector>

// =============================================================================
// ExplanationWidget — блок-схема алгоритма, рисуется через QPainter
// =============================================================================

class ExplanationWidget : public QWidget {
    Q_OBJECT
public:
    explicit ExplanationWidget(QWidget* parent = nullptr);

    void setSteps(const QJsonArray& arr);
    void clearSteps();
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    struct Step { QString title, detail; };
    std::vector<Step> m_steps;
};
