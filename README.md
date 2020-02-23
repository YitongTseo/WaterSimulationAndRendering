# Water Simulation

Melanie Subbiah, Kenny Jones, Yi-Tong Tseo, and Cole Erickson

mss3@williams.edu, rkj2@williams.edu, kyt2@williams.edu, and nde1@williams.edu


![No man is an Island. Simulated with our particle system. Rendered using the G3D default renderer.](/doc-files/contrastLighthouse.jpg)

**“She felt... how life, from being made up of little separate incidents which one lived one by one, became curled and whole like a wave which bore one up with it and threw one down with it, there, with a dash on the beach.” **

― Virginia Woolf, To the Lighthouse



![Video](/doc-files/video_screenshot.png)](https://www.youtube.com/watch?v=FS6nkQwO7pY)



# Introduction
=====================================================

Water is difficult to simulate because it is fluid and changing, but its movement is based on the physics of interaction between individual molecules. Relatively still bodies of water can be generated using noise functions to simulate waves, but to produce different water effects such as realistic waves and foam, it is necessary to actually use a simulation based on interactions between lots of particles (similar to a higher-level version of molecules). The water surface can be rendered based on the positions of these particles. Our project focuses on creating realistic computer generated videos of water in different contexts. To achieve this goal, we had to overcome three main challenges: running an accurate particle simulator for the physics of the water, converting this particle simulation into a mesh for rendering, and rendering the light effects involved with water, all within reasonable time constraints.

We used the NVIDIA Flex library to make a position-based dynamics (PBD) simulation for the water. This library also handles diffuse particles, which we were able to mark as foam particles and render as individual small spheres. For the rest of the water, we converted the particles into a surface triangle mesh using the marching cubes algorithm. To render water beautifully, we would need to take an approach like bidirectional path-tracing (recently used to render water scenes in Pixar's Finding Dory), but this approach is computationally expensive and time-consuming to reach convergence. Therefore, we used a Whitted-ray-tracingesque version of path-tracing to achieve the necessary reflection and refraction, used a texture map as a proxy for caustics, and used the Beer-Lambert equation (with minor tweaks) for extinction coloring.


# Specification
=====================================================

1. Run a particle simulation for water by integrating with the NVIDIA Flex library.

    - Create a particle system with particles.  
    - Use Flex diffuse particles for foam.
    - Include rigid object collision.

2. Generate geometry for the simulation information.

    - Implement marching cubes to convert the particle simulation into a triangle mesh for the water surface. Search only grid space near particles and keep the cell size to about half the radius of a particle.
    - Assign appropriate material values to the water surface mesh.
    - Integrate with the Flex diffuse particles to create sphere entities for the foam particles with different alpha values for the lifetime of the diffuse particles.

3. Render water appropriately.

    - Modify path-tracing to scatter only along impulses.
    - Use a texture map to approximate caustics.
    - Adjust coloring for light absorption using the Beer-Lambert equation.
    - Shade foam so that it fades into the the background at its edges.
    - Hook up rendering to automatically dump a rendered video of the simulation.
    - Render a simple scene (such as the bunny with goo) in under 15 seconds at 1280x720 resolution.

4. Demonstrate correctness.

    - Video with particles visualized as spheres.
    - Video of just foam particles.
    - Image of marching cubes mesh for water surface.
    - Rendering of Cornell Box spheres scene.
    - Rendering of realistic water.


# Topic Overview
=====================================================

This project involves three main stages: particle simulation, mesh generation, and rendering.

Particle simulation is a common technique used in computer graphics to approximate natural phenomena, such as explosions, sand, and fluids. Particles are a good approximation of these phenomena because, in the physical world, they are the product of interactions between many moving parts. For example, water movement is governed by molecular interactions, sand shifts based on dynamics between each grain, and explosions take previously rigid objects and separate them into lots of small pieces. A particle simulation can therefore reasonably capture the movement involved in these dynamic systems, and then geometry and rendering can be used to create the aesthetics associated with that movement. There are many different approaches to particle simulation as any computational system is an approximation of the real-world physics. For this project, we used the NVIDIA Flex library, which performs real-time simulation computation on the GPU. This library uses the Position Based Dynamics approach to simulation, which uses force information to calculate velocity, position, and collision effects. The simulation creates a particle system, for which you can specify initial particle positions and velocities and specify if there should be a continuous release point (emitter) for the system. This system also creates diffuse particles, or foam, when the relative velocities and kinetic energy of adjacent particles crosses a certain threshold.

To transform this particle system data into a usable mesh for rendering, we used the marching cubes algorithm. This algorithm imposes a grid on 3D space and then searches through each grid cube, assigning vertex values based on how close that vertex is to nearby particles. Each grid cube is then mapped to an existing table of configurations for vertex values which determines where to slice the cube with a mesh surface. The slicing is interpolated along the cube edges based on relative vertex values so that the produced surface is smooth. This computation is expensive, especially as the grid cells get smaller and smaller, but each grid cell is only considered once. At the end of the marching cubes step, we have a mesh representing the water surface, but we chose to keep the diffuse (foam) particles as spheres since they are often separated from the rest of the water surface or sitting on top and have different material properties.

Beautiful and physically accurate water rendering involves techniques like bidirectional path-tracing or photon mapping in order to pick up on all high-transport paths that produce important caustics. However, this kind of rendering would be too time-intensive for this project, so we used a path-tracer modeled after Whitted ray-tracing, a technique which involves scattering only the reflection and refraction rays off of a surface so as to save on computation. We modified our path-tracer to only scatter along impulses and we made sure that as long as 2^numBounces rays were cast per pixel then all possible paths through these impulses would be explored. However, this coding meant that there was some determinism to the paths as the first ray would always reflect while the second ray would refract, etc. We then used a texture mapping to create the illusion of caustics which is effective as long as the water is choppy, denying the viewer the chance to carefully examine the physical accuracy of the caustics. For foam particle shading, we fade the particle's edges into the background by casting a ray through the particle to see what color that pixel would be if there was no foam there. This approach makes the foam look softer without having to spend the rendering time involved with making the particles transmissive. The other complication with water rendering is that light gets absorbed gradually by water which produces what we think of as hue, or the darkening of the water color in deeper areas. We handle this hue coloring with the Beer-Lambert equation, which multiplies the biradiance value for a surface by an exponential value based on the index of refraction of the medium and the distance the light will travel through the medium before exiting again. Finally, we only cast shadows from opaque entities in the scene.


# Design
=====================================================

Our App class has the general G3D app functionality and our custom GUI code which allows for easy adjustment of rendering parameters. The render button in the GUI has a lamda function that renders and records frames for videos of our simulation. This rendering is handled by our PathTracer class. The particle simulation is handled by the NVIDIA Flex library, which is triggered from the App class. The water surface generation code lives in our MarchingCubes class, which produces a mesh that is then passed to the WaterModel class to set material properties and add the water model and foam particles to the scene. 


By splitting our design into multiple classes, we are able to isolate and organize components that have different functionality. The App class is largely used to coordinate the other classes, leaving much of the heavy lifting to other parts of the code. It was really useful to separate the remaining functionality between different classes because many of the techniques used in this project needed to be developed in isolation and could easily be used in future projects. We now can easily add simulation, real-time rendering, or marching cubes to any future project by adding the appropriate class from this project. This approach also made it easy to approach optimizing each of these steps because we knew they couldn't happen simultaneously, so we could easily isolate the performance of each class/algorithm. All of these decisions combine to create a design which is not only functional for this project, but will allow us to extend on these components in the future.



           **Data Flow Through Classes:**
******************************************************
*  .-------.                     .------------.      *
* |  Flex   |<-------+   +------>| WaterModel |      * 
*  '-+---+-'         |   |       '-+---+------'      *    
*    |   |    d      |   |         |   |  ^          *
*   b|   +-------+  a|   |e  +-----+  f|  |g         *
*    |           |   |   |   | h       |  |          *
*    v           v   |   |   v         v  |          *
* .--+------.  .-+---+---+---+-.    .-----+--.       *
* |FlexScene|  |      App      |   |  MCubes  |      *
* '--+------'  '---+---+----+--'    '--------'       *
*    |             |   ^    |                        *
*   c|            i|  j|    +--------+    +----->    *
*    v             v   |          k  v    |  l       *
*  .-+-----.    .--+---+-----.    .--+----+-------.  *
* | Emitter |   | PathTracer |    | videoRenderer |  *
*  '-------'    '------------'    '-------+-------'  *
******************************************************      

*Flex, Emitter, and MCubes include outside code 

| Step | Transferred Data |
|-------|-----|
|   a|  create a flex object (that then interfaces with Nvidia Flex)|
|   b|  specify simulation parameters|
|   c|  add emitters to flexScene|
|   d|  return particle positions|
|   e|  diffuse particle and water particle positions|
|   g|  return mesh|
|   h|  writes an obj to the scene|
|   i|  user settings|
|   j|  return image object  |
|   k|  give texture objects |
|   l|  outputs .mp4 file |


**DataFlow Overview**

**Flex Integration**
    
The core of NVidia’s Flex engine is found in the files flex.h and flexExt.h. The principal idea of Flex is that there is a central FlexSolver, initialized at the beginning of any particle simulation, which keeps track of all particles in the scene. Once a FlexSolver is created, specific parameters that affect the simulation can be set by calling flexSetParams and passing a FlexParams object filled with the desired values. To get the particle positions, we initialize a chunk of memory as a positions array and then feed this array into the Solver with the flexSetParticles function. We then tell the solver to take a step forward with the function flexUpdateSolver, which advances the function forward in time by a parameter specified amount. To finally read back the changed particle positions, we call flexGetParticles, and our previous particle position array gets updated with the values from the simulation. Our code was heavily borrowed from the demo project that comes with the Flex engine, and we share a very similar although simplified code structure.

Note: because of our project's Flex the lib/x64/ folder must be copied from the flux Demo into the source/ folder.

## Correctness Results
=====================================================
Video with particles visualized as spheres

![](/doc-files/particlesAsSpheres.mp4) 

Visualization of just the foam particles on top of a wave. Note how they fade then disappear.

![](/doc-files/2016-11-30_005_CubeMarch_r2566_g3d_r983__Good_correctness_results_here.mp4)

Water surface mesh wireframe created with marching cubes.

![](/doc-files/meshExample.jpg) 

Rendering of the Cornell Box spheres scene with our path-tracer with 5 bounces and 32 paths/pixel. Demonstrates reflection and refraction are handled correctly.

![](/doc-files/cornellBoxMirrors.png) 

Rendering of a test scene with our path-tracer at 1280x720 resolution with 5 bounces and 32 paths/pixel. Demonstrates reflection, refraction, shadows from the triangle within the water, shadows from the triangle above the scene, and extinction coloring creating a gradient along the slanted triangle. This image rendered in 11 seconds.
![](/doc-files/testBox.png)
  
Rendering of water with our path-tracer with 5 bounces and 32 paths/pixel. Demonstrates extinction coloring from Beer-Lambert and fake caustics.
      
![](/doc-files/waterRendered.png)


## Quality Results
=====================================================

Overfloweth the Chalice of Knowledge

[](/doc-files/keep3.mp4)

Any Port in the Storm

[](/doc-files/keep7.mp4)

Love, War, and Everything in Between

[](/doc-files/keep8.mp4)

Cool Marbles

[](/doc-files/keep9.mp4)

Versaille Amplified

[](/doc-files/keep4.mp4)

Death Cab for Cutie

[](/doc-files/keep6.mp4)

Nickelodeon Nostalgia

[](/doc-files/keep5.mp4)

Under Pressure ( Pushing Down on Me ( Pushing Down on ME))

[](/doc-files/keep1.mp4)

To the Lighthouse 

![](/doc-files/otherLighthouse.jpg)

\The Flood

[](/doc-files/sponza.png)


## Self Evaluation
=====================================================

We feel we deserve an A on this project

- Our specification was scoped well to the size of the project. Initially we felt tackling water simulation might be ambitious, but we were able to build on existing libraries and previous rendering work done for the class to achieve our spec. Our spec is detailed enough that a future group could roughly replicate our work.
- We learned a lot about developing using existing libraries, working with physics simulations, and choosing the right rendering techniques for the task at hand.
- We managed our workflow well, balancing time discussing high-level goals as a team with spending time working individually and in pairs on separate aspects of the project.
- Our code is well-structured. Breaking the necessary functionality into separate classes helped us to develop independently and then integrate our work.
- Our report is well-written, concise, and clearly explains the technical features of our work.


## Schedule
=====================================================

By 11/10:

- specification (all)
- quality images (all)

By 11/15:

- report draft (Melanie)
- MVP particle simulation in glass box (Melanie, Cole)
- MVP marching cubes (Kenny, Yitong) 

By 11/17:

- MVP code (all)
- nice enough journal to tell the whole story (all)
- MVP waves in simulation, rigid object in scene (Melanie, Cole)

By 12/1:

- MVP report (all)
- water rendering (Melanie)
- integration with Flex library (Kenny)
- foam particles (Kenny)
- scene setup (Cole)
- video rendering (Yitong)

By 12/7:

- everything due (all)

By 12/8:

- presentations (all)


## Change Log
=====================================================

1. Path-tracing needs to be able to render foam appropriately (Melanie, 11/11).
2. We will use the Flex library instead of the PhysX library (Kenny, 11/26).
3. Instead of path-tracing, we will use Whitted ray-tracing (Melanie, 11/28).
4. We will keep diffuse (foam) particles as spheres instead of rendering them with marching cubes (Kenny, 11/29).
5. We will render 4 different scenes as demonstration videos: faucet, drain, waves, rigid object (Melanie, 11/30).
6. No longer rendering drain, instead rendering goo, fountain, lighthouse (Kenny, 12/4).

## Acknowledgements
=====================================================
We used Nvidia's Flex system for our water simulation (https://developer.nvidia.com/flex)
 
We used Paul Bourke's Cube Marching code as a base for our mesh generation (http://paulbourke.net/geometry/polygonise/) 

We integrated Grohe Zedra's faucet model into our scene (http://archive3d.net/?a=download&id=0e627555)

Thanks to Prof. Morgan McGuire and Jamie Lesser for their guidance and ideas as always!
