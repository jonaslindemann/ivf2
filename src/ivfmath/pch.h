#pragma once

#define _USE_MATH_DEFINES

// STL
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cmath>

// GLM (stable headers only — gtx experimental headers are excluded so
// per-file GLM_ENABLE_EXPERIMENTAL defines continue to work correctly)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>
