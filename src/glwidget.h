//#pragma once
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QMatrix4x4>
#include <QTimer>

struct VertexData
{
    QVector3D position;    
};


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    static bool isTransparent() { return m_transparent; }
    static void setTransparent(bool t) { m_transparent = t; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void initData();    
    void initCubeGeometry(float width);
    void initShaders();
    bool isRunning() const;

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);
    void cleanup();
    void onReadProgress();

    void setRunning(bool running);
    void setRotationSpeed(int speed);
    void setRotationDirection(bool clockwise);
    void resetRotation();


signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void rChanged(int r);
    void gChanged(int g);
    void bChanged(int b);

    void rotationStarted();
    void rotationStopped();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateRotation();

private:
    void setupVertexAttribs();

    bool m_core;
    int m_xRot = 0;
    int m_yRot = 0;
    int m_zRot = 0;
    QPoint m_lastPos;    
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_projMatrixLoc = 0;
    int m_mvMatrixLoc = 0;
    int m_normalMatrixLoc = 0;
    int m_lightPosLoc = 0;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
    static bool m_transparent;    

    int m_red;
    int m_green;
    int m_blue;

    QOpenGLBuffer m_arrayBuf;
    QOpenGLBuffer m_indexBuf;

    QTimer *m_timer;
    // Rotation parameters
    float m_rotationAngle;
    float m_rotationSpeed ;
    bool m_rotationDirection; // true = clockwise, false = counterclockwise
    bool m_isRunning;
};

#endif
