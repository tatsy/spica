#include "render_param_widget.h"

namespace spica {
    
    RenderParamWidget::RenderParamWidget(QWidget* parent)
        : QWidget(parent)
        , formWidget(new QWidget)
        , formLayout(new QFormLayout)
        , verticalLayout(new QVBoxLayout)
        , samplePixelEdit(new QLineEdit)
        , bounceLimitEdit(new QLineEdit)
        , startRouletteEdit(new QLineEdit)
        , castPhotonsEdit(new QLineEdit)
        , gatherPhotonsEdit(new QLineEdit)
        , gatherRadiusEdit(new QLineEdit)
        , randomTypeCombo(new QComboBox)
        , filenameFormatEdit(new QLineEdit)
        , renderButton(new QPushButton)
    {
        setLayout(verticalLayout);

        formWidget->setLayout(formLayout);
        formLayout->addRow("Samples", samplePixelEdit);
        formLayout->addRow("Bounce Limit", bounceLimitEdit);
        formLayout->addRow("Roulette Start", startRouletteEdit);
        formLayout->addRow("Cast Photons", castPhotonsEdit);
        formLayout->addRow("Gather Photons", gatherPhotonsEdit);
        formLayout->addRow("Gather Radius", gatherRadiusEdit);
        formLayout->addRow("Random Type", randomTypeCombo);
        formLayout->addRow("Filename", filenameFormatEdit);

        verticalLayout->addWidget(formWidget);
        verticalLayout->addWidget(renderButton);

        renderButton->setText("Render");
    }

    RenderParamWidget::~RenderParamWidget()
    {
        delete samplePixelEdit;
        delete bounceLimitEdit;
        delete startRouletteEdit;
        delete castPhotonsEdit;
        delete gatherPhotonsEdit;
        delete gatherRadiusEdit;
        delete randomTypeCombo;
        delete filenameFormatEdit;

        delete renderButton;

        delete verticalLayout;
    }

}  // namespace spica
