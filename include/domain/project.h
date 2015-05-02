#ifndef PROJECT_H
#define PROJECT_H

#include <QSet>

#include "mesh.h"
#include "grid_data_configuration.h"

class Project {
    private:
        qint8 id;
        QString name;
        QString description;
        QString filename;
        bool hydrodynamic;
        bool waterQuality;
        bool sediment;
        QSet<Mesh*> meshes;
        QSet<GridDataConfiguration*> gridDataConfigurations;

        //Transient attributes
        bool dirty;

    public:
        Project(QString &_name, QString &_description, bool &_hydrodynamic, bool &_sediment, bool &_waterQuality);

        void setId(const qint8 &id);
        qint8 getId() const;

        void setName(const QString &name);
        QString getName() const;

        void setDescription(const QString &description);
        QString getDescription() const;

        void setFilename(const QString &filename);
        QString getFilename() const;

        void setHydrodynamic(const bool &hydrodynamic);
        bool getHydrodynamic() const;

        void setWaterQuality(const bool &waterQuality);
        bool getWaterQuality() const;

        void setSediment(const bool &sediment);
        bool getSediment() const;

        const QSet<Mesh*>& getMeshes();

        bool addMesh(Mesh *mesh);
        void removeMesh(Mesh *mesh);
        bool containsMesh(Mesh *mesh);
        Mesh* getMesh(Mesh *mesh);
        Mesh* getMesh(const QString &meshName);

        bool addGridDataConfiguration(GridDataConfiguration *gridDataConfiguration);
        void removeGridDataConfiguration(const QString &configurationName);
        GridDataConfiguration* getGridDataConfiguration(const QString &configurationName);
        QSet<GridDataConfiguration*>& getGridDataConfigurations();

        //Transient gets and sets
        bool isDirty() const;
        void setDirty(const bool &dirty);
};

#endif // PROJECT_H
