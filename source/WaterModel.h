#pragma once
#include <G3D/G3DAll.h>
#include "MCubes.h"
#include "PathTracer.h"

class WaterModel {
protected:
    /** The number of diffuse particles in the scene the last time diffuse particles were generated. Used to remove unused entities. */
    int m_previousDiffuseParticleCount = 0;
public:
    /** Returns a pointer to a model representing the water particles as described by the parameters. The model is created through marching cubes. */
    shared_ptr<Model> createWaterModel(Array<Vector3>& waterPositions, float waterRadius, float waterStep);

    /** Creates a water model with WaterModel::createWaterModel and adds it to the passed scene. */
    void addWaterToScene(Array<Vector3>& waterPositions, shared_ptr<Scene>& scene, float waterRadius, float waterStep);

    /** Adds diffuse particles to the scene. The particles are visible entities sharing a sphere model. */
	void WaterModel::addDiffuseToScene(Array<Vector4>& diffusePositions, shared_ptr<Scene>& scene, float diffuseRadius, float diffuseStep);
};