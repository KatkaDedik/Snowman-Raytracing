# Homework for pv227 GPU Rendering

(OpenGL)

Initial State
  • The scene contains a snowman formed by 10 spheres, 3-point lights rotating around the
    snowman, and the main camera. The scene is rendered using the standard OpenGL rasterization
    pipeline with forward lit and unlit shaders.
  • The lights are stored in the ‘phong_lights_ubo’ buffer.
  • The spheres forming the snowman are stored in the ‘snowman’ structure, which is in turn stored
    in the ‘snowmen_ubo’ buffer – see the definition of the SnowmanUBO in the header file. 

===================================================================================================
                                              ASSIGNMENT
===================================================================================================
Snow Particles
  • Your application must contain an effect of falling snow (e.g., using a particle system). No
  physical behavior is required; simple linear movement along Y-axis is enough. The application
  must enable real-time rendering of various numbers of snow particles, which is set in the UI. The
  application should also enable to turn off the snow rendering completely if the corresponding
  checkbox in UI is unchecked.
  • The snow must correctly handle the depth, meaning that if the snowflakes fall behind the
  snowman, they should not be visible.
  
Ambient Occlusion
  The spheres forming the snowman should have an ambient occlusion on them. The areas where the
  spheres are touching each other, or the ground, should be darker. The application must allow turning
  the ambient occlusion on and off based on the corresponding checking in UI.
  
Reflections
  • The scene must contain reflections – i.e., both the snowman and the lights should be reflected
  on the floor as well as on the snowman itself. The amount/quality of the reflections should
  depend on the Reflections Quality slider from UI.

Shadows
  • The snowman must cast soft shadows on the floor (note that the floor is not in the initial project
  and must be added by you).
  • The softness of the shadows will correspond to the size of the lights, which can be adjusted from
  the UI. The smaller the light, the harder edges on the shadow (i.e., 0 corresponds to the hard
  shadows and 1 corresponds to the very soft shadows). The size of the light should also be
  reflected by its sphere representation within the scene. The light spheres are not in the
  snowman buffer and must be added by you (I suggest doing this directly in the shader).
  • The quality of the soft shadows should also be adjustable depending on the value of the Shadow
  Quality slider from UI to enable trading speed for quality (unless you use advanced analytical
  approaches which estimate the penumbra correctly). 
