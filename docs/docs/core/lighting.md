# Lighting

Ivf2 supports basic lighting in 3D scenes, allowing for more realistic rendering of objects. The model is implemented in the basic shader built into the engine, which uses the Phong reflection model. This model simulates how light interacts with surfaces, providing a way to create highlights and shadows.

The light system is controlled by the `LightManager` singleton class. It manages the lights in the scene and provides methods to add, remove, and update lights. There are three types of lights supported:

- **Directional Light**: This type of light simulates sunlight or other distant light sources. It has a direction but no position, meaning it illuminates all objects in the scene from a specific angle.
- **Point Light**: This type of light emits light in all directions from a specific point in space. It is useful for simulating light bulbs or other localized light sources.
- **Spot Light**: This type of light emits a cone of light from a specific point in space, similar to a flashlight. It has a direction and an angle that defines the size of the cone.

The `Material` also uses the `LightManager` to determine how to render objects with lighting. Each material can specify whether it uses lighting and how it interacts with the lights in the scene and calls the appropriate methods to calculate the final color of the object based on the lights affecting it. 

The basic shader model in Ivf2 supports 8 light of each type. The lights are added to the `LightManager` and can be updated dynamically. The following code shows an example of how to create and add a directional light:

```cpp
auto lightMgr = LightManager::getInstance();
auto pointLight = lightMgr->addPointLight();
```

By default adding a light will automatically enable the light. Be default the light is white and has a position of (0, 0, 0). In the following code we set the position and color properties of the light:

```cpp
pointLight->setEnabled(false);
pointLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
pointLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
pointLight->setAttenuation(1.0, 0.14, 0.07);
pointLight->setPosition(glm::vec3(2.0, 2.0, 2.0));
```

The number of lights created for each can be queried using the `pointLightCount()`, `directionalLightsCount()`, and `spotLightCount()` methods. The different light types can be cleared using the `clearPointLights()`, `clearDirectionalLights()`, and `clearSpotLights()` methods. All lights can be cleared using the `clearLights()` method.

A directional light can be created in a similar way except that instead of setting the position we set the direction of the light. The following code shows how to create a directional light:

```cpp
auto directionalLight = lightMgr->addDirectionalLight();
directionalLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
```

For the spot light, we can set the position, direction, and angle of the light. The following code shows how to create a spot light:

```cpp
auto spotLight = lightMgr->addSpotLight();
spotLight->setDirection(glm::vec3(0.0, 0.0, -1.0));
spotLight->setPosition(glm::vec3(0.0, 0.0, 10.0));
spotLight->setCutoff(12.0f, 13.0f);
spotLight->setEnabled(true);
```

The `setCutoff` method sets the inner and outer angles of the spotlight. The inner angle is where the light is fully bright, and the outer angle is where the light starts to fade out.

The light manager also provides methods to enable or disable lighting globally. The `setUseLighting` method can be used to enable or disable the lighting system for the entire scene. When lighting is disabled, objects will not be affected by any lights, and their appearance will be based solely on their materials and textures. This is also used internally by the `Material` class to disable lighting for nodes that do not use the lighting system. 