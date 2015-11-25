#ifndef LAYER_PROPERTIES_DIALOG_H
#define LAYER_PROPERTIES_DIALOG_H

#include "include/domain/layer_properties.h"

#include <QColor>
#include <QDialog>
#include <QToolButton>
#include <QAbstractButton>

namespace Ui {
	class LayerPropertiesDialog;
}

class LayerPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    enum LayerPropertiesTab {
        MAP = (1 << 0),
        POINTS = (1 << 1),
        MESH = (1 << 2),
        VECTORS = (1 << 3)
    };
    
    LayerPropertiesDialog(QWidget *parent, LayerProperties *layerProperties, const LayerPropertiesTab &tabs = static_cast<LayerPropertiesTab>( LayerPropertiesTab::MAP | LayerPropertiesTab::POINTS | LayerPropertiesTab::MESH));
    ~LayerPropertiesDialog();
private:
    Ui::LayerPropertiesDialog *ui;
    LayerProperties *layerProperties;
    LayerPropertiesTab tabs;
    QColor currentLineColor;
    QColor currentVectorColor;
    QToolButton *currentMapColorGradientButton;
    QToolButton *defaultMapColorGradientButton;
    QToolButton *currentPointsColorGradientButton;
    QToolButton *defaultPointsColorGradientButton;
    
    void setupMapTab();
    void setupPointsTab();
    void setupMeshTab();
    void setupVectorsTab();
    void removeTab(const LayerPropertiesTab &tab);
    void setupColorGradientTemplates(QToolButton *&defaultButton, QToolButton *&currentButton, bool isMapTab);
    bool isValid();
private slots:
    void colorGradientButtonClicked(bool checked);
    void on_buttonBox_clicked(QAbstractButton *button);
    void showColorPickerDialog();
signals:
    void applyChanges();
};

#endif // LAYER_PROPERTIES_DIALOG_H