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
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <cassert>
#include <optional>

// OpenGL loader — must come before any GL headers
#include <glad/glad.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLFW (after glad)
#include <GLFW/glfw3.h>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
