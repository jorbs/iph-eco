#ifndef MESH_H
#define MESH_H

#include "mesh_polygon.h"

#include <QSet>
#include <QHash>
#include <QList>
#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkIdTypeArray.h>

enum class LabelType { CELL_ID = 1, CELL_WEIGHT, VERTICE_ID, UNDEFINED };

class Mesh : public QObject {
    Q_OBJECT
protected:
    uint id;
    QString name;
    MeshPolygon *boundaryPolygon;
    double coordinatesDistance;
    QList<MeshPolygon*> islands;
    QList<MeshPolygon*> refinementAreas;
    vtkSmartPointer<vtkPolyData> meshPolyData;
    vtkSmartPointer<vtkPolyData> boundaryPolyData;

    // Transient attributes
    bool generationCanceled;
    QString color;
    int lineWidth;
    int lineStyle;
    int opacity;
    
    void generateBoundaryPolyData();

public:
    Mesh();
    virtual ~Mesh();

    void setId(const uint &id);
    uint getId() const;
    bool isPersisted() const;
    void setName(const QString &name);
    QString getName() const;
    void setCoordinatesDistance(const double &coordinatesDistance);
    double getCoordinatesDistance() const;
    MeshPolygon* getBoundaryPolygon();
    QList<MeshPolygon*> getIslands();
    QList<MeshPolygon*> getRefinementAreas();
    vtkSmartPointer<vtkPolyData> getMeshPolyData() const;
    vtkSmartPointer<vtkPolyData> getBoundaryPolyData() const;
    QString getMeshPolygonsAsString();
    QString getMeshPolyDataAsString() const;
    QString getBoundaryPolyDataAsString() const;
    void loadMeshPolyDataFromString(const QString &polyDataStr);
    void loadBoundaryPolyDataFromString(const QString &polyDataStr);
    double getLatitudeAverage() const;
    
    MeshPolygon* addMeshPolygon(const QString &name, const QString &filename, const MeshPolygonType &meshPolygonType, const CoordinateSystem &coordinateSystem);
    void addMeshPolygon(MeshPolygon *meshPolygon);
    void removeMeshPolygon(const QString &name, const MeshPolygonType &meshPolygonType);
    MeshPolygon* getMeshPolygon(const QString &name, const MeshPolygonType &meshPolygonType) const;

    bool hasArray(const QString &arrayName);
    void removeArray(const QString &arrayName);
    QSet<vtkIdType> getBoundaryCellIds(vtkSmartPointer<vtkIdTypeArray> edgeIds) const;
    
    QString getColor() const;
    void setColor(const QString &color);
    int getLineWidth() const;
    void setLineWidth(int lineWidth);
    int getLineStyle() const;
    void setLineStyle(int lineStyle);
    int getOpacity() const;
    void setOpacity(int opacity);

    virtual bool instanceOf(const QString &type) = 0;
    virtual void generate() = 0;
    virtual void clear();
signals:
    void updateProgress(int value);
    
public slots:
    void cancelGeneration(bool value = true);
};

#endif // MESH_H