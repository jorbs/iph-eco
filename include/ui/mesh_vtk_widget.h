#ifndef MESH_VTK_WIDGET_H
#define MESH_VTK_WIDGET_H

#include <QWidget>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>

#include <domain/mesh.h>
#include <utility/mesh_mouse_interactor.h>

class MeshVTKWidget : public QVTKWidget {
	Q_OBJECT
protected:
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<MeshMouseInteractor> mouseInteractor;
    
    vtkSmartPointer<vtkActor> meshActor;
    vtkSmartPointer<vtkCubeAxesActor> axesActor;
    vtkSmartPointer<vtkActor2D> labelsActor;
    vtkSmartPointer<vtkActor> verticesActor;
    
    Mesh *currentMesh;
    bool showBoundaryEdges;
    bool showMesh;
    bool showAxes;
    
    void renderAxesActor();
public:
    MeshVTKWidget(QWidget *parent, MeshMouseInteractor *meshMouseInteractor = nullptr);
    ~MeshVTKWidget();
    void render(Mesh *mesh);
    virtual void clear();
    Mesh* getMesh() const;

public slots:
    virtual void toggleMesh(bool show);
    virtual void toggleAxes(bool show);
    virtual void resetZoom();
    virtual void toggleZoomArea(bool activate);
    virtual void toggleLabels(const LabelType &labelType);
    virtual void changeBackgroundColor(const double &r, const double &g, const double &b);
    virtual void exportToImage(const QString &fileName);
    virtual void changeMeshProperties(Mesh *mesh);
};

#endif // MESH_VTK_WIDGET_H
