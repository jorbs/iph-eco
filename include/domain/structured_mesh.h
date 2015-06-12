#ifndef STRUCTURED_MESH_H
#define STRUCTURED_MESH_H

#include "mesh.h"

class StructuredMesh : public Mesh {
private:
    uint resolution;

    bool pointInMesh(double *point);

public:
    StructuredMesh();

    uint getResolution() const;
    void setResolution(const uint &resolution);
    void computeBounds(ulong *points);

    virtual bool instanceOf(const QString &type);
    virtual void generate();
};

#endif // STRUCTURED_MESH_H
