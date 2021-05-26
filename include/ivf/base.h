#ifndef __ivf2_base__
#define __ivf2_base__

#include <memory>

namespace ivf {
    
    class Base {
    private:
        
    public:
        Base();
        virtual ~Base();        
    };

    typedef std::shared_ptr<Base> BasePtr;
};

#endif
