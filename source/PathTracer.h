/*
Change Log:
 - Created by Kenny and Melanie for the path-tracer lab
 - modified by Kenny, Yitong, Melanie, and Cole for the final
 */

#pragma once
#include <G3D/G3DAll.h>

enum Resolution {pixel, verysmall, small, medium, large, vLarge};

/* All foam particles will have their kappaPos set to foamParticleTag.
 * This is simply a way of flagging the particles so that we can do appropriate shading
 * on them. 
 */
static Color3 foamParticleTag(0.314f, 0.159f, 0.265f);

/** Performs the path-tracing algorithm. **/
class PathTracer {
private:
    /* returns an alpha extinction value from 0 to 1*/
    static float extinctionFunction(float length) {
        const float waterExtinctionCoefficient = 3.0f; // hgiher values mean the water will absorb light faster
        return clamp(exp(-(length * waterExtinctionCoefficient)), 0.0f, 1.0f);
    }

public:
    
    shared_ptr<CubeMap> m_skybox;
    /** Keeps track of important parameters for the path-tracing algorithm. **/
    class Options {
    public:
        Options() {}

        Resolution resolution = Resolution::large;
        String name = "beautifulWaves";
        bool save;

        int raysPerPixel = 32; // in general the raysPerPixel should be 2^(maxRayDepth + 1)
        int maxRayDepth = 5; // must be >=3 to pass through the water fully
        bool lowerCameraSensitivity = true;
    };

    /** Sets the scene and intializes key member variables and data structures. **/
    PathTracer::PathTracer
       (const shared_ptr<G3D::Scene>& s, 
        const shared_ptr<Camera>& c, 
        const shared_ptr<Image>& i,
        const Options& o,
        const shared_ptr<Image>& m);

    /** Starts the path-tracing. **/
    void pathTrace();

   /** Computes the Radiance3 of the light coming in along the ray specified in the parameters. Depth is used in indirect lighting to set our branch factor*/
    void L_i
       (Array<Radiance3>&                                       modulationBuffer, 
        const  Array<shared_ptr<Surfel>>&                       surfelBuffer,  
        const Array<Ray>&                                       rayBuffer, 
        const Array<Biradiance3>&                               biradianceBuffer,  
        Array<bool>&                                            lightShadowedBuffer,
        Array<Ray>&                                             shadowRayBuffer,
        const Array<Point3>&                                    extinctionPointBuffer,
        Array<bool>&                                            inMediumBuffer) const;

    /*Color gradient for background*/
    Radiance3 backgroundRadiance
       (const Ray&                                             ray) const;

    /*updates rayBuffer for every ray*/
    void generateRayBuffer
       (Array<Ray>&                                             rayBuffer) const;

    /*updates surfelBuffer for every ray*/
    void findIntersection
       (const Array<Ray>&                                       rayBuffer,  
        Array<shared_ptr<Surfel>>&                              surfelBuffer) const;

    /*updates biradienceBuffer as well as shadowRayBuffer*/
    void chooseLight
       (const Array<shared_ptr<Surfel>>&                        surfelBuffer, 
        Array<Radiance3>&                                       biradianceBuffer,  
        Array<Ray>&                                             shadowRayBuffer) const;

    /*updates lightShadowedBuffer*/
    //expects shadow rays to be bumped
    void testVisibilty
       (const Array<Ray>&                                       shadowRayBuffer,  
        Array<bool>&                                            lightShadowedBuffer) const;

    /*uses surfelBuffer to generate new rayBuffer*/
    void generateRecursiveRay
       (const  Array<shared_ptr<Surfel>>&                       surfelBuffer,  
        Array<Ray>&                                             rayBuffer,  
        Array<Radiance3>&                                       modulationBuffer,
        const int                                               r,
        const int                                               d,
        Array<Point3>&                                          extinctionPointBuffer,
        Array<bool>&                                            inMediumBuffer) const;

    /** sets or resets the modulation buffer to hold 1 / raysPerPixel. Used to average the returned colors from all of the ray casts from a single pixel*/
    void  initializeModulationBuffer
       (Array<Radiance3>&                                       modulationBuffer) const ;

    /** effectively lowers the camera sensitivity by dividing each pixel's color value in half */
    void lowerCameraSensitivity() const;

    // member variables
    Options m_options;
    
    shared_ptr<Scene> m_scene;
    shared_ptr<Camera> m_camera;
    shared_ptr<Image> m_image;
    shared_ptr<Image> m_causticMap;
    Array<shared_ptr<Light>> m_lightArray;
    TriTree m_tritree;
    TriTree m_rigidTriTree;
    int m_width; 
    int m_height;
    int m_rigidTriTreeSize;
};