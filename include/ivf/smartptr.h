//
// Copyright 1999-2006 by Structural Mechanics, Lund University.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "ivf@byggmek.lth.se".
//
//
// Written by Jonas Lindemann
//

#ifndef __ivf_smartptr_h__
#define __ivf_smartptr_h__

#define IvfStdPointer(classname) \
    class classname; \
    typedef classname* classname##StdPtr;

#define IvfSmartPointer(classname) \
    class classname; \
    typedef Pointer<classname> classname##Ptr; \
    typedef classname* classname##StdPtr;

namespace ivf {
    
    /**
     * Smart pointer class
     *
     * CIvfPointer handles the Ivf++ reference counting scheme of
     * the CIvfBase class. To declare a Ivf++ smart pointer use the
     * IvfSmartPointer() macro. See the following example:
     *
     * \code
     * int main()
     * {
     *    CIvfPointer<CIvfMaterial> material = new CIvfMaterial(); // addReference() called.
     *    CIvfPointer<CIvfMaterial> material2;
     *    material2 = material; // addReference() called
     *    .
     *    .
     *
     *    return 0;
     * }
     * // material calls deleteReference()
     * // material2 calls deleteRefernce() and deletes CIvfMaterial object
     * \endcode
     */
    template <class T,class R> class RefPointerBase {
    private:
        T* m_object;
    public:
        RefPointerBase(T* object = 0)
        {
            m_object = object;
            if (m_object)
                m_object->R::addRef();
        }
        
        RefPointerBase(const RefPointerBase& ivfObject)
        {
            m_object = ivfObject.m_object;
            if (m_object)
                m_object->R::addRef();
        }
        
        virtual ~RefPointerBase()
        {
            if (m_object)
            {
                m_object->R::decRef();
                if (!m_object->R::isReferenced())
                    delete m_object;
            }
        }
        
        operator T* () const { return m_object; }
        T& operator* () const { return *m_object; }
        T* operator-> () const { return m_object; }
        
        RefPointerBase& operator= (const RefPointerBase& IvfPointerRefBase)
        {
            if (m_object!=IvfPointerRefBase.m_object)
            {
                if (m_object)
                {
                    m_object->R::decRef();
                    if (!m_object->R::isReferenced())
                        delete m_object;
                }
                
                m_object = IvfPointerRefBase.m_object;
                
                if (m_object)
                    m_object->R::addRef();
                    }
            return *this;
        }
        
        RefPointerBase& operator= (T* ivfObject)
        {
            if (m_object!=ivfObject)
            {
                if (m_object)
                {
                    m_object->R::decRef();
                    if (!m_object->R::isReferenced())
                        delete m_object;
                }
                
                m_object = ivfObject;
                
                if (m_object)
                    m_object->R::addRef();
                    }
            return *this;
        }
        
        bool operator== (T* ivfObject) const { return m_object == ivfObject; }
        bool operator!= (T* ivfObject) const { return m_object != ivfObject; }
        bool operator== (const RefPointerBase& IvfPointerRefBase) const
        {
            return m_object == IvfPointerRefBase.m_object;
        }
        
        bool operator!= (const RefPointerBase& IvfPointerRefBase) const
        {
            return m_object != IvfPointerRefBase.m_object;
        }
    };
    
    template <class T> class Pointer {
    private:
        T* m_object;
    public:
        Pointer(T* object = 0)
        {
            m_object = object;
            if (m_object)
                m_object->addRef();
        }
        
        Pointer(const Pointer& ivfObject)
        {
            m_object = ivfObject.m_object;
            if (m_object)
                m_object->addRef();
        }
        
        virtual ~Pointer()
        {
            if (m_object)
            {
                m_object->decRef();
                if (!m_object->isReferenced())
                    delete m_object;
            }
        }
        
        operator T* () const { return m_object; }
        T& operator* () const { return *m_object; }
        T* operator-> () const { return m_object; }
        
        Pointer& operator= (const Pointer& ivfPointer)
        {
            if (m_object!=ivfPointer.m_object)
            {
                if (m_object)
                {
                    m_object->decRef();
                    if (!m_object->isReferenced())
                        delete m_object;
                }
                
                m_object = ivfPointer.m_object;
                
                if (m_object)
                    m_object->addRef();
                    }
            return *this;
        }
        
        Pointer& operator= (T* ivfObject)
        {
            if (m_object!=ivfObject)
            {
                if (m_object)
                {
                    m_object->decRef();
                    if (!m_object->isReferenced())
                        delete m_object;
                }
                
                m_object = ivfObject;
                
                if (m_object)
                    m_object->addRef();
                    }
            return *this;
        }
        
        bool operator== (T* ivfObject) const { return m_object == ivfObject; }
        bool operator!= (T* ivfObject) const { return m_object != ivfObject; }
        bool operator== (const Pointer& ivfPointer) const
        {
            return m_object == ivfPointer.m_object;
        }
        
        bool operator!= (const Pointer& ivfPointer) const
        {
            return m_object != ivfPointer.m_object;
        }
    };
    
};

#endif
