#pragma once

/**
 * @file gl.h
 * @brief All core ivf headers in one include: OpenGL wrappers, managers, nodes and materials.
 *
 * For questions on the library, open an issue on the
 * [GitHub repository](https://github.com/jonaslindemann/ivf2).
 */

#include <ivf/float_field.h>
#include <ivf/vertex_buffer.h>
#include <ivf/vertex_array.h>
#include <ivf/vertices.h>
#include <ivf/colors.h>
#include <ivf/vertex_shader.h>
#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/render_context.h>
#include <ivf/gl_debug.h>
#include <ivf/result.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/texture_manager.h>
#include <ivf/utils.h>
#include <ivf/light_manager.h>
#include <ivf/normal_factory.h>
#include <ivf/texture.h>
#include <ivf/texture3d.h>
#include <ivf/material.h>
#include <ivf/font_manager.h>
#include <ivf/selection_manager.h>
#include <ivf/buffer_selection.h>
#include <ivf/camera.h>
#include <ivf/mesh_manager.h>
#include <ivf/math_utils.h>
#include <ivf/palette.h>
#include <ivf/tween.h>
#include <ivf/time_controller.h>
#include <ivf/scene_timeline.h>
#include <ivf/flow_field.h>
#include <ivf/timer_manager.h>
#include <ivf/debug_draw.h>
#include <ivf/shader_watcher.h>
#include <ivf/cubemap.h>
#include <ivf/skybox.h>
#include <ivf/billboard.h>
#include <ivf/bump_material.h>
#include <ivf/shader_material.h>
#include <ivf/blend_shader_material.h>
#include <ivf/glow_material.h>
#include <ivf/mist_material.h>
#include <ivf/gradient_sky_material.h>
