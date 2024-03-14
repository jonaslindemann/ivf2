#pragma once

#include <memory>

namespace ivf {
    
    class Base {
    private:
        
    public:
        Base() = default;
        virtual ~Base() = default;       
    };

    typedef std::shared_ptr<Base> BasePtr;
};


