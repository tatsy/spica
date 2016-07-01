#include "image_viewer.h"

#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>

namespace spica {

class ImageViewer::GraphicsImageItem : public QGraphicsPixmapItem {
public:
    GraphicsImageItem()
        : QGraphicsPixmapItem{} {
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->drawPixmap(0, 0, pixmap());
    }

    void showImage(const QImage& image) {
        setPixmap(QPixmap::fromImage(image));
    }

private:
    QImage image_;
};

ImageViewer::ImageViewer(QWidget* parent)
    : QWidget{ parent } {
    layout = new QVBoxLayout(this);
    setLayout(layout);

    scene = new QGraphicsScene(this);
    view  = new QGraphicsView(scene);
    view->show();    
    layout->addWidget(view);

    QImage image(100, 100, QImage::Format_ARGB32);
    image.fill(Qt::black);

    item = new GraphicsImageItem();
    item->showImage(image);
    scene->addItem(item);
}

ImageViewer::~ImageViewer() {
    delete view;
    delete layout;
}

void ImageViewer::showImage(const QImage& image) {
    item->showImage(image);
}

void ImageViewer::wheelEvent(QWheelEvent* ev) {
    QRectF beforeRect = item->sceneBoundingRect();

    QTransform trans = item->transform();
    float scale = ev->delta() > 0 ? 1.0f / 0.95f : 0.95f;
    item->setTransform(trans.scale(scale, scale));
}

}  // namespace spica
