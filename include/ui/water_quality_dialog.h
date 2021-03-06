#ifndef WATER_QUALITY_DIALOG_H
#define WATER_QUALITY_DIALOG_H

#include <domain/water_quality_configuration.h>
#include <repository/water_quality_repository.h>

#include <QDialog>
#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
	class WaterQualityDialog;
}

class WaterQualityDialog : public QDialog {
	Q_OBJECT
private:
	Ui::WaterQualityDialog *ui;
	WaterQualityConfiguration *unsavedConfiguration;
	WaterQualityConfiguration *currentConfiguration;
	WaterQualityRepository *waterQualityRepository;
    bool isWebViewLoaded;

	void buildParameterTree(WaterQualityParameter *parameter);
private slots:
    void on_cbxConfiguration_currentIndexChanged(const QString &configurationName);
	void on_cbxGridDataConfiguration_currentIndexChanged(const QString &gridDataConfigurationName);
	void on_btnRemoveConfiguration_clicked();
	void on_btnNewConfiguration_clicked();
	void on_btnApplyConfiguration_clicked();
	void on_trwStructure_itemChanged(QTreeWidgetItem *item, int column);
    void on_tabWidget_currentChanged(int index);
    void on_webView_loadFinished(bool ok);
    void addJSObject();
    void onTabularInputButtonClicked();
public:
	explicit WaterQualityDialog(QWidget *parent = 0);
	~WaterQualityDialog();
    
    Q_INVOKABLE void toggleItem(const QString &itemName, const bool &checked);
};

#endif // WATER_QUALITY_DIALOG_H