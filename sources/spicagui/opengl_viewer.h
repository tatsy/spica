#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_OPENGL_VIEWER_H_
#define _SPICA_OPENGL_VIEWER_H_

#include <memory>

#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qopenglwidget.h>

namespace spica {

    class OpenGLViewer : public QOpenGLWidget {
        Q_OBJECT

    public:
        explicit OpenGLViewer(QWidget *parent = 0);
        virtual ~OpenGLViewer();

    protected:
        void initializeGL();
        void resizeGL(int width, int height);
        void paintGL();

        void wheelEvent(QWheelEvent* e);
        void mousePressEvent(QMouseEvent* e);
        void mouseMoveEvent(QMouseEvent* e);
        void mouseReleaseEvent(QMouseEvent* e);

    protected slots:
        void animate();

    private:
        std::unique_ptr<QTimer> timer = nullptr;
        
    };

}

#endif  // _SPICA_OPENGL_VIEWER_H_
