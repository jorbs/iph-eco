#include "include/domain/structured_mesh.h"

#include "include/utility/cgal_definitions.h"
#include "include/domain/boundary_condition.h"

#include <vtkQuad.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <QApplication>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellCenters.h>

StructuredMesh::StructuredMesh() : resolution(100) {}

uint StructuredMesh::getResolution() const {
    return resolution;
}

void StructuredMesh::setResolution(const uint &resolution) {
    this->resolution = resolution;
}

bool StructuredMesh::instanceOf(const QString &type) {
    return type.contains("StructuredMesh");
}

void StructuredMesh::generate() {
    ulong bounds[4];
    
    computeBounds(bounds);
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> quads = vtkSmartPointer<vtkCellArray>::New();
    int columns = (bounds[1] - bounds[0]) / this->resolution; // xmax - xmin
    int rows = (bounds[3] - bounds[2]) / this->resolution; // ymax - ymin
    double x = bounds[0];
    double y = bounds[2];
    int count = 0, currentStep = 0;
    QMap<Point, vtkIdType> pointsMap;
    
    for (int i = 0; i < rows && !generationCanceled; i++) {
        for (int j = 0; j < columns && !generationCanceled; j++) {
            emit updateProgress(currentStep++);
            QApplication::processEvents();
            
            double quadCenter[3] = { x + this->resolution / 2.0, y + this->resolution / 2.0, 0.0 };
            bool includeQuad = this->pointInMesh(quadCenter);
            
            if (includeQuad) {
                vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();
                Point quadCoordinates[4] = {
                    Point(x, y),
                    Point(x + this->resolution, y),
                    Point(x + this->resolution, y + this->resolution),
                    Point(x, y + this->resolution)
                };
                
                quad->GetPointIds()->SetNumberOfIds(4);
                
                for (int k = 0; k < 4; k++) {
                    Point point = quadCoordinates[k];
                    
                    if (!pointsMap.contains(point)) {
                        pointsMap.insert(point, count++);
                        points->InsertNextPoint(point.x(), point.y(), 0.0);
                    }
                    
                    quad->GetPointIds()->SetId(k, pointsMap.value(point));
                }
                
                quads->InsertNextCell(quad);
            }
            
            x += this->resolution;
        }
        x = bounds[0];
        y += this->resolution;
    }
    
    if (!generationCanceled) {
        meshPolyData = vtkSmartPointer<vtkPolyData>::New();
        meshPolyData->SetPoints(points);
        meshPolyData->SetPolys(quads);
        
        generateBoundaryPolyData();
    }
}

void StructuredMesh::computeBounds(ulong *points) {
    double *bounds = this->boundaryPolygon->getFilteredPolygon()->GetPoints()->GetBounds();

    points[0] = bounds[0] - ((ulong) bounds[0] % this->resolution); // xmin
    points[1] = bounds[1] - ((ulong) bounds[1] % this->resolution); // xmax
    points[2] = bounds[2] - ((ulong) bounds[2] % this->resolution); // ymin
    points[3] = bounds[3] - ((ulong) bounds[3] % this->resolution); // ymax
}

bool StructuredMesh::pointInMesh(double *point) {
    if (boundaryPolygon->pointInPolygon(point)) {
        QList<MeshPolygon*> islands = this->getIslands();

        for (QList<MeshPolygon*>::const_iterator it = islands.begin(); it != islands.end(); it++) {
            if ((*it)->pointInPolygon(point)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

SimulationDataType::StructuredMesh* StructuredMesh::toSimulationDataType(const HydrodynamicConfiguration *hydrodynamicConfiguration) const {
    SimulationDataType::StructuredMesh *structuredMesh = new SimulationDataType::StructuredMesh();
    enum class EdgeDirection { SOUTH = 0, EAST, NORTH, WEST };
    vtkIdType numberOfCells = this->meshPolyData->GetNumberOfCells();
    
    vtkSmartPointer<vtkCellCenters> cellCentersFilter = vtkSmartPointer<vtkCellCenters>::New();
    cellCentersFilter->SetInputData(this->meshPolyData);
    cellCentersFilter->Update();
    
    structuredMesh->numberOfElements = numberOfCells;
    structuredMesh->resolution = this->resolution;
    structuredMesh->xCoordinates = new double[numberOfCells];
    structuredMesh->yCoordinates = new double[numberOfCells];
    
    for (vtkIdType i = 0; i < cellCentersFilter->GetOutput()->GetNumberOfPoints(); i++) {
        double center[3];
        cellCentersFilter->GetOutput()->GetPoint(i, center);
        structuredMesh->xCoordinates[i] = center[0];
        structuredMesh->yCoordinates[i] = center[1];
    }
    
    BoundaryCondition *waterFlowBoundaryCondition = nullptr;
    
    // TODO: search all water flow conditions
    for (BoundaryCondition *boundaryCondition : hydrodynamicConfiguration->getBoundaryConditions()) {
        if (boundaryCondition->getType() == BoundaryConditionType::WATER_FLOW) {
            waterFlowBoundaryCondition = boundaryCondition;
            break;
        }
    }
    
    QSet<vtkIdType> boundaryCellIds;
    
    if (waterFlowBoundaryCondition) {
        boundaryCellIds = this->getBoundaryCellIds(waterFlowBoundaryCondition->getVTKObjectIds());
    }
    
    structuredMesh->northNeighbors = new vtkIdType[numberOfCells];
    structuredMesh->westNeighbors = new vtkIdType[numberOfCells];
    structuredMesh->southNeighbors = new vtkIdType[numberOfCells];
    structuredMesh->eastNeighbors = new vtkIdType[numberOfCells];
    
    for (vtkIdType cellId = 0; cellId < numberOfCells; cellId++) {
        vtkSmartPointer<vtkIdList> cellNeighbors = vtkSmartPointer<vtkIdList>::New();
        vtkSmartPointer<vtkIdList> cellPointIds = vtkSmartPointer<vtkIdList>::New();
        vtkIdType directionIndex = 0;
        
        this->meshPolyData->GetCellPoints(cellId, cellPointIds);
        
        for (vtkIdType cellPointId = 0; cellPointId < cellPointIds->GetNumberOfIds(); cellPointId++) {
            vtkSmartPointer<vtkIdList> edge = vtkSmartPointer<vtkIdList>::New();
            
            edge->InsertNextId(cellPointIds->GetId(cellPointId));
            edge->InsertNextId(cellPointIds->GetId(cellPointId + 1 == cellPointIds->GetNumberOfIds() ? 0 : cellPointId + 1));
            this->meshPolyData->GetCellNeighbors(cellId, edge, cellNeighbors);
            
            if (cellNeighbors->GetNumberOfIds() == 0) {
                cellNeighbors->InsertNextId(-1);
            }
            
            for (vtkIdType i = 0; i < cellNeighbors->GetNumberOfIds(); i++) {
                vtkIdType *directionArray = nullptr;
                vtkIdType neighborId = cellNeighbors->GetId(i);
                
                if (directionIndex == (vtkIdType) EdgeDirection::SOUTH) {
                    directionArray = structuredMesh->southNeighbors;
                } else if (directionIndex == (vtkIdType) EdgeDirection::EAST) {
                    directionArray = structuredMesh->eastNeighbors;
                } else if (directionIndex == (vtkIdType) EdgeDirection::NORTH) {
                    directionArray = structuredMesh->northNeighbors;
                } else {
                    directionArray = structuredMesh->westNeighbors;
                }
                
                directionArray[cellId] = boundaryCellIds.contains(neighborId) ? -2 : neighborId;
                directionIndex++;
            }
        }
    }
    
    structuredMesh->verticeIdsLength = this->meshPolyData->GetNumberOfCells() * 4;
    structuredMesh->verticeIds = new long long int[structuredMesh->verticeIdsLength];
    vtkIdType count = 0;
    
    for (vtkIdType i = 0; i < this->meshPolyData->GetNumberOfCells(); i++) {
        vtkSmartPointer<vtkIdList> vertices = vtkSmartPointer<vtkIdList>::New();

        this->meshPolyData->GetCellPoints(i, vertices);
        for (vtkIdType j = 0; j < vertices->GetNumberOfIds(); j++) {
            structuredMesh->verticeIds[count++] = vertices->GetId(j);
        }
    }
    
    return structuredMesh;
}