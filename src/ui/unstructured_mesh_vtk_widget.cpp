#include "include/ui/unstructured_mesh_vtk_widget.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkFeatureEdges.h>
#include <vtkWorldPointPicker.h>

#include "include/ui/unstructured_mesh_dialog.h"
#include "include/utility/mouse_interactor.h"

vtkStandardNewMacro(MouseInteractor);

UnstructuredMeshVTKWidget::UnstructuredMeshVTKWidget(QWidget *parent) : QVTKWidget(parent) {
//    polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    actor = vtkSmartPointer<vtkActor>::New();
//    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//    renderer = vtkSmartPointer<vtkRenderer>::New();
}

UnstructuredMeshVTKWidget::~UnstructuredMeshVTKWidget() {
//    renderer->Delete();
//    actor->Delete();
//    renderWindow->Delete();
//    actor->Delete();
//    polyDataMapper->Delete();
}

void UnstructuredMeshVTKWidget::setMesh(UnstructuredMesh *mesh) {
    if (mesh == NULL) {
        return;
    }

//    vtkPolyData *polyData = mesh->toPolyData();

//    if (polyData == NULL) {
//        return;
//    }

    MeshPolygon *boundaryPolygon = mesh->getBoundaryPolygon();

    if (boundaryPolygon->is_empty()) {
        return;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    int i = 0;

    polygon->GetPointIds()->SetNumberOfIds(boundaryPolygon->size());

    for (MeshPolygon::Vertex_iterator vt = boundaryPolygon->vertices_begin(); vt != boundaryPolygon->vertices_end(); vt++) {
        points->InsertNextPoint(vt->x(), vt->y(), 0.0);
        polygon->GetPointIds()->SetId(i, i);
        i++;
    }

    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    vtkSmartPointer<vtkFeatureEdges> edges = vtkSmartPointer<vtkFeatureEdges>::New();
    edges->SetInputData(polyData);
    edges->BoundaryEdgesOn();
    // edges->FeatureEdgesOff();
    // edges->ManifoldEdgesOff();
    // edges->NonManifoldEdgesOff();
    edges->Update();

    // vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    // triangleFilter->SetInputData(polyData);
    // triangleFilter->Update();

    // vtkSmartPointer<vtkPolyData> pd2 = triangleFilter->GetOutput();

    vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    polyDataMapper->SetInputData(edges->GetOutput());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(polyDataMapper);

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->AddRenderer(renderer);

    vtkSmartPointer<vtkWorldPointPicker> worldPointPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetPicker(worldPointPicker);
    renderWindowInteractor->SetRenderWindow(renderWindow);

    vtkSmartPointer<MouseInteractor> mouseInteractor = vtkSmartPointer<MouseInteractor>::New();
    mouseInteractor->SetDefaultRenderer(renderer);
    UnstructuredMeshDialog *parent = (UnstructuredMeshDialog*) this->parent();
    QObject::connect(mouseInteractor, SIGNAL(coordinateChanged(double&, double&)), this->parent(), SLOT(setCoordinate(double&, double&)));

    renderWindowInteractor->SetInteractorStyle(mouseInteractor);

    renderer->AddActor(actor);
    renderer->ResetCamera();

    this->SetRenderWindow(renderWindow);

    renderWindow->Render();
}
