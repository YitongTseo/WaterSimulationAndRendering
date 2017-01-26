#pragma once
#include <G3D/G3DAll.h>
#include <Math.h>

/** A marching cubes implementation based off of Paul Bourke's.
 *  See http://paulbourke.net/geometry/polygonise/.
 */
class MCubes {
public:
    typedef struct {
       Point3 p[8];
       float val[8];
    } GRIDCELL;

    Array<Point3> m_points;

    /** Radius of particles. */
    float radius;

    /** Marching cubes grid increment. */
    float step;
	float invStep;

	Table<Point3, bool> gridFlagTable;

    /**
     * Linearly interpolate the position where an isosurface cuts
     * an edge between two vertices, each with their own scalar value
     */
    Point3 MCubes::VertexInterp(const float isolevel,const Point3 p1,const Point3 p2,const float valp1,const float valp2);
    MCubes(Array<Point3> points, float _rad, float _step);

    /**
     *  Taken from:
     *  http://paulbourke.net/geometry/polygonise/
     *  Given a grid cell and an isolevel, calculate the triangular
     *  facets required to represent the isosurface through the cell.
     *  Return the number of triangular facets, the array "triangles"
     *  will be loaded up with the vertices at most 5 triangular facets.
	 *  0 will be returned if the grid cell is either totally above
     *  of totally below the isolevel.
     */
    void MCubes::Polygonise(const GRIDCELL& grid, const float isolevel,Array<CPUVertexArray::Vertex>& vertexArray);

    /** Populates the passed arrays with the output geometry data. */
    void marchCubes(Array<CPUVertexArray::Vertex>& vertexArray, Array<int>& indexArray);

    /** Populates the index array. */
    void trianglesToVertAndInd(const Array<CPUVertexArray::Vertex>& vertexArray, Array<int>& indexArray);

    /** Helper for triangulate grid. */
	void MCubes::updateCell(GRIDCELL& grid, const Point3& p, const PointHashGrid<Vector3>& hashGrid);
    
    /** Updates the geometry for the corresponding marching cubes grid cell. */
    void triangulateGrid(GRIDCELL& grid, const Point3& botCoord, Array<CPUVertexArray::Vertex>& vertexArray, const PointHashGrid<Vector3>& hashGrid);
};