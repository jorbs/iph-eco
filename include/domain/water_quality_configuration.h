#ifndef WATER_QUALITY_CONFIGURATION_H
#define WATER_QUALITY_CONFIGURATION_H

#include "hydrodynamic_configuration.h"
#include "water_quality_parameter.h"

#include <QHash>

class WaterQualityConfiguration {
private:
    uint id;
    QString name;
    QList<WaterQualityParameter*> parameters;
    QHash<QPair<QString, QString>, double> foodMatrix;
    HydrodynamicConfiguration *hydrodynamicConfiguration;
    
    // Transient
    bool loaded;
    
public:
    WaterQualityConfiguration();
    ~WaterQualityConfiguration();

    uint getId() const;
    void setId(const uint &id);
    bool isPersisted() const;
    QString getName() const;
    void setName(const QString &name);
    HydrodynamicConfiguration* getHydrodynamicConfiguration() const;
    void setHydrodynamicConfiguration(HydrodynamicConfiguration *hydrodynamicConfiguration);
    bool addWaterQualityParameter(WaterQualityParameter *waterQualityParameter);
    QList<WaterQualityParameter*> getParameters(const bool &persistable = false) const;
    WaterQualityParameter* getParameter(const QString &name, const WaterQualityParameterSection &section) const;
    WaterQualityParameter* getParameterByDiagramItem(const QString &itemName) const;
    QList<WaterQualityParameter*> getRootParameters(const WaterQualityParameterSection &section) const;
    QList<double> getFoodMatrixValues(const QString &predator, const QString &prey) const;
    void setFoodMatrixItem(const QString &predator, const QString &prey, const double &value);
    void clearFoodMatrix();
    QHash<QPair<QString, QString>, double> getFoodMatrix() const;
    void setLoaded(const bool &loaded);
    bool isLoaded() const;
    SimulationDataType::WaterQualityConfiguration* toSimulationDataType() const;
};

#endif // WATER_QUALITY_CONFIGURATION_H