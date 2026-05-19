#include "graphwidget.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QMatrix4x4>
#include <QFont>
#include <cmath>
#include <algorithm>
using namespace std;

GraphWidget::GraphWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMinimumSize(400, 300);
}

GraphWidget::~GraphWidget() {
    makeCurrent();
    m_vbo.destroy();
    m_vao.destroy();
    delete m_shader;
    doneCurrent();
}

void GraphWidget::setExpression(const QString& expr, double xMin, double xMax) {
    m_eval.setExpression(expr.toStdString());
    m_hasGraph = m_eval.isValid();
    m_xMin = xMin; m_xMax = xMax;

    if (m_hasGraph) {
        double yMin =  1e18, yMax = -1e18;
        for (int i = 0; i <= 500; i++) {
            double x = m_xMin + (m_xMax - m_xMin) * i / 500.0;
            try {
                double y = m_eval.evaluate(x);
                if (isfinite(y)) { yMin = min(yMin, y); yMax = max(yMax, y); }
            } catch (...) {}
        }
        if (yMin < yMax) {
            double pad = (yMax - yMin) * 0.2;
            m_yMin = yMin - pad; m_yMax = yMax + pad;
        } else { m_yMin = -10; m_yMax = 10; }
    }

    if (isValid()) { makeCurrent(); buildAllGeometry(); doneCurrent(); }
    update();
}

void GraphWidget::clearGraph() {
    m_hasGraph = false;
    m_curveSegments.clear();
    m_xMin = -10; m_xMax = 10;
    m_yMin = -10; m_yMax = 10;
    if (isValid()) { makeCurrent(); buildGrid(); doneCurrent(); }
    update();
}

void GraphWidget::initializeGL() {
    initializeOpenGLFunctions();
    m_shader = new QOpenGLShaderProgram(this);
    m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\n"
        "layout(location=0) in vec2 pos;\n"
        "uniform mat4 projection;\n"
        "void main() { gl_Position = projection * vec4(pos, 0.0, 1.0); }\n");
    m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "uniform vec4 color;\n"
        "out vec4 outColor;\n"
        "void main() { outColor = color; }\n");
    m_shader->link();

    m_vao.create(); m_vbo.create();
    m_vao.bind(); m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_shader->enableAttributeArray(0);
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);
    m_vao.release(); m_vbo.release();

    glClearColor(0.08f, 0.08f, 0.13f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    buildAllGeometry();
}

void GraphWidget::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void GraphWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    m_shader->bind();

    QMatrix4x4 proj;
    proj.ortho(m_xMin, m_xMax, m_yMin, m_yMax, -1, 1);
    m_shader->setUniformValue("projection", proj);

    drawLines(m_gridLines, GL_LINES,     0.20f, 0.20f, 0.32f, 1.0f, 1.0f);
    drawLines(m_axisLines, GL_LINES,     0.60f, 0.60f, 0.75f, 1.0f, 2.0f);
    for (auto& seg : m_curveSegments)
        drawLines(seg, GL_LINE_STRIP, 0.25f, 0.70f, 1.00f, 1.0f, 2.5f);

    m_shader->release();
    drawLabels();
}

void GraphWidget::wheelEvent(QWheelEvent* event) {
    double factor = event->angleDelta().y() > 0 ? 0.85 : 1.0 / 0.85;
    double cx = (m_xMin + m_xMax) / 2.0, cy = (m_yMin + m_yMax) / 2.0;
    double hw = (m_xMax - m_xMin) / 2.0 * factor;
    double hh = (m_yMax - m_yMin) / 2.0 * factor;
    m_xMin = cx - hw; m_xMax = cx + hw;
    m_yMin = cy - hh; m_yMax = cy + hh;
    makeCurrent(); buildAllGeometry(); doneCurrent(); update();
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true; m_dragStart = event->pos();
        m_dragXMin = m_xMin; m_dragXMax = m_xMax;
        m_dragYMin = m_yMin; m_dragYMax = m_yMax;
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_dragging) return;
    QPoint delta = event->pos() - m_dragStart;
    double dx = -(double)delta.x() / width()  * (m_dragXMax - m_dragXMin);
    double dy =  (double)delta.y() / height() * (m_dragYMax - m_dragYMin);
    m_xMin = m_dragXMin + dx; m_xMax = m_dragXMax + dx;
    m_yMin = m_dragYMin + dy; m_yMax = m_dragYMax + dy;
    makeCurrent(); buildAllGeometry(); doneCurrent(); update();
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) m_dragging = false;
}

double GraphWidget::niceStep(double range) {
    double raw = range / 8.0;
    double mag = pow(10.0, floor(log10(raw)));
    if (raw <= 1.0 * mag) return 1.0 * mag;
    if (raw <= 2.0 * mag) return 2.0 * mag;
    if (raw <= 5.0 * mag) return 5.0 * mag;
    return 10.0 * mag;
}

void GraphWidget::buildGrid() {
    m_gridLines.clear(); m_axisLines.clear();
    double xStep = niceStep(m_xMax - m_xMin);
    double yStep = niceStep(m_yMax - m_yMin);

    for (double x = ceil(m_xMin/xStep)*xStep; x <= m_xMax+1e-9; x += xStep) {
        auto& t = fabs(x) < xStep*0.01 ? m_axisLines : m_gridLines;
        t.push_back((float)x); t.push_back((float)m_yMin);
        t.push_back((float)x); t.push_back((float)m_yMax);
    }
    for (double y = ceil(m_yMin/yStep)*yStep; y <= m_yMax+1e-9; y += yStep) {
        auto& t = fabs(y) < yStep*0.01 ? m_axisLines : m_gridLines;
        t.push_back((float)m_xMin); t.push_back((float)y);
        t.push_back((float)m_xMax); t.push_back((float)y);
    }
}

void GraphWidget::buildCurve() {
    m_curveSegments.clear();
    if (!m_hasGraph) return;
    vector<float> seg;
    double prevY = NAN;
    for (int i = 0; i <= 2000; i++) {
        double x = m_xMin + (m_xMax - m_xMin) * i / 2000.0;
        double y = NAN;
        try { y = m_eval.evaluate(x); } catch (...) {}
        bool inRange = isfinite(y) && y > m_yMin*2 && y < m_yMax*2;
        bool jump    = isfinite(prevY) && isfinite(y) &&
                       fabs(y - prevY) > 1e4 * (m_yMax - m_yMin);
        if (inRange && !jump) {
            seg.push_back((float)x); seg.push_back((float)y);
        } else {
            if (seg.size() >= 4) m_curveSegments.push_back(seg);
            seg.clear();
            if (inRange) { seg.push_back((float)x); seg.push_back((float)y); }
        }
        prevY = y;
    }
    if (seg.size() >= 4) m_curveSegments.push_back(seg);
}

void GraphWidget::buildAllGeometry() { buildGrid(); buildCurve(); }

void GraphWidget::drawLines(vector<float>& v, GLenum mode,
                             float r, float g, float b, float a, float lw) {
    if (v.empty()) return;
    glLineWidth(lw);
    m_shader->setUniformValue("color", r, g, b, a);
    m_vao.bind(); m_vbo.bind();
    m_vbo.allocate(v.data(), (int)(v.size() * sizeof(float)));
    m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);
    m_shader->enableAttributeArray(0);
    glDrawArrays(mode, 0, (GLsizei)(v.size() / 2));
    m_vao.release(); m_vbo.release();
}

void GraphWidget::drawLabels() {
    QPainter p(this);
    p.setFont(QFont("Monospace", 8));
    p.setPen(QColor(150, 150, 190));
    double xRange = m_xMax - m_xMin, yRange = m_yMax - m_yMin;
    double xStep = niceStep(xRange), yStep = niceStep(yRange);

    for (double x = ceil(m_xMin/xStep)*xStep; x <= m_xMax+1e-9; x += xStep) {
        int px = (int)((x - m_xMin) / xRange * width());
        int py = max(0, min(height()-15, (int)((m_yMax-0) / yRange * height())));
        p.drawText(px-22, py+2, 44, 14, Qt::AlignCenter, QString::number(x,'g',3));
    }
    for (double y = ceil(m_yMin/yStep)*yStep; y <= m_yMax+1e-9; y += yStep) {
        if (fabs(y) > yStep*0.01) {
            int px = max(3, min(width()-55, (int)((0-m_xMin)/xRange*width())));
            int py = (int)((m_yMax - y) / yRange * height());
            p.drawText(px+3, py-6, 52, 14, Qt::AlignLeft, QString::number(y,'g',3));
        }
    }
    if (!m_hasGraph) {
        p.setPen(QColor(110, 110, 160));
        p.setFont(QFont("Monospace", 11));
        p.drawText(rect(), Qt::AlignCenter, "График появится после анализа");
    }
    p.end();
}
