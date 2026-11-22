#pragma once

namespace solar {

class Base {
private:
    
public:
    virtual ~Base() = default;
    Base() = default;
    Base(const Base &) = default;
    Base &operator=(const Base &) = default;
    Base(Base &&) = default;
    Base &operator=(Base &&) = default;

    virtual void print();
};

} // namespace solar
