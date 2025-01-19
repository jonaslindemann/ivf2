#include <ivfmath/keyframe_data.h>

using namespace ivfmath;

KeyframeData::KeyframeData(float time, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, float tension)
    : timestamp(time), position(pos), rotation(rot), scale(scale), tension(0.5f)
{}
