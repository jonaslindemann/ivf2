#pragma once

/**
 * @file procedural.h
 * @brief Convenience header that includes all procedural texture functionality.
 * 
 * This header file includes both the procedural utilities and all procedural texture classes,
 * providing a single include for all procedural texture functionality in IVF2.
 * 
 * @example
 * ```cpp
 * #include <ivf/procedural.h>
 * 
 * auto noise = ivf::PerlinNoiseTexture::create();
 * noise->setSize(512, 512);
 * noise->setScale(4.0f);
 * noise->regenerate();
 * ```
 */

#include <ivf/proc_utils.h>
#include <ivf/procedural_texture.h>
