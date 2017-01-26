/** \file App.cpp */
#include "App.h"

/*
Change Log:
- based on G3D sample code
- modified by Kenny, Yitong, Melanie, and Cole for the final
*/

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

float m_time = 0.0f;
PathTracer::Options m_options = PathTracer::Options();
const String App::WATER_ENTITY_NAME = "water";

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.caption             = argv[0];

    // Set enable to catch more OpenGL errors
    // settings.window.debugContext     = true;

    // Some common resolutions:
    // settings.window.width            =  854; settings.window.height       = 480;
    // settings.window.width            = 1024; settings.window.height       = 768;
    settings.window.width               = 1280; settings.window.height       = 720;
    //settings.window.width             = 1920; settings.window.height       = 1080;
    // settings.window.width            = OSWindow::primaryDisplayWindowSize().x; settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;

    // Set to true for a significant performance boost if your app can't render at 60fps, or if
    // you *want* to render faster than the display.
    settings.window.asynchronous        = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();
    settings.screenshotDirectory        = "./journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings), m_videoRecorder() {
}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    GApp::onInit();
    setFrameDuration(1.0f / 120.0f); 
    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default scene here.
    
    showRenderingStats      = false;

    makeGUI();
    // For higher-quality screenshots:
    // developerWindow->videoRecordDialog->setScreenShotFormat("PNG");
    // developerWindow->videoRecordDialog->setCaptureGui(false);
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));
    loadScene(
        //"G3D Sponza"
        //"particles" // Load something simple
        //"faucet"
		"Collision"
		//"goo"
		//"lightHouse"
		//"Fountain"
        //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
        );

    // set the particle sizes
	waterRadius = flex.getWaterRadius();
	diffuseRadius = flex.getDiffuseRadius();
}

void App::onAfterLoadScene(const Any &any, const String &sceneName) {
    Array<shared_ptr<VisibleEntity>> vEntities;
    scene()->getTypedEntityArray<VisibleEntity>(vEntities);
    Array<Vector3> vertices;

    // For every entity in the scene
    for (auto& e : vEntities) {

        // Only add rigid objects
        if (e->canChange()) { continue; }

        float x, y, z, ignore;
        e->frame().getXYZYPRDegrees(x, y, z, ignore, ignore, ignore);
        const Vector3 entityTranslation(x, y, z);

        shared_ptr<ArticulatedModel> model = dynamic_pointer_cast<ArticulatedModel>(e->model());
        if (isNull(model)) { continue; }

        // Add entities to the simulation scene so that they will be part of the simulation collisions
        const auto& meshes = model->meshArray();
        for (const auto& mesh : meshes) {
            auto* flexMesh = flex.CreateTriangleMesh(mesh);
            if (isNull(flexMesh)) { continue; }

            flex.AddTriangleMesh(flexMesh, entityTranslation, 1.0f);

            break; // For correctness, this line should be omitted. But for whatever reason, Flex segfaults if we omit it.
        }
    }

    flex.Init();
}


void App::makeGUI() {
    // Initialize the developer HUD (using the existing scene)
    createDeveloperHUD();
    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);
    developerWindow->cameraControlWindow->setVisible(false);

    GuiPane* interfacePane = debugPane->addPane("Interface");
    Array<String> labels = {"1x1","10x10","100x100","320x200","640x400", "1280×720"};
    interfacePane->addTextBox("Name",&m_options.name);
    interfacePane->addCheckBox("Save?", &m_options.save);
    interfacePane->addDropDownList("Resolution", labels, (int*) &m_options.resolution);
    interfacePane->addNumberBox("Rays per pixel", &m_options.raysPerPixel, "rays", GuiTheme::NO_SLIDER, 0, 10000, 1);
    interfacePane->addNumberBox("Max ray depth", &m_options.maxRayDepth, "", GuiTheme::NO_SLIDER, 0, 10000, 1);
    interfacePane->addCheckBox("Halve camera sensitivity", &m_options.lowerCameraSensitivity);
    interfacePane->addButton("Render Picture", [this](){
        drawMessage("Rendering...");
        Point2 dimensions;
        if (m_options.resolution == pixel) {
            dimensions = Point2(1,1);
        } else if (m_options.resolution == verysmall) {
            dimensions = Point2(10,10);
        } else if (m_options.resolution == small) {
            dimensions = Point2(100,100);
        } else if (m_options.resolution == medium) {
            dimensions = Point2(320,200);
        } else if (m_options.resolution == large) {
             dimensions = Point2(640,400);
        } else if (m_options.resolution == vLarge) {
             dimensions = Point2(1280,720);
        }
        shared_ptr<Texture> dst;
        float renderTime = traceImage(dst, dimensions);
        show(dst, format("Rendering Time: %f ", renderTime));
    });

    interfacePane->addNumberBox("Video length", &videoLength, "s", GuiTheme::NO_SLIDER, 0, 10000, 1);
    interfacePane->addButton("Render Video", [this](){
        Point2 dimensions;
        if (m_options.resolution == pixel) {
            dimensions = Point2(1,1);
        } else if (m_options.resolution == verysmall) {
            dimensions = Point2(10,10);
        } else if (m_options.resolution == small) {
            dimensions = Point2(100,100);
        } else if (m_options.resolution == medium) {
            dimensions = Point2(320,200);
        } else if (m_options.resolution == large) {
             dimensions = Point2(640,400);
        } else if (m_options.resolution == vLarge) {
             dimensions = Point2(1280,720);
        }
        m_videoRecorder.startRecording(dimensions, m_options.name, videoLength);
    });

     interfacePane->addButton("Stop Video", [this](){
        m_videoRecorder.numFrames = 1;
    });

    if (false) {
        developerWindow->profilerWindow->setVisible(true);
        Profiler::setEnabled(true);
    }

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}

float App::traceImage(shared_ptr<Texture>& dst, Point2 dimensions) {
    shared_ptr<Image> img = G3D::Image::create(dimensions.x, dimensions.y,ImageFormat::RGB32F());

    // Select the caustic map to use
    int num = int(m_videoRecorder.numFrames) % 32 + 1;
    const shared_ptr<Image> causticMap = Image::fromFile(format("data-files/waterCaustic/waterCaustic_0%d%d.jpg",num / 10,num % 10));

    // Start the path-tracer
    PathTracer tracer = PathTracer(scene(), activeCamera(), img, m_options, causticMap);
    Stopwatch clock;
    clock.tick();
    tracer.pathTrace();
    clock.tock();
    float renderTime = clock.elapsedTime();
    debugPrintf("Rendering Time: %f\n", renderTime);
    const shared_ptr<Texture>& src = Texture::fromImage("Source", img, ImageFormat::RGB32F());

    // post-process the image
    m_film->exposeAndRender(renderDevice, activeCamera()->filmSettings(), src, 0, 0, dst);
    if (m_options.save) {
        const String name = format("%s%s", m_options.name, ".png");
        shared_ptr<Image> savedImage = dst->toImage(ImageFormat::RGB32F());
        savedImage->convert(ImageFormat::RGB8()); //have to convert the image back to RG8 which means a loss of precision.
        savedImage->save(name);
    }
    return renderTime;
}

// This default implementation is a direct copy of GApp::onGraphics3D to make it easy
// for you to modify. If you aren't changing the hardware rendering strategy, you can
// delete this override entirely.
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (!scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (!rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }

    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);
    m_gbuffer->resize(m_framebuffer->width(), m_framebuffer->height());
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_renderer->render(rd, m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : shared_ptr<Framebuffer>(),
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces);

    // Debug visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        // Call to make the App show the output of debugDraw(...)
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        drawDebugShapes();
        const shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : shared_ptr<Entity>();
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        // Post-process special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

        m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_EXPRESSIVE_MOTION),
            m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
            m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    rd->clear();

    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.colorGuardBandThickness.x + settings().hdrFramebuffer.depthGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x);



    // Save videos from our path-tracer
    if (m_videoRecorder.numFrames > 0) {
        debugPrintf(format("\nRendering frame %d \n", m_videoRecorder.numFrames).c_str());
        shared_ptr<Texture> dst = Texture::createEmpty(format("%d", m_videoRecorder.numFrames), m_videoRecorder.dimensions.x, m_videoRecorder.dimensions.y, ImageFormat::RGB32F());
        traceImage(dst, m_videoRecorder.dimensions);
        m_videoRecorder.recordFrame(dst);
    }
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);
	
	if (m_skipAhead) {
        // Skip ahead 100 simulation steps. Useful for scenes that are very slow to simulate.
		for(int i =0; i <100; ++i){
		    flex.flexStep();
		}		
		m_skipAhead = false;
	} else if (m_time > 1.0f && m_isSimulating){
        // Take one simulation step
		flex.flexStep();

        // Update the scene
		Array<Vector3> points = flex.getWaterPositions();
		m_waterModel.addWaterToScene(points, scene(), waterRadius, waterRadius * stepRatio);
		Array<Vector4> Dpoints = flex.getDiffusePositions();
		m_waterModel.addDiffuseToScene(Dpoints, scene(), diffuseRadius, diffuseRadius*stepRatio);
	}

    // Update simulation time
	m_time += sdt;

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }

    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { 
        shared_ptr<DefaultRenderer> r = dynamic_pointer_cast<DefaultRenderer>(m_renderer);
        r->setDeferredShading(! r->deferredShading());
        return true; 
    }
    
    // Pause/restart the simulation by pressing 'l'
    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'l')) { 
        m_isSimulating = !m_isSimulating;
    }

    // Jump forward 100 simulation steps by pressing 'e'
	if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'e')) { 
        m_skipAhead = true;
    }

    return false;
}


void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);
    (void)ui;
    // Add key handling here based on the keys currently held or
    // ones that changed in the last frame.
}


void App::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    GApp::onPose(surface, surface2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}

void App::saveScene() {
    // Add the water to the scene
    auto waterEntity = scene()->entity(WATER_ENTITY_NAME);
    if (waterEntity) {
        scene()->removeEntity(WATER_ENTITY_NAME);
    }
    GApp::saveScene();
    if (waterEntity) {
        scene()->insert(waterEntity);
    }
}