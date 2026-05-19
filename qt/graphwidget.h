#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <vector>

#include "../src/main.cpp"

// =============================================================================
// GraphWidget — отрисовка графика f(x) через OpenGL 3.3 Core Profile
// Колесо мыши — масштаб, зажатая кнопка — перетаскивание
// =============================================================================

class GraphWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent = nullptr);
    ~GraphWidget() override;

    void setExpression(const QString& expr, double xMin, double xMax);
    void clearGraph();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    double niceStep(double range);
    void   buildGrid();
    void   buildCurve();
    void   buildAllGeometry();
    void   drawLines(std::vector<float>& v, GLenum mode,
                     float r, float g, float b, float a, float lw);
    void   drawLabels();

    QOpenGLShaderProgram*       m_shader = nullptr;
    QOpenGLVertexArrayObject    m_vao;
    QOpenGLBuffer               m_vbo{QOpenGLBuffer::VertexBuffer};
    std::vector<float>          m_gridLines, m_axisLines;
    std::vector<std::vector<float>> m_curveSegments;
    ExpressionEvaluator         m_eval;

    bool   m_hasGraph = false;
    double m_xMin = -10, m_xMax = 10, m_yMin = -10, m_yMax = 10;
    bool   m_dragging = false;
    QPoint m_dragStart;
    double m_dragXMin = 0, m_dragXMax = 0, m_dragYMin = 0, m_dragYMax = 0;
};
