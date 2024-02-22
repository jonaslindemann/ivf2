#include <ivf/transform.h>

using namespace std;
using namespace ivf;

Transform::Transform()
{
}

std::shared_ptr<Transform> ivf::Transform::create()
{
	return std::make_shared<Transform>();
}