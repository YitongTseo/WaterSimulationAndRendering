/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */

/*
Change Log:
- based on NVIDIA Flex sample code
- modified by Kenny, Melanie, Yitong, and Cole for the final
*/

#pragma once
#include "flex.h"
#include <G3D/G3DAll.h>

class Flex;

enum sceneName { waves, bunny, sprout, goo, fountain, lightHouse, sponza };

/** Defines the emitter, which is the class that produces and releases particles into the particle system. Conceptually this is similar to a hose.*/
struct Emitter
{
	Emitter() : mSpeed(0.0f), mEnabled(false), mLeftOver(0.0f), mWidth(8), timeLeft(2.0f) {}

    // variables to define the properties of the emitter and how it will release particles
	Vector3 mPos;
	Vector3 mDir;
	Vector3 mRight;
	float mSpeed;
	bool mEnabled;
	float mLeftOver;
	int mWidth;
	float timeLeft;
};

/** A scene class for the simulation scene, which keeps track of all bodies involved in the simulation. */
class flexScene {
public:
	Flex* mFlex;

	flexScene(Flex* flex) : mFlex(flex) {};
	flexScene(){};

	virtual void Initialize() = 0;
};

/** The simulation handler. */
class Flex {
public:
	#define FLEX_VERSION 100

	const float PI = 3.141592654;
	
	int g_numSubsteps;
	int g_cudaDevice = -1;
	
	FlexSolver* g_flex;
	FlexParams g_params;
	
    // parameters for foam particles
	int g_maxDiffuseParticles;
	unsigned char g_maxNeighborsPerParticle;
	int g_numExtraParticles;
	int g_numExtraMultiplier = 1;
	
    // parameters for water particles
	std::vector<Vector4> g_positions;
	std::vector<Vector4> g_restPositions;
	std::vector<Vector3> g_velocities;
	std::vector<float> g_densities;
	std::vector<Vector4> g_anisotropy1;
	std::vector<Vector4> g_anisotropy2;
	std::vector<Vector4> g_anisotropy3;
	std::vector<Vector4> g_normals;
	std::vector<Vector4> g_diffusePositions;
	std::vector<Vector4> g_diffuseVelocities;
	std::vector<int> g_diffuseIndicies;
	std::vector<int> g_phases;
	std::vector<int> g_activeIndices;
	float g_diffuseScale;
	int g_diffuseActive;
	int g_waterActive;
	
	float g_windTime = 0.0f;
	float g_windFrequency = 0.1f;
	float g_windStrength = 0.0f;
	
	bool g_wavePool = false;
	float g_waveTime = 0.0f;
	float g_wavePlane;
	float g_waveFrequency = 1.5f;
	float g_waveAmplitude = 1.0f;
	float g_waveFloorTilt = 0.0f;
	
	Vector3 g_sceneLower;
	Vector3 g_sceneUpper;
	bool g_emit = false;
	std::vector<Emitter> g_emitters;
	
	
	float g_dt = 1.0f/60.0f;	// the time delta used for simulation
	float g_realdt;				// the real world time delta 
	int g_levelScroll;			// offset for level selection scroll area
	
	int g_frame = 0;
	int g_numSolidParticles = 0;
	
	bool g_profile = false;
	flexScene* g_scene;

    /** The simulation handler. */
	Flex::Flex(sceneName name);

    /** One step forward in the simulation. */
	void Flex::flexStep();

    /** Initialize the scene. */
	void Flex::Init();

    // Methods involved in the simulation.
	Vector3 Flex::safeNormalize(Vector3 v);
	void Flex::CreateParticleGrid(Vector3 lower, int dimx, int dimy, int dimz, float radius, Vector3 velocity, float invMass, bool rigid, float rigidStiffness, int phase, float jitter=0.005f);
	Vector3 Flex::RandomUnitVector();
	void Flex::GetParticleBounds(Vector3& lower, Vector3& upper);
	void Flex::ErrorCallback(FlexErrorSeverity, const char* msg, const char* file, int line);
	
    /** Returns water particle positions. To be read back and rendered. */
	Array<Vector3> Flex::getWaterPositions();

    /** Returns diffuse particle positions. To be read back and rendered. */
	Array<Vector4> Flex::getDiffusePositions();

    // returns the particle sizes
	float Flex::getWaterRadius();
	float Flex::getDiffuseRadius();

	// convexes
	std::vector<FlexCollisionGeometry> g_shapeGeometry;
	std::vector<Vector4> g_shapePositions;
	std::vector<Quat> g_shapeRotations;
	std::vector<Vector4> g_shapePrevPositions;
	std::vector<Quat> g_shapePrevRotations;
	std::vector<uint32_t> g_shapeStarts;
	std::vector<Vector4> g_shapeAabbMin;
	std::vector<Vector4> g_shapeAabbMax;
	std::vector<int> g_shapeFlags;
	void AddTriangleMesh(FlexTriangleMesh* mesh, const Vector3& translation, float scale);
	FlexTriangleMesh* Flex::CreateTriangleMesh(const ArticulatedModel::Mesh* mesh);
};
