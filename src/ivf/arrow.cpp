#include <ivf/arrow.h>

using namespace ivf;

Arrow::Arrow()
{
}

std::shared_ptr<Arrow> Arrow::create()
{
    return std::make_shared<Arrow>();
}

