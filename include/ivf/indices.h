#ifndef __arc2_indices__
#define __arc2_indices__

#include <ivf/intfield.h>

namespace ivf {
    
	class Indices : public IntField {
    public:
		Indices(GLuint nIndices, GLuint cols);

		static std::shared_ptr<Indices> create(GLuint nIndices, GLuint cols);

		void setIndex(GLuint idx, GLuint i0);
		void setIndex(GLuint idx, GLuint i0, GLuint i1);
		void setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2);
		void setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2, GLuint i3);

		void index(GLuint idx, GLuint& i0);
		void index(GLuint idx, GLuint& i0, GLuint& i1);
		void index(GLuint idx, GLuint& i0, GLuint& i1, GLuint& i2);
		void index(GLuint idx, GLuint& i0, GLuint& i1, GLuint& i2, GLuint& i3);
	};
	typedef std::shared_ptr<Indices> IndicesPtr;
};


#endif