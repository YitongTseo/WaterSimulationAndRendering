#include "WaterModel.h"

/*
Change Log:
- created by Kenny, Yitong, Melanie, and Cole for the final
*/

shared_ptr<Model> WaterModel::createWaterModel(Array<Vector3>& waterPositions, float waterRadius, float waterStep) {
    const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty("waterModel");

    ArticulatedModel::Part*     part      = model->addPart("root");
    ArticulatedModel::Geometry* geometry  = model->addGeometry("geom");
    ArticulatedModel::Mesh*     mesh      = model->addMesh("mesh", part, geometry);

    // Assign a material
    mesh->material = UniversalMaterial::create(
        PARSE_ANY(
        UniversalMaterial::Specification {
			lambertian = Color3(0);
            glossy     = Color4(Color3(0.1), 1);
            transmissive = Color3(0.8, 0.9, 1.0);
            extinctionTransmit = Color3(1,1,1);
            extinctionReflect = Color3(0,0,0);
        }));
    
    Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
    Array<int>& indexArray = mesh->cpuIndexArray;
    MCubes(waterPositions, waterRadius, waterStep).marchCubes(vertexArray, indexArray);

    // Tell the ArticulatedModel to generate bounding boxes, GPU vertex arrays,
    // normals and tangents automatically. We already ensured correct
    // topology, so avoid the vertex merging optimization.
    ArticulatedModel::CleanGeometrySettings geometrySettings;
    geometrySettings.allowVertexMerging = false;
    model->cleanGeometry(geometrySettings);

    return model;
}


void WaterModel::addWaterToScene(Array<Vector3>& waterPositions, shared_ptr<Scene>& scene, float waterRadius, float waterStep) {
    // Replace any existing torus model. Models don't 
    // have to be added to the model table to use them 
    // with a VisibleEntity.
    const shared_ptr<Model>& waterModel = createWaterModel(waterPositions, waterRadius, waterStep);
    if (scene->modelTable().containsKey(waterModel->name())) {
        scene->removeModel(waterModel->name());
    }
    scene->insert(waterModel);

    // Replace any existing torus entity that has the wrong type
    shared_ptr<Entity> water = scene->entity("water");
    if (notNull(water) && isNull(dynamic_pointer_cast<VisibleEntity>(water))) {
        logPrintf("The scene contained an Entity named 'water' that was not a VisibleEntity\n");
        scene->remove(water);
        water.reset();
    }
    //this is to stop it from crashing when the water model is empty
    if (waterPositions.size() < 1) { 
        return;
    }

    if (isNull(water)) { 
        // We could either explicitly instantiate a VisibleEntity or simply
        // allow the Scene parser to construct one. The second approach
        // has more consise syntax for this case, since we are using all constant
        // values in the specification.
        water = scene->createEntity("water",
            PARSE_ANY(
                VisibleEntity {
                    model = "waterModel";
                };
            ));
    } else {
        // Change the model on the existing torus entity
        dynamic_pointer_cast<VisibleEntity>(water)->setModel(waterModel);
    }

    water->setFrame(CFrame::fromXYZYPRDegrees(0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

const Any DIFFUSE_PARTICLE_ENTITY_ANY = PARSE_ANY(
    VisibleEntity {
        model = "sphereModel";
    };
);

void WaterModel::addDiffuseToScene(Array<Vector4>& diffusePositions, shared_ptr<Scene>& scene, float diffuseRadius, float diffuseStep) {
    for (int i = 0; i < diffusePositions.size(); ++i) {
        const Vector4& pos = diffusePositions[i];

        const String entityName = format("Diffuse Sphere %d", i);
        if (isNull(scene->typedEntity<VisibleEntity>(entityName))) {
            scene->createEntity(entityName, DIFFUSE_PARTICLE_ENTITY_ANY);
        }
        const shared_ptr<VisibleEntity>& entity = scene->typedEntity<VisibleEntity>(entityName);
        debugAssertM(!isNull(entity), "Diffuse sphere entity should exist but doesn't");
        
        UniversalMaterial::Specification spec(Color4(0.95f, 0.95f, 1.0f, int(10 * pos.w) / 10.0f));
        // We're using the kappaPos to tag this model as a diffuse particle
        // kappNeg stores the current age and radius of the diffuse particle
        spec.setExtinction(Color3(int(10 * pos.w) / 10.0f, 0, diffuseRadius), foamParticleTag);
        // Transmissive can't be Color3(0) otherwise it will cast a shadow in our pathTracer so we just set to it some value that's not 0.
        spec.setTransmissive(Texture::Specification(Color3(0.5)));

        entity->articulatedModelPose().materialTable.set("default/low-poly-sphere", UniversalMaterial::create(spec));
        entity->setFrame(CFrame::fromXYZYPRDegrees(pos.x, pos.y, pos.z));
    }

    for (int i = diffusePositions.size(); i < m_previousDiffuseParticleCount; ++i) {
        const String entityName = format("Diffuse Sphere %d", i);
        scene->removeEntity(entityName);
    }
    m_previousDiffuseParticleCount = diffusePositions.size();
}
