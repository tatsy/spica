#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_IMAGE_VIEWER_H_
#define _SPICA_IMAGE_VIEWER_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>

#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtWidgets/qgraphicsitem.h>

namespace spica {

class ImageViewer : public QWidget {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    virtual ~ImageViewer();

    void showImage(const QImage& image);

protected:
    void wheelEvent(QWheelEvent* ev) override;

private:
    QVBoxLayout* layout = nullptr;

    QGraphicsScene* scene = nullptr;
    QGraphicsView* view = nullptr;
    
    class GraphicsImageItem;
    GraphicsImageItem* item = nullptr;
};

}  // namespace spica

#endif  // _SPICA_IMAGE_VIEWER_H_
