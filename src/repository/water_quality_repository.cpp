#include <repository/water_quality_repository.h>

#include <domain/water_quality_parameter.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

WaterQualityRepository* WaterQualityRepository::instance = nullptr;

WaterQualityRepository* WaterQualityRepository::getInstance() {
    if (!instance) {
        instance = new WaterQualityRepository();
    }
    
    return instance;
}

WaterQualityRepository::WaterQualityRepository() {
    QFile structureFile(":/data/water_quality_structure.json");

    structureFile.open(QFile::ReadOnly);

    QJsonDocument jsonDocument = QJsonDocument::fromJson(structureFile.readAll());
    QJsonArray jsonParameters = jsonDocument.array();
    
    structureFile.close();

    for (int i = 0; i < jsonParameters.size(); i++) {
        QJsonObject jsonParameter = jsonParameters[i].toObject();
        WaterQualityParameter *parameter = new WaterQualityParameter();
        
        parameter->setName(jsonParameter["name"].toString());
        parameter->setLabel(jsonParameter["label"].toString());
        parameter->setDescription(jsonParameter["label"].toString());
        parameter->setSection(WaterQualityParameterSection::STRUCTURE);
        parameter->setTarget(jsonParameter["target"].toString());
        parameter->setDiagramItem(jsonParameter["diagramItem"].toString());
        parameter->setCheckable(jsonParameter["checkable"].toBool(false));
        parameter->setChecked(jsonParameter["checked"].toBool());
        parameter->setEnabled(jsonParameter["enabled"].toBool(true));
        parameter->setRadio(jsonParameter["radio"].toBool(false));
        parameter->setGroup(jsonParameter["group"].toBool(false));
        parameter->setValue(0);
        
        structure.append(parameter);
        
        QString parentName = jsonParameter["parentName"].toString();
        
        for (WaterQualityParameter *parent : structure) {
            if (parent->getName() == parentName) {
                parameter->setParent(parent);
                break;
            }
        }
        
        if (!parameter->getParent()) {
            rootStructure.append(parameter);
        }
    }
    
    QFile parametersFile(":/data/water_quality_parameters.json");
    
    parametersFile.open(QFile::ReadOnly);
    
    jsonDocument = QJsonDocument::fromJson(parametersFile.readAll());
    jsonParameters = jsonDocument.array();
    WaterQualityParameter *lastGroupParameter = nullptr;
    QStringList lastGroups;
    
    parametersFile.close();
    
    for (int i = 0; i < jsonParameters.size(); i++) {
        QJsonObject jsonParameter = jsonParameters[i].toObject();
        WaterQualityParameter *parameter = new WaterQualityParameter();
        
        parameter->setName(jsonParameter["name"].toString());
        parameter->setLabel(jsonParameter["label"].toString());
        parameter->setSection(WaterQualityParameterSection::PARAMETER);
        parameter->setTarget(jsonParameter["target"].toString());
        parameter->setDescription(jsonParameter["description"].toString());
        parameter->setEnabled(jsonParameter["enabled"].toBool(true));
        parameter->setInputType(WaterQualityParameter::mapInputTypeFromString(jsonParameter["inputType"].toString()));
        parameter->setRangeMinimum(jsonParameter["rangeMinimum"].toDouble());
        parameter->setRangeMaximum(jsonParameter["rangeMaximum"].toDouble());
        parameter->setValue(jsonParameter["defaultValue"].toDouble());
        parameter->setRadio(jsonParameter["radio"].toBool(false));
        parameter->setGroup(jsonParameter["group"].toBool(false));
        parameter->setPersistable(jsonParameter["persistable"].toBool(false));
        
        if (parameter->getLabel() == "Groups") {
            lastGroupParameter = parameter;
            lastGroups.clear();
        }
        
        QString parentName = jsonParameter["parentName"].toString();
        
        if (lastGroupParameter) {
            if (lastGroupParameter->getName() == parentName) {
                lastGroups.append(parameter->getName());
            } else {
                QString group = jsonParameter["groups"].toString();
                
                if (lastGroupParameter->getName() == group) {
                    parameter->setGroups(lastGroups);
                }
            }
        }
        
        if (!jsonParameter["groupDefaultValues"].isUndefined()) {
            if (jsonParameter["groupDefaultValues"].isArray()) {
                QJsonArray defaultValuesArray = jsonParameter["groupDefaultValues"].toArray();
                QList<double> values;

                for (QJsonValue value : defaultValuesArray) {
                    values.append(value.toDouble());
                }

                parameter->setGroupValues(values);
            } else {
                parameter->setGroupValues(QVector<double>(lastGroups.size(), jsonParameter["groupDefaultValues"].toDouble()).toList());
            }
        }
        
        for (WaterQualityParameter *parentParameter : parameters) {
            if (parentParameter->getName() == parentName) {
                parameter->setParent(parentParameter);
                break;
            }
        }
        
        if (!parameter->getParent()) {
            rootParameters.append(parameter);
        }
        
        parameters.append(parameter);
    }

    QFile foodMatrixFile(":/data/food_matrix.json");
    
    foodMatrixFile.open(QFile::ReadOnly);
    
    jsonDocument = QJsonDocument::fromJson(foodMatrixFile.readAll());
    QJsonArray jsonFoodMatrix = jsonDocument.array();
    
    for (int i = 0; i < jsonFoodMatrix.size(); i++) {
        QJsonObject jsonFood = jsonFoodMatrix[i].toObject();
        WaterQualityParameter *parameter = this->findParameterByName(jsonFood["parameter"].toString(), WaterQualityParameterSection::STRUCTURE);
        WaterQualityParameter *group = this->findParameterByName(jsonFood["group"].toString(), WaterQualityParameterSection::PARAMETER);
        FoodMatrixElement *foodMatrixElement = new FoodMatrixElement();
        
        foodMatrixElement->setName(jsonFood["name"].toString());
        foodMatrixElement->setLabel(jsonFood["label"].toString());
        foodMatrixElement->setGroup(group);
        foodMatrixElement->setParameter(parameter);
        
        if (jsonFood["prey"].toBool()) {
            preys.append(foodMatrixElement);
        }
        
        if (jsonFood["predator"].toBool()) {
            QJsonArray jsonPredatorValues = jsonFood["values"].toArray();
            
            for (int j = 0; j < jsonPredatorValues.size(); j++) {
                QJsonObject jsonPreyValuePair = jsonPredatorValues[j].toObject();
                FoodMatrixElement *prey = this->findPreyByName(jsonPreyValuePair["prey"].toString());
                FoodMatrixElement *predator = foodMatrixElement;
                
                predator->addPrey(prey);
                foodMatrix.append(FoodMatrix(predator, prey, jsonPreyValuePair["value"].toDouble()));
            }
            
            predators.append(foodMatrixElement);
        }
    }
}

WaterQualityRepository::~WaterQualityRepository() {
    for (WaterQualityParameter *parameter : parameters) {
        delete parameter;
    }
    
    for (WaterQualityParameter *parameter : structure) {
        delete parameter;
    }
}

FoodMatrixElement* WaterQualityRepository::findPreyByName(const QString &name) {
	for (FoodMatrixElement *prey : preys) {
        if (prey->getName() == name) {
            return prey;
        }
    }

    return nullptr;
}

WaterQualityParameter* WaterQualityRepository::findParameterByName(const QString &name, const WaterQualityParameterSection &section) {
    QList<WaterQualityParameter*> *list = section == WaterQualityParameterSection::STRUCTURE ? &structure : &parameters;
    
    for (WaterQualityParameter *parameter : *list) {
        if (parameter->getName() == name && parameter->getSection() == section) {
            return parameter;
        }
    }
    
    return nullptr;
}

WaterQualityParameter* WaterQualityRepository::findParameterByDiagramItem(const QString &diagramItem) {
    for (WaterQualityParameter *parameter : structure) {
        if (parameter->getDiagramItem() == diagramItem) {
            return parameter;
        }
    }
    
    return nullptr;
}

QList<WaterQualityParameter*> WaterQualityRepository::getParameters(const WaterQualityParameterSection &section) const {
    return section == WaterQualityParameterSection::STRUCTURE ? structure : parameters;
}

QList<WaterQualityParameter*> WaterQualityRepository::getRootParameters(const WaterQualityParameterSection &section) const {
    return section == WaterQualityParameterSection::STRUCTURE ? rootStructure : rootParameters;
}

QList<FoodMatrixElement*> WaterQualityRepository::getPredators() const {
    return predators;
}

QList<FoodMatrixElement*> WaterQualityRepository::getPreys() const {
    return preys;
}

double WaterQualityRepository::getFoodMatrixValue(FoodMatrixElement *predator, FoodMatrixElement *prey) const {
    for (FoodMatrix fm : foodMatrix) {
        if (fm.getPredator() == predator && fm.getPrey() == prey) {
            return fm.getValue();
        }
    }
    
    return 0.0;
}