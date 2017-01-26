/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include "flex.h"
#include "MCubes.h"
#include "WaterModel.h"
#include "PhysFlex.h"
#include "PathTracer.h"
#include "Video.h"

/* Change Log:
    - based on G3D sample code
    - modified by Melanie, Kenny, Yitong, and Cole for the final
*/

/** \brief Application framework. */
class App : public GApp {
protected:
    /** The name of the visible entity used for the water model. */
    static const String WATER_ENTITY_NAME;
    
    /** Whether the simulation should execute in the next step. */
    bool m_isSimulating = true;

    /** Whether the simulation should fast-forward during the next simulation step. */
	bool m_skipAhead = false;

    /** Object used for rendering the water model and the diffuse particles every scene. */
    WaterModel m_waterModel;

    int videoLength = 0;
    VideoRecorder m_videoRecorder;

	// sceneName Sname = waves;
	// sceneName Sname = bunny;
	// sceneName Sname = sprout;
	// sceneName Sname = goo;
	// sceneName Sname = lightHouse;
	// sceneName Sname = fountain

    /** Interface to the NVIDIA Flex particle system */
	Flex flex = Flex(waves);

	float waterRadius; // radius of water particles
	float diffuseRadius; // radius of diffuse particles

    /** Step parameter for marching cubes. Set to .5 for no holes, .8 for faster but some holes, 1 if you're a madman (or madwoman). */
	float stepRatio = .5f;

    /** Called from onInit */
    void makeGUI();

    /** Populates the dst image with a path-traced image representing the scene. Returns time it took to render image. */
    float traceImage(shared_ptr<Texture>& dst, Point2 dimensions);
public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
        
    virtual void onAfterLoadScene(const Any &any, const String &sceneName) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual bool onEvent(const GEvent& e) override;
    virtual void onUserInput(UserInput* ui) override;

    virtual void saveScene() override;
};
