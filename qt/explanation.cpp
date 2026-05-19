#include "explanation.h"

#include <QPainter>
#include <QJsonObject>
#include <QFont>
#include <QFontMetrics>
#include <QPolygon>
#include <algorithm>
using namespace std;

ExplanationWidget::ExplanationWidget(QWidget* parent) : QWidget(parent) {}

void ExplanationWidget::setSteps(const QJsonArray& arr) {
    m_steps.clear();
    for (int i = 0; i < arr.size(); i++) {
        auto obj = arr[i].toObject();
        m_steps.push_back(Step{ obj["title"].toString(), obj["detail"].toString() });
    }
    updateGeometry(); update();
}

void ExplanationWidget::clearSteps() { m_steps.clear(); update(); }

QSize ExplanationWidget::sizeHint() const {
    if (m_steps.empty()) return { 600, 300 };
    QFontMetrics fm(QFont("Monospace", 9));
    int h = 20;
    for (auto& s : m_steps)
        h += max(80, fm.boundingRect(QRect(0,0,540,9999),
                     Qt::TextWordWrap, s.detail).height() + 56) + 44;
    return { 640, h };
}

void ExplanationWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(0x0e, 0x0e, 0x18));

    if (m_steps.empty()) {
        p.setPen(QColor(120, 120, 170));
        p.setFont(QFont("Monospace", 11));
        p.drawText(rect(), Qt::AlignCenter, "Схема алгоритма появится после анализа");
        return;
    }

    QColor bg[]     = { QColor(0x1e,0x3a,0x5f), QColor(0x1e,0x4f,0x35),
                         QColor(0x4f,0x2e,0x1e), QColor(0x3b,0x1e,0x4f) };
    QColor border[] = { QColor(0x4a,0x9e,0xff), QColor(0x4a,0xd4,0x7f),
                         QColor(0xff,0x9e,0x4a), QColor(0xb0,0x4a,0xff) };

    int cx = width()/2, boxW = min(560, width()-80), y = 16;
    QFont titleF("Monospace", 10, QFont::Bold), detailF("Monospace", 9);
    QFontMetrics fm(detailF);

    for (int i = 0; i < (int)m_steps.size(); i++) {
        int ci = i % 4;
        int boxH = max(80, fm.boundingRect(QRect(0,0,boxW-32,9999),
                            Qt::TextWordWrap, m_steps[i].detail).height() + 56);
        QRect box(cx - boxW/2, y, boxW, boxH);

        p.setPen(Qt::NoPen); p.setBrush(QColor(0,0,0,60));
        p.drawRoundedRect(box.adjusted(4,4,4,4), 10, 10);
        p.setBrush(bg[ci]); p.setPen(QPen(border[ci], 2));
        p.drawRoundedRect(box, 10, 10);

        QRect circle(box.left()-28, y+boxH/2-14, 28, 28);
        p.setBrush(border[ci]); p.setPen(Qt::NoPen);
        p.drawEllipse(circle);
        p.setPen(QColor(0x0e,0x0e,0x18));
        p.setFont(QFont("Monospace", 10, QFont::Bold));
        p.drawText(circle, Qt::AlignCenter, QString::number(i+1));

        p.setPen(border[ci]); p.setFont(titleF);
        p.drawText(box.adjusted(16,12,-16,0), Qt::AlignTop, m_steps[i].title);
        p.setPen(QColor(210,210,230)); p.setFont(detailF);
        p.drawText(box.adjusted(16,36,-16,-12), Qt::TextWordWrap|Qt::AlignTop, m_steps[i].detail);

        y += boxH;
        if (i+1 < (int)m_steps.size()) {
            p.setPen(QPen(QColor(100,100,150), 2));
            p.drawLine(cx, y, cx, y+30);
            QPolygon arr;
            arr << QPoint(cx,y+44) << QPoint(cx-9,y+30) << QPoint(cx+9,y+30);
            p.setBrush(QColor(100,100,150)); p.setPen(Qt::NoPen);
            p.drawPolygon(arr);
            y += 44;
        }
    }
}
