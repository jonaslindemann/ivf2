#pragma once

// MSVC requires this define before <cmath>/<math.h> to expose M_PI and friends.
// Must precede the cmath include inside this PCH.
#define _USE_MATH_DEFINES

// Required by imgui_internal.h — must be defined before the first imgui.h include
// (which arrives transitively through ivfui/ui.h).
#define IMGUI_DEFINE_MATH_OPERATORS

// STL
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <numbers>

// ivf core — these two umbrella headers are included in virtually every example
#include <ivf/gl.h>
#include <ivf/nodes.h>

// ivfui — window, application, camera, UI widgets
#include <ivfui/ui.h>

// ImGui (pulled in transitively by ivfui/ui.h, but listed explicitly for clarity)
#include <imgui.h>
