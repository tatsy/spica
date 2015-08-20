#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_PARAM_WIDGET_H_
#define _SPICA_RENDER_PARAM_WIDGET_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qboxlayout.h>

#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>

namespace spica {

    class RenderParamWidget : public QWidget {
    public:
        RenderParamWidget(QWidget* parent = NULL);
        ~RenderParamWidget();

    protected:        
        QWidget* formWidget;
        QFormLayout* formLayout;
        QVBoxLayout* verticalLayout;

        QLineEdit* samplePixelEdit;
        QLineEdit* bounceLimitEdit;
        QLineEdit* startRouletteEdit;
        QLineEdit* castPhotonsEdit;
        QLineEdit* gatherPhotonsEdit;
        QLineEdit* gatherRadiusEdit;
        QComboBox* randomTypeCombo;
        QLineEdit* filenameFormatEdit;

        QPushButton* renderButton;

        friend class SceneViewer;
    };

}  // namespace spica

#endif  // _SPICA_RENDER_PARAM_WIDGET_H_
