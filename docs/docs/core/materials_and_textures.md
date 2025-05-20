# Materials and Textures

All drawable objects in ivf2 supports materials and textures. Materials are used to define the surface properties of an object, such as its color, shininess, and texture. Textures are images that are applied to the surface of an object to give it a more realistic appearance.

## Materials

Materials are implemented in the `Material` class. It is created just like any other object in ivf2. The `Material` class has several properties that can be set to define the appearance of the material. These properties include:

- `diffuse`: The diffuse color of the material. This is the base color of the material and is used to define how the material interacts with light.
- `specular`: The specular color of the material. This is the color of the highlights on the material and is used to define how shiny the material is.
- `shininess`: The shininess of the material. This is a value that defines how shiny the material is. A higher value means a shinier material.
- `ambient`: The ambient color of the material. This is the color of the material in shadow and is used to define how the material interacts with light in shadow.
- `alpha`: The alpha value of the material. This is a value between 0 and 1 that defines how transparent the material is. A value of 0 means the material is completely transparent, while a value of 1 means the material is completely opaque.

The following code shows an example of how to create a material in ivf2:

```cpp
auto material = Material::create();
material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));
```	

A material can be applied to a node by calling the `setMaterial` method on the node. The following code shows an example of how to apply a material to a node:

```cpp
node->setMaterial(material);
```

There are some other properties that can be set to get specific behavior. If the node is not using the lighting system, the `setUseLighting` method should be set to false. This will disable the lighting system for the node. The diffuse and alpha properties will be used as the color of the node. In this mode it is also possible to use the colors defined at the vertices, which is controlled using the `setUseVertexColors` method. 

## Textures

Textures are implemented in the `Texture` class. A texture is an image that is applied to the surface of a material to give it a more realistic appearance. Textures can be created from images in various formats, such as PNG, JPEG, and BMP.

Textures can be applied to a material by calling the `setTexture` method on the material. The following code shows an example of how to create a texture and apply it to a material:

```cpp
auto textureCat = Texture::create();
textureCat->load("assets/pop_cat.png");

auto textureBrick = Texture::create();
textureBrick->load("assets/brick.png");

auto sphere = Sphere::create();
sphere->setTexture(textureCat);
sphere->setMaterial(sphereMaterial);
sphere->setPos(glm::vec3(0.0, 3.0, 0.0));

auto box = Box::create();
box->setTexture(textureBrick);
box->setMaterial(sphereMaterial);
box->setPos(glm::vec3(3.0, 0.0, 0.0));
```

There are some propoerties the control how the texture is applied to the node. The `setWrapT()` and `setWrapS()` methods control how the texture is wrapped around the node. The `setMinFilter()` and `setMagFilter()` methods control how the texture is filtered when it is scaled. 

How textures are applied globally is controlled by the `TextureManager` class. This class is a singleton and controls how the textures are applied.

```cpp
auto texMgr = TextureManager::getInstance();
texMgr->setTextureBlendMode(TextureBlendMode::Normal);
texMgr->setUseTextures(true);
texMgr->setBlendFactor(0.5);
```

The `Texture` class can also control blending if `setUseLocalBlending` is set to true. This will use the local blending mode for the texture. The `setBlendFactor` method of the class controls how much of the texture is blended with the material. A value of 0 means no blending, while a value of 1 means full blending.

```cpp
auto tex = Texture::create();
tex->load("assets/pop_cat.png");
tex->setUseLocalBlending(true);
tex->setBlendFactor(0.5);
```


