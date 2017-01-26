/*
Change Log:
- created by Kenny and Melanie for the paths lab
- modified by Kenny, Yitong, Melanie, and Cole for the final
*/


/** \file PathTracer.cpp */
#include "PathTracer.h"
#include "iostream"
#include "fstream"

PathTracer::PathTracer(
    const shared_ptr<Scene>& s, 
    const shared_ptr<Camera>& c, 
    const shared_ptr<Image>& i,
    const Options& o,
    const shared_ptr<Image>& m
) : m_scene(s),
    m_camera(c),
    m_image(i),
    m_options(o),
    m_lightArray(m_scene->lightingEnvironment().lightArray),
    m_width(m_image->width()),
    m_height(m_image->height()),
    m_causticMap(m)
{
    // Set up the TriTree for the scene
    Array<shared_ptr<Surface>> surfaceArray;
    m_scene->onPose(surfaceArray);
    m_tritree.setContents(surfaceArray); 

    // Set up a separate TriTree with only solid surfaces for shadow-casting
    Array<shared_ptr<Surface>> rigidSurfaces;
    for (int m = 0; m < surfaceArray.size(); ++m) {
        if (!surfaceArray[m]->hasTransmission()) {
            rigidSurfaces.append(surfaceArray[m]);
        }
    }
    m_rigidTriTree.setContents(rigidSurfaces);
    m_rigidTriTreeSize = m_rigidTriTree.size();
}

void PathTracer::pathTrace() {
    m_skybox = m_scene->skyboxAsCubeMap();

    // buffers
    Array<Ray> rayBuffer;
    Array<Radiance3> modulationBuffer;
    Array<shared_ptr<Surfel>> surfelBuffer; 
    Array<Biradiance3> biradianceBuffer;
    Array<Ray> shadowRayBuffer;
    Array<bool> lightShadowedBuffer;
    Array<Point3> extinctionPointBuffer;
    Array<bool> inMediumBuffer;

    // init all buffers to m_image.width*m_image.height
    const int imageDim = m_width * m_height;
    rayBuffer.resize(imageDim);
    modulationBuffer.resize(imageDim);
    surfelBuffer.resize(imageDim);
    biradianceBuffer.resize(imageDim);
    shadowRayBuffer.resize(imageDim);
    lightShadowedBuffer.resize(imageDim);
    extinctionPointBuffer.resize(imageDim);
    inMediumBuffer.resize(imageDim);

    // No point hit on the first cast should be in Medium
    inMediumBuffer.setAll(false);

    m_image->setAll(Radiance3::black());

    for(int r = 0; r < m_options.raysPerPixel; ++r){
        initializeModulationBuffer(modulationBuffer);
        generateRayBuffer(rayBuffer);

        // Progress bar
        debugPrintf("Rendering... %f percent done \n", 100 * (float)r / (float)m_options.raysPerPixel);

        for(int d = 0; d < m_options.maxRayDepth; ++d){
            findIntersection(rayBuffer, surfelBuffer);
            chooseLight(surfelBuffer, biradianceBuffer, shadowRayBuffer);
            testVisibilty(shadowRayBuffer, lightShadowedBuffer);
            
            L_i(modulationBuffer,surfelBuffer, rayBuffer, biradianceBuffer, lightShadowedBuffer, shadowRayBuffer, extinctionPointBuffer, inMediumBuffer);
            
            generateRecursiveRay(surfelBuffer, rayBuffer, modulationBuffer, r, d, extinctionPointBuffer, inMediumBuffer);
        }
    }
    lowerCameraSensitivity();
}

void PathTracer::lowerCameraSensitivity() const {
    if (!m_options.lowerCameraSensitivity) { return; }
    Thread::runConcurrently(Point2int32(0, 0), Point2int32(m_width, m_height), [&](Point2int32 point) {
        Color3 color;
        m_image->get(point, color);
        m_image->set(point, color / (float) 2);
    });
}

void PathTracer::L_i(Array<Radiance3>& modulationBuffer, const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<Ray>& rayBuffer, const Array<Biradiance3>& biradianceBuffer, Array<bool>& lightShadowedBuffer, Array<Ray>& shadowRayBuffer, const Array<Point3>& extinctionPointBuffer, Array<bool>& inMediumBuffer) const {
    Thread::runConcurrently(Point2int32(0, 0), Point2int32(m_width, m_height),[&](Point2int32 point) {
        const int i = point.x + (m_width * point.y);
        const Vector3 w_i = -1 * shadowRayBuffer[i].direction();
        const Vector3 w_o = -1 * rayBuffer[i].direction();
        Color3 c(0.0f);
        float alpha = 1.0f;

        // Set the color of the liquid and shadowColor should be waterColor/2
        Color3 waterColor(0, 0.02, 0.15);
        Color3 shadowColor(0, 0.01, 0.08);
        Color3 foamColor(0.95f, 0.95f, 1.0f);

        // Check to see if we hit the sky
        if (isNull(surfelBuffer[i])) {  
            m_image->increment(point, backgroundRadiance(rayBuffer[i]) * modulationBuffer[i]);
            return;
        }
        
        // Check the kappaPos tag to see did we hit a foam particle?
        if (surfelBuffer[i]->kappaPos == foamParticleTag) {
            // Between 0 and 1. is 1 when shadingNormal and eyeRay are exactly opposite.
            float cosTerm = surfelBuffer[i]->shadingNormal.dot(-rayBuffer[i].direction());
            cosTerm *= cosTerm;

            // Shade foam particles such that they are whiter in the center and fade to the color behind them on the edges based on how transmissive they are.
            // This is a hack to avoid the rendering cost of making each foam particle transmissive with an extinction coefficient.
            m_image->increment(point, cosTerm * foamColor * surfelBuffer[i]->kappaNeg.r * modulationBuffer[i]);
            return;
        }
        
        // If the point is in the liquid, shade for light absorption
        if (inMediumBuffer[i]) {
            inMediumBuffer[i] = surfelBuffer[i]->transmissive(); // we've probably exited the liquid
            
            // Calculate the Beer-Lambert modifier
            const Vector3& inMediumVector = surfelBuffer[i]->position - extinctionPointBuffer[i];
            const float inMediumDistance = inMediumVector.length();
            alpha = extinctionFunction(inMediumDistance);

            // Add color as the light is absorbed
            c += (1.0f - alpha) * waterColor;

            // Map to the caustic texture when hit a roughly horizontal surface
            Color3 causticLight;
            int width = m_causticMap->width();
            int height = m_causticMap->height();
            const int inverseCausticSize = 150; // multiply by more to go smaller
            m_causticMap->get(Point2int32(int(abs(surfelBuffer[i]->position.x * inverseCausticSize)) % width, int(abs(surfelBuffer[i]->position.z * inverseCausticSize)) % height), causticLight);
            c += max(alpha * causticLight * surfelBuffer[i]->geometricNormal.dot(Vector3(0,1,0)), Color3(0.0f)); //caustics are brightest when the surfel's normal == Vector3(0,1,0)
            
            // Handle areas of liquid in shadow
            if (lightShadowedBuffer[i]) {  
                // Add ambient light
                m_image->increment(point, ((1.0f - alpha) * shadowColor + alpha * surfelBuffer[i]->reflectivity(Random::threadCommon()) * 0.05f) * modulationBuffer[i]);

                // Any future light contributed will be as if seen through the water so we need to weight it appropriately.
                // This is not physically accurate but looks good.
                modulationBuffer[i] *= alpha;
                modulationBuffer[i] += (1.0f-alpha) * shadowColor / m_options.raysPerPixel;
                return;
            }
        } 
        else if(lightShadowedBuffer[i]) {    
            // Handle shadows outside of the liquid
            m_image->increment(point, alpha * surfelBuffer[i]->reflectivity(Random::threadCommon()) * 0.05f*modulationBuffer[i]);
            return;
        }
        
        // Calculate surfel color
        const Radiance3& emit = surfelBuffer[i]->emittedRadiance(w_o);
        const Radiance3& birad = biradianceBuffer[i];
        const float cosTerm = abs(surfelBuffer[i]->shadingNormal.dot(w_i));
        const Color3& bsdf = surfelBuffer[i]->finiteScatteringDensity(w_i, w_o);

        Color3 temp_c = (emit + birad * bsdf * cosTerm);
        temp_c += surfelBuffer[i]->reflectivity(Random::threadCommon()) * 0.05f;

        // Beer-Lambert shading
        c += alpha * temp_c;
        m_image->increment(point, c * modulationBuffer[i]);

        // Any future light contributed will be as if seen through the water so we need to weight it appropriately.
        // This is not physically accurate but looks good.
        modulationBuffer[i] *= alpha;
        modulationBuffer[i] += (1.0f-alpha) * waterColor / m_options.raysPerPixel;
    });
}

void PathTracer::initializeModulationBuffer(Array<Radiance3>& modulationBuffer) const {
    Thread::runConcurrently(0, modulationBuffer.size(), [&](int i) {
        modulationBuffer[i] = Radiance3( 1 / (float)m_options.raysPerPixel );
    });
}


Radiance3 PathTracer::backgroundRadiance(const Ray& ray) const {
     return m_skybox->bilinear(ray.direction()).rgb();
}

void PathTracer::generateRayBuffer(Array<Ray>& rayBuffer) const {
    Thread::runConcurrently(Point2int32(0, 0), Point2int32(m_width, m_height), [&](Point2int32 point) {
        if (m_options.raysPerPixel == 1) {
            // Start in the center of the pixel
            rayBuffer[point.x + point.y * m_width] = m_camera->worldRay(
                point.x + 0.5f,
                point.y + 0.5f,
                m_image->bounds()
            );
        } else {
            // Start at random points within the pixel
            Random& rng = Random::threadCommon();
            rayBuffer[point.x+(point.y*m_width)] = m_camera->worldRay(
                point.x + rng.uniform(),
                point.y + rng.uniform(),
                m_image->bounds()
            );
        }
    });
}

void PathTracer::findIntersection( const Array<Ray>& rayBuffer, Array<shared_ptr<Surfel>>& surfelBuffer) const {
    m_tritree.intersectRays(rayBuffer, surfelBuffer, TriTree::COHERENT_RAY_HINT); 
}

void PathTracer::chooseLight(const Array<shared_ptr<Surfel>>& surfelBuffer, Array<Biradiance3>& biradianceBuffer, Array<Ray>& shadowRayBuffer) const {
    Thread::runConcurrently(0, biradianceBuffer.size(), [&](int i) {
        if (isNull(surfelBuffer[i])) return;

        //For efficiency, if there is only one light, select it
        if (m_lightArray.size() == 1) {
            biradianceBuffer[i] = m_lightArray[0]->biradiance(surfelBuffer[i]->position);

            const Point3& P0 = m_lightArray[0]->position().xyz();;
            const Point3& P1 = surfelBuffer[i]->position;
            const float len = (P1-P0).length() - .0001f;
            shadowRayBuffer[i] = Ray::fromOriginAndDirection(P0, (P1-P0) / len, 0.0f, len - 1e-3f).bumpedRay(.0001f);
            return;
        }

        float totalBiradiance = 0.0f;
        for (int j = 0; j < m_lightArray.size(); ++j) {
            totalBiradiance += m_lightArray[j]->biradiance(surfelBuffer[i]->position).sum();      
        }
        
        Random& rng = Random::threadCommon();
        float r = totalBiradiance * rng.uniform();
        for (int j = 0; j < m_lightArray.size(); ++j) {
            const Biradiance3 birad = m_lightArray[j]->biradiance(surfelBuffer[i]->position);
            r -= birad.sum();
            if (r < 0) {
                biradianceBuffer[i] = birad * (totalBiradiance / birad.sum());

                const Point3& P0 = m_lightArray[j]->position().xyz();;
                const Point3& P1 = surfelBuffer[i]->position;
                const float len = (P1-P0).length() - 0.0001f;
                shadowRayBuffer[i] = Ray::fromOriginAndDirection(P0, (P1-P0) / len, 0.0f, len - 1e-3f).bumpedRay(0.0001f);
                break;
            }
        }
    });

}

void PathTracer::testVisibilty(const Array<Ray>& shadowRayBuffer, Array<bool>& lightShadowedBuffer) const {
    // Only test for shadows from opaque objects.
    if (m_rigidTriTreeSize == 0) {
        return;
    }
    m_rigidTriTree.intersectRays(shadowRayBuffer, lightShadowedBuffer, TriTree::OCCLUSION_TEST_ONLY | TriTree::DO_NOT_CULL_BACKFACES |  TriTree::COHERENT_RAY_HINT);
}

void PathTracer::generateRecursiveRay(const Array<shared_ptr<Surfel>>& surfelBuffer, Array<Ray>& rayBuffer, Array<Radiance3>& modulationBuffer, const int r, const int d, Array<Point3>& extinctionPointBuffer, Array<bool>& inMediumBuffer) const {
    Thread::runConcurrently(0, surfelBuffer.size(), [&](int i) {
        if (isNull(surfelBuffer[i])) {
            // for rays the hit the sky don't keep adding to pixel value
            modulationBuffer[i] = Radiance3::black();   
            return;
        }; 

        // Save the point hit so that later the distance traveled through the liquid can be calculated
        extinctionPointBuffer[i] = surfelBuffer[i]->position;

        // Get impulses
        Vector3 w_after;
        const Vector3 w_before = -rayBuffer[i].direction();
        Surfel::ImpulseArray impulseArray;
        surfelBuffer[i]->getImpulses(PathDirection::EYE_TO_SOURCE, w_before, impulseArray);

        // If no impulses, cast no further rays
        if (impulseArray.size() <= 0) {
            return;
        };

        // Selects whether to reflect or refract. This adds some determinism into the path-tracing because the
        // first ray will always go one way and the second will always go the other way, etc. However, this
        // guarantees that if you have 2^numBounces rays then you will explore all paths as done in Whitted
        // ray-tracing. Ultimately this approach looked good and was fast enough.
        int a = (r / (d + 1)) % impulseArray.size();

        float prob = 1.0f / impulseArray.size(); // probability of scattering this direction
        w_after = impulseArray[a].direction;

        const shared_ptr<Surfel>& surf = surfelBuffer[i];

        // Check if we hit foam
        if (surf->kappaPos == foamParticleTag) {
            //kappaNeg.b doesn't actually store what you think it stores. it stores the diffuse particle radius size
            const Point3& origin = surf->position + -(surf->geometricNormal * surf->kappaNeg.b * 1.01);
            //if we hit a diffuse particle, in order to get the color behind the diffuse particle, we need to cast another ray from behind the particle in the same direction
            rayBuffer[i] = Ray::fromOriginAndDirection(origin, rayBuffer[i].direction());
            return;
        }

        // Check if we're in the liquid
        float k = sign(w_after.dot(surf->geometricNormal));
        if (k <= 0) {
            //we're refracting
            inMediumBuffer[i] = surf->kappaNeg != Color3::black();
        } else {
            //we're reflecting
            inMediumBuffer[i] = surf->kappaPos != Color3::black();
        }

        modulationBuffer[i] *= impulseArray[a].magnitude / prob;
        const Point3& origin = surf->position + surf->geometricNormal * .01f * k;
        //bumped ray origin
        rayBuffer[i] = Ray::fromOriginAndDirection(origin, w_after);
    });
}
