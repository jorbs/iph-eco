#include "include/domain/quad.h"

Quad::~Quad() {
    for (QSet<CellInfo*>::iterator it = cellInfoSet.begin(); it != cellInfoSet.end(); it++) {
        delete *it;
    }
}

QSet<CellInfo*>& Quad::getCellInfoSet() {
    return cellInfoSet;
}

bool Quad::addCellInfo(CellInfo *cellInfo) {
    if (cellInfoSet.contains(cellInfo)) {
        return false;
    }

    cellInfoSet.insert(cellInfo);
    return true;
}

CellInfo* Quad::getCellInfo(GridInformationType &gridInformationType) {
    for (QSet<CellInfo*>::const_iterator it = cellInfoSet.begin(); it != cellInfoSet.end(); it++) {
        if ((*it)->getGridInformationType() == gridInformationType) {
            return *it;
        }
    }

    return NULL;
}