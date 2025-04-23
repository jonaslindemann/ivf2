#pragma once

#include <memory>

namespace ivf {

/**
 * Base class for all objects in the ivf library.
 * 
 * This class is used to define a common interface for all objects in the ivf library.
 * Currently it is an empty class, but it can be extended in the future to include common
 * functionality.
 */    
class Base {
private:
public:
    Base() = default;
    virtual ~Base() = default;
};
}; // namespace ivf
