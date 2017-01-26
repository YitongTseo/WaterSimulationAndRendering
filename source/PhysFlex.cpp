/** \file App.cpp */
#include "App.h"
#include "PhysFlex.h"

/** Parameters for the simulation of waves. */
class Waves: public flexScene
{
public:

	Waves(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		float radius = 0.025f; // particle size
		const float restDistance = radius*0.65f; // desired distance between particles at rest

		int dx = int(ceilf(.75f / restDistance));
		int dy = int(ceilf(.75f / restDistance));
		int dz = int(ceilf(.75f / restDistance));
		mFlex->CreateParticleGrid(Vector3(-.5f, restDistance, -1.0f), dx, dy, dz, restDistance, Vector3(0.0f,0.0f,0.0f), 1.0f, false, 0.0f, flexMakePhase(0, eFlexPhaseSelfCollide | eFlexPhaseFluid), 0.005f);
		mFlex->g_sceneLower = Vector3(-0.1f, -0.1f, -0.6f);
		mFlex->g_sceneUpper = Vector3(2.1f, 0.1f, -0.6f);
		mFlex->g_numSubsteps = 2;
		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mFluid = true;
		mFlex->g_params.mDynamicFriction = 0.00f;
		mFlex->g_params.mViscosity = 0.01f;
		mFlex->g_params.mNumIterations = 2;
		mFlex->g_params.mVorticityConfinement = 75.0f;
		mFlex->g_params.mAnisotropyScale = 30.0f;		
		mFlex->g_params.mFluidRestDistance = radius*0.6f;
		mFlex->g_params.mRelaxationFactor = 1.0f;
		mFlex->g_params.mSmoothing = 0.5f;
		mFlex->g_params.mDiffuseThreshold *= .25f;
		mFlex->g_params.mCohesion = 0.05f;
		mFlex->g_maxDiffuseParticles = 64*1024;	// 0 if you don't want diffuse particles	

        // special parameters for wave pool simulation
		mFlex->g_wavePool = true;
		mFlex->g_params.mNumPlanes = 5;
		mFlex->g_waveFloorTilt = 0.0f;
		mFlex->g_waveFrequency = 5.0f;
		mFlex->g_waveAmplitude = .75f;

        mFlex->g_params.mCollisionDistance = radius;			//!< Distance particles maintain against shapes, note that for robust collision against triangle meshes this distance should be greater than zero
		mFlex->g_params.mParticleCollisionMargin = radius *.05f;		//!< Increases the radius used during neighbor finding, this is useful if particles are expected to move significantly during a single step to ensure contacts aren't missed on subsequent iterations
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mCollisionDistance *0.05f;//radius * .25f;		//!< Increases the radius used during contact finding against kinematic shapes
	
		mFlex->g_params.mMaxSpeed = mFlex->g_numSubsteps*mFlex->g_params.mRadius/mFlex->g_dt;			
		mFlex->g_params.mDynamicFriction = 0.25f;
		mFlex->g_params.mParticleFriction =.25f;
		mFlex->g_params.mDissipation = 0.0f;
		mFlex->g_params.mRestitution = 0.0;
		mFlex->g_params.mGravity[1] *= 1.75f;

	}

	Flex* mFlex;

};

/** Parameters for the simulation of the faucet. */
class Sprout : public flexScene
{
public:
	Sprout(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		float radius = 0.03f;
		float restDistance = radius*0.8f;
	
		mFlex->g_params.mFluid = true;
		mFlex->g_numSubsteps = 2;
		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mNumIterations = 3;
		mFlex->g_params.mFluidRestDistance =restDistance;

		mFlex->g_params.mViscosity = 0.01f;
		mFlex->g_params.mNumIterations = 2;
		mFlex->g_params.mVorticityConfinement = 75.0f;
		mFlex->g_params.mAnisotropyScale = 30.0f;		
		mFlex->g_params.mFluidRestDistance = radius*0.6f;
		mFlex->g_params.mRelaxationFactor = 1.0f;
		mFlex->g_params.mSmoothing = 0.5f;
		mFlex->g_params.mDiffuseThreshold *= 0.25f;
		mFlex->g_params.mCohesion = 0.05f;
		mFlex->g_params.mCollisionDistance = radius;			//!< Distance particles maintain against shapes, note that for robust collision against triangle meshes this distance should be greater than zero
		mFlex->g_params.mParticleCollisionMargin = radius *.05f;		//!< Increases the radius used during neighbor finding, this is useful if particles are expected to move significantly during a single step to ensure contacts aren't missed on subsequent iterations
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mCollisionDistance *0.05f;//radius * .25f;		//!< Increases the radius used during contact finding against kinematic shapes
	
		mFlex->g_params.mMaxSpeed = mFlex->g_numSubsteps*mFlex->g_params.mRadius/mFlex->g_dt;			
		mFlex->g_params.mDynamicFriction = 0.25f;
		mFlex->g_params.mParticleFriction =.25f;
		mFlex->g_params.mDissipation = 0.0f;
		mFlex->g_params.mRestitution = 0.0;
		mFlex->g_params.mGravity[1] *= 1.25f;
		float emitterSize = 0.2f;
		

		Emitter e;
		e.mEnabled = true;
		e.mWidth = int(emitterSize/restDistance);
		e.mPos = Vector3(0.3f, 1.6f, 0.1f);
		e.mDir = Vector3(0.0f, -1.0f, 0.0f);
		e.mRight = Vector3(1.0f, 0.0f, 0.0f);
		e.mSpeed = 3.f*(restDistance*2.f/mFlex->g_dt);        
		e.timeLeft = 8.0f;
		
		mFlex->g_sceneUpper.z = 5.0f;
		mFlex->g_emitters.push_back(e);
		mFlex->g_numExtraParticles = 64*1024;
		mFlex->g_emit = true;
	}

	Flex* mFlex;

};


/** Parameters for the simulation of the bunny collision scene. */
class Bunny: public flexScene
{
public:

	Bunny(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		float radius = 0.05f;
		const float restDistance = radius*0.65f;

		mFlex->g_sceneLower = Vector3(-1.5f, 0.0f, -1.f);
		mFlex->g_sceneUpper = Vector3(2.5f, 5.0f, 1.f);

		mFlex->g_numSubsteps = 2;
		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mFluid = true;


		mFlex->g_maxDiffuseParticles = 6*1024;		
		mFlex->g_diffuseScale = 0.5f;
		mFlex->g_params.mDiffuseBallistic = 16;
		mFlex->g_params.mDiffuseBuoyancy = 1.0f;
		mFlex->g_params.mDiffuseDrag = 1.0f;
		mFlex->g_params.mNumPlanes = 5;
		

		mFlex->g_params.mViscosity = 0.01f;
		mFlex->g_params.mNumIterations = 2;
		mFlex->g_params.mVorticityConfinement = 75.0f;
		mFlex->g_params.mAnisotropyScale = 30.0f;		
		mFlex->g_params.mFluidRestDistance = radius*0.6f;
		mFlex->g_params.mRelaxationFactor = 1.0f;
		mFlex->g_params.mSmoothing = 0.5f;
		mFlex->g_params.mDiffuseThreshold *= 5.0f;
		mFlex->g_params.mCohesion = 0.05f;
		mFlex->g_params.mParticleCollisionMargin = mFlex->g_params.mRadius*0.5f;
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mRadius*0.5f;
		mFlex->g_params.mMaxSpeed = 0.5f*.1f*float(mFlex->g_numSubsteps)/mFlex->g_dt;
		mFlex->g_params.mDynamicFriction = 0.25f;
		mFlex->g_params.mParticleFriction =.25f;
		mFlex->g_params.mDissipation = 0.0f;
		mFlex->g_params.mRestitution = 0.0;


		float emitterSize = 0.5f;
		mFlex->g_params.mGravity[1] *= 1.2f;

		Emitter e;
		e.mEnabled = true;
		e.mWidth = int(emitterSize/restDistance);
		e.mPos = Vector3(2.7f,.5f, 0.0f);
		e.mDir = Vector3(-1.0f, 0.0f, 0.0f);
		e.mRight = Vector3(0.0f, 0.0f, 1.0f);
		e.mSpeed = 2.f*(restDistance/mFlex->g_dt);
		e.timeLeft = 3.0f;
		mFlex->g_emitters.push_back(e);
		mFlex->g_numExtraParticles = 64*1024;
		mFlex->g_emit = true;
	}

	Flex* mFlex;

};

/** Parameters for the simulation of the goo. */
class Goo: public flexScene
{
public:

	Goo(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		
		float radius = 0.07f;
		float restDistance = radius*0.5f;
		float mDissipation = .12f;
		float mViscosity = 5.0f;

		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mFluid = true;
		mFlex->g_params.mNumIterations = 3;
		mFlex->g_params.mVorticityConfinement = 0.0f;
		mFlex->g_params.mFluidRestDistance = restDistance;
		mFlex->g_params.mAnisotropyScale = 3.0f/radius;
		mFlex->g_params.mSmoothing = 0.35f;
		mFlex->g_params.mRelaxationFactor = 1.f;
		mFlex->g_params.mRestitution = 0.0f;
		mFlex->g_params.mCollisionDistance = 0.00125f;
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mCollisionDistance*0.25f;
		mFlex->g_params.mDissipation = mDissipation;
		mFlex->g_params.mGravity[1] *= 2.0f;
		mFlex->g_params.mDynamicFriction = 1.0f;
		mFlex->g_params.mStaticFriction = 0.0f;
		mFlex->g_params.mViscosity = 20.0f + 20.0f*mViscosity;
		mFlex->g_params.mAdhesion = 0.1f*mViscosity;
		mFlex->g_params.mCohesion = 0.05f*mViscosity;
		mFlex->g_params.mSurfaceTension = 0.0f;	
		mFlex->g_sceneLower = Vector3(-1.5f, 0.0f, -1.f);
		mFlex->g_sceneUpper = Vector3(2.5f, 5.0f, 1.f);
		mFlex->g_params.mNumPlanes = 5;


		float emitterSize = 0.5f;

		Emitter e;
		e.mEnabled = true;
		e.mWidth = int(emitterSize/restDistance);
		e.mPos = Vector3(-.4f,3.0f, 0.3f);
		e.mDir = Vector3(0.0f, -1.0f, 0.0f);
		e.mRight = Vector3(1.0f, 0.0f, 0.0f);
		e.mSpeed = (restDistance*2.f/mFlex->g_dt);        
		e.timeLeft = 2.0f;
		mFlex->g_emitters.push_back(e);
		mFlex->g_numExtraParticles = 64*1024;
		mFlex->g_emit = true;
	}

	Flex* mFlex;

};

/** Parameters for the simulation of flooding sponza. */
class Sponza : public flexScene
{
public:

	Sponza(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		
		float radius = 0.6f;
		float restDistance = radius*0.5f;
		float mDissipation = .12f;
		float mViscosity = 1.0f;

		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mFluid = true;
		mFlex->g_params.mNumIterations = 3;
		mFlex->g_params.mVorticityConfinement = 0.0f;
		mFlex->g_params.mFluidRestDistance = restDistance;
		mFlex->g_params.mAnisotropyScale = 3.0f/radius;
		mFlex->g_params.mSmoothing = 0.35f;
		mFlex->g_params.mRelaxationFactor = 1.f;
		mFlex->g_params.mRestitution = 0.0f;
		mFlex->g_params.mCollisionDistance = 0.00125f;
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mCollisionDistance*0.25f;
		mFlex->g_params.mDissipation = mDissipation;
		mFlex->g_params.mGravity[1] *= 2.0f;
		mFlex->g_params.mDynamicFriction = 1.0f;
		mFlex->g_params.mStaticFriction = 0.0f;
		mFlex->g_params.mViscosity = 20.0f + 20.0f*mViscosity;
		mFlex->g_params.mAdhesion = 0.1f*mViscosity;
		mFlex->g_params.mCohesion = 0.05f*mViscosity;
		mFlex->g_params.mSurfaceTension = 0.0f;	
		mFlex->g_sceneLower = Vector3(-10.0f, 0.0f, -10.0f);
		mFlex->g_sceneUpper = Vector3(10.0f, 5.0f, 10.0f);
		mFlex->g_params.mNumPlanes = 5;


		float emitterSize = 3.5f;

		Emitter e;
		e.mEnabled = true;
		e.mWidth = int(emitterSize/restDistance);
		e.mPos = Vector3(-.4f,5.0f, 0.3f);
		e.mDir = Vector3(0.0f, -1.0f, 0.0f);
		e.mRight = Vector3(1.0f, 0.0f, 0.0f);
		e.mSpeed = (restDistance*2.f/mFlex->g_dt);        
		e.timeLeft = 8.0f;
		mFlex->g_emitters.push_back(e);
		mFlex->g_numExtraParticles = 64*1024;
		mFlex->g_emit = true;
	}

	Flex* mFlex;

};

/** Parameters for the lighthouse scene. */
class LightHouse: public flexScene
{
public:

	LightHouse(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		float radius = 0.05f;
		float restDistance = radius*0.6f;

		int dx = int(ceilf(2.0f / restDistance));
		int dy = int(ceilf(3.0f / restDistance));
		int dz = int(ceilf(2.0f / restDistance));
		mFlex->CreateParticleGrid(Vector3(1.0f, 1.0f, 1.0f), dx, dy, dz, restDistance, Vector3(0.0f,0.0f,0.0f), 1.0f, false, 0.0f, flexMakePhase(0, eFlexPhaseSelfCollide | eFlexPhaseFluid), 0.005f);
		mFlex->g_sceneLower = Vector3(-3.5f,0.0f,-2.5f);
		mFlex->g_sceneUpper = Vector3(2.5f, 10.0f, 3.5f);
		mFlex->g_numSubsteps = 2;
		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mDynamicFriction = 0.f;
		mFlex->g_params.mFluid = true;
		mFlex->g_params.mViscosity = 0.01f;		
		mFlex->g_params.mNumIterations = 3;
		mFlex->g_params.mVorticityConfinement = 50.0f;
		mFlex->g_params.mAnisotropyScale = 20.0f;
		mFlex->g_params.mFluidRestDistance = restDistance;
		mFlex->g_params.mGravity[1] *= 0.5f;
		mFlex->g_params.mCohesion *= 0.5f;
		mFlex->g_maxDiffuseParticles = 64*1024;
		mFlex->g_diffuseScale = 0.3f;
		mFlex->g_params.mDiffuseThreshold *= 0.5f;
		mFlex->g_params.mDiffuseBallistic = 4;
		mFlex->g_params.mDiffuseBuoyancy = 2.0f;
		mFlex->g_params.mDiffuseDrag = 1.0f;
		mFlex->g_params.mNumPlanes = 5;
		mFlex->g_waveFrequency = 1.2f;
		mFlex->g_waveAmplitude = 2.2f;
		mFlex->g_wavePool = true;
	}

	Flex* mFlex;

};

FlexTriangleMesh* Flex::CreateTriangleMesh(const ArticulatedModel::Mesh* mesh) {
    const ArticulatedModel::Geometry* geom = mesh->geometry;
    if (isNull(geom)) {
        return NULL;
    }

    float lower[3] = { -100.0f, -100.0f, -100.0f };
	float upper[3] = { 100.0f, 100.0f, 100.0f };

	FlexTriangleMesh* flexMesh = flexCreateTriangleMesh();

    if (isNull(flexMesh)) { return flexMesh; }
    
	std::vector<float> pos;
	std::vector<int> vert;
        
	for (int i = 0; i < geom->cpuVertexArray.size(); ++i) {
		const CPUVertexArray::Vertex& v = geom->cpuVertexArray.vertex[i];
		pos.push_back(v.position.x);
        pos.push_back(v.position.y);
        pos.push_back(v.position.z);
	}

	for (const auto& index : mesh->cpuIndexArray) {
		vert.push_back(index);
	}

	int vertices = mesh->cpuIndexArray.size();
	int faces = mesh->triangleCount();
	flexUpdateTriangleMesh(flexMesh, pos.data(), vert.data(), vertices, faces, lower, upper, eFlexMemoryHost);
	return flexMesh;
}

/** Parameters for the simulation of the fountain scene. */
class Fountain: public flexScene
{
public:
	Fountain(Flex* flex) : mFlex(flex) {}

	virtual void Initialize()
	{
		float radius = 0.1f;
		float restDistance = radius*0.8f;
	
		mFlex->g_params.mFluid = true;
		mFlex->g_numSubsteps = 2;
		mFlex->g_params.mRadius = radius;
		mFlex->g_params.mNumIterations = 3;
		mFlex->g_params.mFluidRestDistance =restDistance;

		mFlex->g_params.mViscosity = 0.01f;
		mFlex->g_params.mNumIterations = 2;
		mFlex->g_params.mVorticityConfinement = 75.0f;
		mFlex->g_params.mAnisotropyScale = 30.0f;		
		mFlex->g_params.mFluidRestDistance = radius*0.6f;
		mFlex->g_params.mRelaxationFactor = 1.0f;
		mFlex->g_params.mSmoothing = 0.5f;
		mFlex->g_params.mDiffuseThreshold *= 0.25f;
		mFlex->g_params.mCohesion = 0.05f;
		mFlex->g_params.mCollisionDistance = radius;			//!< Distance particles maintain against shapes, note that for robust collision against triangle meshes this distance should be greater than zero
		mFlex->g_params.mParticleCollisionMargin = radius *.05f;		//!< Increases the radius used during neighbor finding, this is useful if particles are expected to move significantly during a single step to ensure contacts aren't missed on subsequent iterations
		mFlex->g_params.mShapeCollisionMargin = mFlex->g_params.mCollisionDistance *0.05f;//radius * .25f;		//!< Increases the radius used during contact finding against kinematic shapes
	
		mFlex->g_params.mMaxSpeed = .75 * mFlex->g_numSubsteps*mFlex->g_params.mRadius/mFlex->g_dt;			
		mFlex->g_params.mDynamicFriction = 0.25f;
		mFlex->g_params.mParticleFriction =.25f;
		mFlex->g_params.mDissipation = 0.0f;
		mFlex->g_params.mRestitution = 0.0;
		//mFlex->g_params.mGravity[1] *= 1.5f;
		float emitterSize = 0.2f;
		

		Emitter e;
		e.mEnabled = true;
		e.mWidth = int(emitterSize/restDistance);
		e.mPos = Vector3(0.0f, 5.5f, 0.0f);
		e.mDir = Vector3(0.0f, 1.0f, 0.0f);
		e.mRight = Vector3(1.0f, 0.0f, 0.0f);
		e.mSpeed = 2.f*(restDistance*2.f/mFlex->g_dt);        
		e.timeLeft = 8.0f;
		
		mFlex->g_sceneUpper.z = 15.0f;
		mFlex->g_emitters.push_back(e);
		mFlex->g_numExtraParticles = 64*1024;
		mFlex->g_emit = true;
	}

	Flex* mFlex;

};

void Flex::AddTriangleMesh(FlexTriangleMesh* mesh, const Vector3& translation, float scale)
{
	if (!mesh)
		return;

	Vector3 lower, upper;
	flexGetTriangleMeshBounds(mesh, (float*)&lower.x, (float*)&upper.x);

	Quat rotation = Quat();
	FlexCollisionGeometry geo;
	geo.mTriMesh.mMesh = mesh;
	geo.mTriMesh.mScale = scale;

	g_shapeStarts.push_back(g_shapeGeometry.size());
	g_shapeAabbMin.push_back(Vector4(lower, 0.0f));
	g_shapeAabbMax.push_back(Vector4(upper, 0.0f));
	g_shapePositions.push_back(Vector4(translation, 0.0f));
	g_shapePrevPositions.push_back(Vector4(translation, 0.0f));
	g_shapeGeometry.push_back(geo);
	g_shapeFlags.push_back(flexMakeShapeFlags(eFlexShapeTriangleMesh, false));		
	g_shapeRotations.push_back(Quat(rotation));
	g_shapePrevRotations.push_back(Quat(rotation));

}

void Flex::ErrorCallback(FlexErrorSeverity, const char* msg, const char* file, int line)
{
	printf("Flex: %s - %s:%d\n", msg, file, line);
	assert(0);
}

void Flex::GetParticleBounds(Vector3& lower, Vector3& upper)
{
	lower = Vector3(FLT_MAX,FLT_MAX,FLT_MAX);
	upper = Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	for (size_t i=0; i < g_positions.size(); ++i)
	{
		lower = min(Vector3(g_positions[i].x,g_positions[i].y,g_positions[i].z), lower);
		upper = max(Vector3(g_positions[i].x,g_positions[i].y,g_positions[i].z), upper);
	}
}

Vector3 Flex::RandomUnitVector()
{
	Random r;
	float phi = r.uniform(0,PI*2.0f);
	float theta = r.uniform(0,PI*2.0f);

	float cosTheta = cos(theta);
	float sinTheta = sin(theta);

	float cosPhi = cos(phi);	
	float sinPhi = sin(phi);

	return Vector3(cosTheta*sinPhi,cosPhi,sinTheta*sinPhi);
}


void Flex::CreateParticleGrid(Vector3 lower, int dimx, int dimy, int dimz, float radius, Vector3 velocity, float invMass, bool rigid, float rigidStiffness, int phase, float jitter)
{
	for (int x = 0; x < dimx; ++x) {
		for (int y = 0; y < dimy; ++y) {
			for (int z= 0; z < dimz; ++z) {
				Vector3 position = lower + Vector3(float(x), float(y), float(z)) * radius + RandomUnitVector() * jitter;

				g_positions.push_back(Vector4(position.x, position.y, position.z, invMass));
				g_velocities.push_back(velocity);
				g_phases.push_back(phase);
			}
		}
	}

}

Vector3 Flex::safeNormalize(Vector3 v){
	if (!v.x || !v.y || !v.z) {
		return Vector3(0.0f, 1.0f, 0.0f);
	} else {
		return v;
	}
}

void Flex::Init(){
	g_positions.resize(0);
	g_velocities.resize(0);
	g_phases.resize(0);

	g_emitters.resize(0);

	// sim params
	g_params.mGravity[0] = 0.0f;
	g_params.mGravity[1] = -9.8f;
	g_params.mGravity[2] = 0.0f;

	g_params.mWind[0] = 0.0f;
	g_params.mWind[1] = 0.0f;
	g_params.mWind[2] = 0.0f;

	g_params.mRadius = 0.15f;
	g_params.mViscosity = 0.0f;
	g_params.mDynamicFriction = 0.0f;
	g_params.mStaticFriction = 0.0f;
	g_params.mParticleFriction = 0.0f; // scale friction between particles by default
	g_params.mFreeSurfaceDrag = 0.0f;
	g_params.mDrag = 0.0f;
	g_params.mLift = 0.0f;
	g_params.mNumIterations = 3;
	g_params.mFluidRestDistance = 0.0f;
	g_params.mSolidRestDistance = 0.0f;
	
	g_params.mAnisotropyScale = 1.0f;
	g_params.mAnisotropyMin = 0.1f;
	g_params.mAnisotropyMax = 2.0f;
	g_params.mSmoothing = 1.0f;

	g_params.mDissipation = 0.0f;
	g_params.mDamping = 0.0f;
	g_params.mParticleCollisionMargin = 0.0f;
	g_params.mShapeCollisionMargin = 0.0f;
	g_params.mCollisionDistance = 0.0f;
	g_params.mPlasticThreshold = 0.0f;
	g_params.mPlasticCreep = 0.0f;
	g_params.mFluid = false;
	g_params.mSleepThreshold = 0.0f;
	g_params.mShockPropagation = 0.0f;
	g_params.mRestitution = 0.0f;	
	g_params.mMaxSpeed = FLT_MAX;
	g_params.mRelaxationMode = eFlexRelaxationLocal;
	g_params.mRelaxationFactor = 1.0f;
	g_params.mSolidPressure = 1.0f;
	g_params.mAdhesion = 0.0f;
	g_params.mCohesion = 0.025f;
	g_params.mSurfaceTension = 0.0f;
	g_params.mVorticityConfinement = 0.0f;
	g_params.mBuoyancy = 1.0f;
	g_params.mDiffuseThreshold = 100.0f;
	g_params.mDiffuseBuoyancy = 1.0f;
	g_params.mDiffuseDrag = 0.8f;
	g_params.mDiffuseBallistic = 16;
	g_params.mDiffuseSortAxis[0] = 0.0f;
	g_params.mDiffuseSortAxis[1] = 0.0f;
	g_params.mDiffuseSortAxis[2] = 0.0f;
	g_params.mDiffuseLifetime = 1.0f;
	g_params.mInertiaBias = 0.001f;

	g_numSubsteps = 2;

	g_diffuseScale = 0.5f;

	// planes created after particles
	g_params.mNumPlanes = 1;


	g_numSolidParticles = 0;

	g_waveFrequency = 1.5f;
	g_waveAmplitude = 1.5f;
	g_waveFloorTilt = 0.0f;

	
	g_maxDiffuseParticles = 0;	// number of diffuse particles
	g_maxNeighborsPerParticle = 96;
	g_numExtraParticles = 0;	// number of particles allocated but not made active	

	g_sceneLower = Vector3(FLT_MAX,FLT_MAX,FLT_MAX);
	g_sceneUpper = Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	//sceneInit();

	g_scene->Initialize();

	uint32_t numParticles = g_positions.size();
	uint32_t maxParticles = numParticles + g_numExtraParticles*g_numExtraMultiplier;

	// by default solid particles use the maximum radius
	if (g_params.mFluid && g_params.mSolidRestDistance == 0.0f)
		g_params.mSolidRestDistance = g_params.mFluidRestDistance;
	else
		g_params.mSolidRestDistance = g_params.mRadius;

	// collision distance with shapes half the radius
	if (g_params.mCollisionDistance == 0.0f)
	{
		g_params.mCollisionDistance = g_params.mRadius*0.5f;

		if (g_params.mFluid)
			g_params.mCollisionDistance = g_params.mFluidRestDistance*0.5f;
	}

	// default particle friction to 10% of shape friction
	if (g_params.mParticleFriction == 0.0f)
		g_params.mParticleFriction = g_params.mDynamicFriction*0.1f; 

	// add a margin for detecting contacts between particles and shapes
	if (g_params.mShapeCollisionMargin == 0.0f)
		g_params.mShapeCollisionMargin = g_params.mCollisionDistance*0.5f;
		
	// calculate particle bounds
	Vector3 lower, upper;
	GetParticleBounds(lower, upper);

	// expand
	lower -= Vector3(g_params.mCollisionDistance,g_params.mCollisionDistance,g_params.mCollisionDistance);
	upper += Vector3(g_params.mCollisionDistance,g_params.mCollisionDistance,g_params.mCollisionDistance);

	g_sceneLower = min(g_sceneLower, lower);
	g_sceneUpper = max(g_sceneUpper, upper);

	// update collision planes to match flexs
	//Vec3 up = Normalize(Vec3(-0.05f, 1.0f, 0.0f));
	Vector3 up = normalize(Vector3(-g_waveFloorTilt, 1.0f, 0.0f));
	(Vector4&)g_params.mPlanes[0] = Vector4(up.x, up.y, up.z, 0.0f);
	(Vector4&)g_params.mPlanes[1] = Vector4(0.0f, 0.0f, 1.0f, -g_sceneLower.z);
	(Vector4&)g_params.mPlanes[2] = Vector4(1.0f, 0.0f, 0.0f, -g_sceneLower.x);
	(Vector4&)g_params.mPlanes[3] = Vector4(-1.0f, 0.0f, 0.0f, g_sceneUpper.x);
	(Vector4&)g_params.mPlanes[4] = Vector4(0.0f, 0.0f, -1.0f, g_sceneUpper.z);
	(Vector4&)g_params.mPlanes[5] = Vector4(0.0f, -1.0f, 0.0f, g_sceneUpper.y);
	
	g_wavePlane = g_params.mPlanes[2][3];

	g_diffusePositions.resize(g_maxDiffuseParticles);
	g_diffuseVelocities.resize(g_maxDiffuseParticles);
	g_diffuseIndicies.resize(g_maxDiffuseParticles);
	
	g_normals.resize(0);
	g_normals.resize(maxParticles);

	for (int i=0; i < int(maxParticles); ++i)
		g_normals[i] = Vector4(safeNormalize(Vector3(g_normals[i].x,g_normals[i].y,g_normals[i].z)), 0.0f);

	g_flex = flexCreateSolver(maxParticles, g_maxDiffuseParticles, g_maxNeighborsPerParticle); 
		
	flexSetParams(g_flex, &g_params);
	flexSetParticles(g_flex, (float*)g_positions.data(), numParticles, eFlexMemoryHost);
	flexSetVelocities(g_flex, (float*)g_velocities.data(), numParticles, eFlexMemoryHost);
	flexSetNormals(g_flex, (float*)g_normals.data(), numParticles, eFlexMemoryHost);
	
	
	g_activeIndices.resize(maxParticles);
	for (size_t i=0; i < g_activeIndices.size(); ++i)
		g_activeIndices[i] = i;

	flexSetActive(g_flex, g_activeIndices.data(), numParticles, eFlexMemoryHost);

	g_positions.resize(maxParticles);
	g_velocities.resize(maxParticles);
	g_phases.resize(maxParticles);

	g_densities.resize(maxParticles);
	g_anisotropy1.resize(maxParticles);
	g_anisotropy2.resize(maxParticles);
	g_anisotropy3.resize(maxParticles);


	if (g_shapePositions.size()) {
		flexSetShapes(
			g_flex, 
			&g_shapeGeometry[0], 
			int(g_shapeGeometry.size()),
			(float*)&g_shapeAabbMin[0], 
			(float*)&g_shapeAabbMax[0], 
			(int*)&g_shapeStarts[0],			
			(float*)&g_shapePositions[0], 
			(float*)&g_shapeRotations[0], 
			(float*)&g_shapePrevPositions[0],
			(float*)&g_shapePrevRotations[0],
			&g_shapeFlags[0],
			int(g_shapeStarts.size()),
			eFlexMemoryHost
        );
	}

	flexSetPhases(g_flex, &g_phases[0], g_phases.size(), eFlexMemoryHost);

	g_restPositions = g_positions;

	flexSetRestParticles(g_flex, (float*)&g_restPositions[0], g_restPositions.size(), eFlexMemoryHost);

}

void Flex::flexStep(){
	static FlexTimers timers;
	static double lastTime;


	memset(&timers, 0, sizeof(timers));

	g_windTime += g_dt;


	if (g_emit){			
		int activeCount = flexGetActiveCount(g_flex);

		for (size_t e = 0; e < g_emitters.size(); ++e){
			if (!g_emitters[e].mEnabled || g_emitters[e].timeLeft < 0.0f) continue;
			g_emitters[e].timeLeft -= g_dt;
			Vector3 emitterDir = g_emitters[e].mDir;
			Vector3 emitterRight = g_emitters[e].mRight;
			Vector3 emitterPos = g_emitters[e].mPos;
			float r = g_params.mFluidRestDistance;
			int phase =  flexMakePhase(0, eFlexPhaseSelfCollide | eFlexPhaseFluid);
			float numParticles = (g_emitters[e].mSpeed / r)*g_dt;
			int n = int(numParticles + g_emitters[e].mLeftOver);
			if (n)
				g_emitters[e].mLeftOver = (numParticles + g_emitters[e].mLeftOver)-n;
			else
				g_emitters[e].mLeftOver += numParticles;

			// create a grid of particles (n particles thick)
			for (int k = 0; k < n; ++k)
			{
				int emitterWidth = g_emitters[e].mWidth;
				int numParticles = emitterWidth*emitterWidth;
				for (int i=0; i < numParticles; ++i)
				{
					float x = float(i%emitterWidth) - emitterWidth/2;
					float y = float((i/emitterWidth)%emitterWidth) - emitterWidth/2;

					if ((pow(x,2) + pow(y,2)) <= (emitterWidth/2)*(emitterWidth/2))
					{
						Vector3 up = normalize(emitterDir.cross(emitterRight));
						Vector3 offset = r*(emitterRight*x + up*y) + float(k)*emitterDir*r;

						if (size_t(activeCount) < g_positions.size())
						{
							g_positions[activeCount] = Vector4(emitterPos + offset, 1.0f);
							g_velocities[activeCount] = emitterDir*g_emitters[e].mSpeed;
							g_phases[activeCount] = phase;
							activeCount++;
						}
					}
				}
			}
		}

		flexSetActive(g_flex, &g_activeIndices[0], activeCount, eFlexMemoryHost);
	}


	const Vector3 kWindDir = Vector3(3.0f, 15.0f, 0.0f);
	Noise n;
	float kNoise = n.sampleFloat(g_windTime*g_windFrequency, 10, 0.25f);
	Vector3 wind = g_windStrength*kWindDir*Vector3(kNoise, fabsf(kNoise), 0.0f);
			
	g_params.mWind[0] = wind.x;
	g_params.mWind[1] = wind.y;
	g_params.mWind[2] = wind.z;

	if (g_wavePool)
	{
		g_waveTime += g_dt;

		g_params.mPlanes[2][3] = g_wavePlane + (sinf(float(g_waveTime)*g_waveFrequency - PI*0.5f)*0.5f + 0.5f)*g_waveAmplitude;
	}

	flexSetParticles(g_flex, &g_positions[0].x, g_positions.size(), eFlexMemoryHost);
	flexSetVelocities(g_flex, &g_velocities[0].x, g_velocities.size(), eFlexMemoryHost);	
	flexSetPhases(g_flex, &g_phases[0], g_phases.size(), eFlexMemoryHost);

	flexSetParams(g_flex, &g_params);
	flexUpdateSolver(g_flex, g_dt, g_numSubsteps, g_profile?&timers:NULL);

	g_frame++;
	

	g_waterActive = flexGetActiveCount(g_flex);

	// need up to date positions host side for interaction / debug rendering
	flexGetParticles(g_flex, &g_positions[0].x, g_positions.size(), eFlexMemoryHost);
	flexGetVelocities(g_flex, &g_velocities[0].x, g_velocities.size(), eFlexMemoryHost);
	flexGetNormals(g_flex, &g_normals[0].x, g_normals.size(), eFlexMemoryHost);
	g_diffuseActive = flexGetDiffuseParticles(g_flex, &g_diffusePositions.data()->x, &g_diffuseVelocities.data()->x, g_diffuseIndicies.data() ,eFlexMemoryHost);
    flexSetFence();
    flexWaitFence();

}

Array<Vector3> Flex::getWaterPositions(){
	Array<Vector3> points;
	for( int i = 0; i < g_waterActive;++i){
		points.append(Vector3(g_positions[i].x,g_positions[i].y,g_positions[i].z));
	}
	return points;
}

//for future look into doing memcopies wtih gpu memory to use cuda stuff for these arrays
Array<Vector4> Flex::getDiffusePositions(){
	Array<Vector4> points;
	for( int i = 0; i < g_diffuseActive;++i){ 
		points.append(Vector4(g_diffusePositions[i].x,g_diffusePositions[i].y,g_diffusePositions[i].z,g_diffusePositions[i].w));
	}

	return points;
}


float Flex::getWaterRadius(){
	return g_params.mRadius;
}
float Flex::getDiffuseRadius(){
	return g_params.mRadius * g_diffuseScale;
}

Flex::Flex(sceneName name){
	switch(name)
	{
		case waves: 
            g_scene = new Waves(this);
            break;
		case bunny:
            g_scene = new Bunny(this);
            break;
		case sprout:
			g_scene = new Sprout(this);
            break;
		case goo:
			g_scene = new Goo(this);
            break;
		case lightHouse:
			g_scene = new LightHouse(this);
			break;
		case sponza:
			g_scene = new Sponza(this);
            break;
		case fountain:
			g_scene = new Fountain(this);
			break;
	}

	FlexError err = flexInit(FLEX_VERSION);
	if (err != eFlexErrorNone)
	{
		printf("Error (%d), could not initialize flex\n", err);
		exit(-1);
	}
}

