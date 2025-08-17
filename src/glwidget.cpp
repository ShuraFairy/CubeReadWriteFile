#include "glwidget.h"

#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>

bool GLWidget::m_transparent = false;

void GLWidget::initCubeGeometry(float width)
{
    float width_div_2 = width / 2.0f;

    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-width_div_2, -width_div_2,  width_div_2)}, // v0
        {QVector3D( width_div_2, -width_div_2,  width_div_2)}, // v1
        {QVector3D(-width_div_2,  width_div_2,  width_div_2)}, // v2
        {QVector3D( width_div_2,  width_div_2,  width_div_2)}, // v3

        // Vertex data for face 1
        {QVector3D( width_div_2, -width_div_2, width_div_2)},  // v4
        {QVector3D( width_div_2, -width_div_2, -width_div_2)}, // v5
        {QVector3D( width_div_2,  width_div_2,  width_div_2)}, // v6
        {QVector3D( width_div_2,  width_div_2, -width_div_2)}, // v7

        // Vertex data for face 2
        {QVector3D( width_div_2, -width_div_2, -width_div_2)}, // v8
        {QVector3D(-width_div_2, -width_div_2, -width_div_2)}, // v9
        {QVector3D( width_div_2,  width_div_2, -width_div_2)}, // v10
        {QVector3D(-width_div_2,  width_div_2, -width_div_2)}, // v11

        // Vertex data for face 3
        {QVector3D(-width_div_2, -width_div_2, -width_div_2)}, // v12
        {QVector3D(-width_div_2, -width_div_2,  width_div_2)}, // v13
        {QVector3D(-width_div_2,  width_div_2, -width_div_2)}, // v14
        {QVector3D(-width_div_2,  width_div_2,  width_div_2)}, // v15

        // Vertex data for face 4
        {QVector3D(-width_div_2, -width_div_2, -width_div_2)}, // v16
        {QVector3D( width_div_2, -width_div_2, -width_div_2)}, // v17
        {QVector3D(-width_div_2, -width_div_2,  width_div_2)}, // v18
        {QVector3D( width_div_2, -width_div_2,  width_div_2)}, // v19

        // Vertex data for face 5
        {QVector3D(-width_div_2,  width_div_2,  width_div_2)}, // v20
        {QVector3D( width_div_2,  width_div_2,  width_div_2)}, // v21
        {QVector3D(-width_div_2,  width_div_2, -width_div_2)}, // v22
        {QVector3D( width_div_2,  width_div_2, -width_div_2)}  // v23
    };

    GLushort indices[] = {
        0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
        4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
        8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

    m_arrayBuf.create();    
    m_arrayBuf.bind();
    m_arrayBuf.allocate(vertices, 24 * sizeof(VertexData));

    m_indexBuf.create();    
    m_indexBuf.bind();
    m_indexBuf.allocate(indices, 34 * sizeof(GLushort));    
}


GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_indexBuf(QOpenGLBuffer::IndexBuffer)
    , m_program(nullptr)
    , m_timer(new QTimer(this))
    , m_rotationAngle(0.0f)
    , m_rotationSpeed(1.0f)
    , m_rotationDirection(false) // counterclockwise by default // против часовой стрелки
    , m_isRunning(false)
{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    qDebug() << m_core;

    if (m_transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }

    m_timer->setInterval(16); // ~60 FPS
    connect(m_timer, &QTimer::timeout, this, &GLWidget::updateRotation);
}

GLWidget::~GLWidget()
{
    cleanup();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(510, 500);
}

void GLWidget::initData()
{    
    initCubeGeometry(4.0f);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void GLWidget::setRed(int r)
{
    m_red = r;
    emit rChanged(m_red);
    update();
}

void GLWidget::setGreen(int g)
{
    m_green = g;
    emit gChanged(m_green);
    update();
}

void GLWidget::setBlue(int b)
{
    m_blue = b;
    emit bChanged(m_blue);
    update();
}

void GLWidget::cleanup()
{
    m_arrayBuf.destroy();
    m_indexBuf.destroy();


    if (m_program == nullptr)
        return;
    makeCurrent();
    m_vbo.destroy();
    delete m_program;
    m_program = nullptr;
    doneCurrent();
    QObject::disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);
}

void GLWidget::onReadProgress()
{
    m_rotationAngle = m_rotationAngle + 100.0f;
}

void GLWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, m_transparent ? 0 : 1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);    

    initData();

    m_program = new QOpenGLShaderProgram;

    initShaders();    

    m_projMatrixLoc = m_program->uniformLocation("projMatrix");
    m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
    m_normalMatrixLoc = m_program->uniformLocation("normalMatrix");
    m_lightPosLoc = m_program->uniformLocation("lightPos");

    m_camera.setToIdentity();    
    m_camera.translate(0, 0.0f, -10.0f);

    m_program->setUniformValue(m_lightPosLoc, QVector3D(0.0f, 0.0f, 70.0f));

    m_program->release();
}

void GLWidget::initShaders()
{    
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/resources/vshader.glsl"))
        close();

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/resources/fshader.glsl"))
        close();

    if (!m_program->link())
        close();

    if (!m_program->bind())
        close();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    m_world.setToIdentity();
    m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRot / 16.0f, 0, 0, 1);

    //m_world.rotate(m_rotationAngle, 0, 1, 0);
    m_world.rotate(m_rotationAngle, m_rotationAngle, m_rotationAngle, 0);

    m_program->bind();
    m_program->setUniformValue(m_projMatrixLoc, m_proj);
    m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
    QMatrix3x3 normalMatrix = m_world.normalMatrix();
    m_program->setUniformValue(m_normalMatrixLoc, normalMatrix);

    m_program->setUniformValue("u_red", static_cast<float>(m_red) / 255.0f);
    m_program->setUniformValue("u_green", static_cast<float>(m_green) / 255.0f);
    m_program->setUniformValue("u_blue", static_cast<float>(m_blue) / 255.0f);    

    m_arrayBuf.bind();
    m_indexBuf.bind();

    int offset = 0;

    int vertexLocation = m_program->attributeLocation("a_position");
    m_program->enableAttributeArray(vertexLocation);
    m_program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
    m_arrayBuf.release();
    m_indexBuf.release();
    m_program->release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->position().toPoint();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->position().toPoint().x() - m_lastPos.x();
    int dy = event->position().toPoint().y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_xRot + 8 * dy);
        setYRotation(m_yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_xRot + 8 * dy);
        setZRotation(m_zRot + 8 * dx);
    }
    m_lastPos = event->position().toPoint();
}

bool GLWidget::isRunning() const
{
    return m_isRunning;
}

void GLWidget::setRunning(bool running)
{
    if (m_isRunning != running) {
        m_isRunning = running;
        if (m_isRunning) {
            m_timer->start();
            emit rotationStarted();
        } else {
            m_timer->stop();
            emit rotationStopped();
        }
    }
}

void GLWidget::setRotationSpeed(int speed)
{
    // Map slider value (0-100) to rotation speed (0.0-5.0)
    m_rotationSpeed = speed * 0.05f;
}

void GLWidget::setRotationDirection(bool clockwise)
{
    m_rotationDirection = clockwise;
}

void GLWidget::resetRotation()
{
    m_rotationAngle = 0.0f;
    update();
}

void GLWidget::updateRotation()
{
    if (m_isRunning)
    {
        float delta = m_rotationSpeed * 0.5f; // Scale factor for reasonable speed
        if (m_rotationDirection) {
            m_rotationAngle += delta; // Clockwise
        } else {
            m_rotationAngle -= delta; // Counterclockwise
        }
        // Keep angle in [0, 360) range
        if (m_rotationAngle >= 360.0f) m_rotationAngle -= 360.0f;
        if (m_rotationAngle < 0.0f) m_rotationAngle += 360.0f;

        update();
    }
}
