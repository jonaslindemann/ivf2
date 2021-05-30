#ifndef __ivf2_mesh__
#define __ivf2_mesh__

#include <ivf/glbase.h>
#include <ivf/vertices.h>
#include <ivf/colors.h>
#include <ivf/normals.h>
#include <ivf/indices.h>
#include <ivf/texcoords.h>

#include <ivf/vertexarray.h>
#include <ivf/vertexbuffer.h>
#include <ivf/indexbuffer.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>

#include <memory>

namespace ivf {
      
    class Mesh : public GLBase {
    private:
        GLuint m_primType;

        std::shared_ptr<Vertices> m_verts;
        std::shared_ptr<Colors> m_colors;
        std::shared_ptr<Indices> m_indices;
        std::shared_ptr<Normals> m_normals;
        std::shared_ptr<TexCoords> m_texCoords;

        std::shared_ptr<Vertices> m_glVerts;
        std::shared_ptr<Colors> m_glColors;
        std::shared_ptr<Normals> m_glNormals;

        int m_vertPos;
        int m_colorPos;
        int m_normalPos;
		int m_indexPos;
        int m_texCoordPos;

		int m_indexSize;
        
        GLint m_vertexAttrId;
        GLint m_colorAttrId;
        GLint m_normalAttrId;
        GLint m_texCoordAttrId;

        std::unique_ptr<VertexArray> m_VAO;
        std::unique_ptr<VertexBuffer> m_vertexVBO;
        std::unique_ptr<VertexBuffer> m_colorVBO;
        std::unique_ptr<VertexBuffer> m_normalVBO;
        std::unique_ptr<IndexBuffer> m_indexVBO;
        std::unique_ptr<VertexBuffer> m_texCoordVBO;

        glm::vec3 m_position;

        bool m_generateNormals;

		void setupPrim();

    public:
        Mesh(GLuint vsize, GLuint isize=0);
        virtual ~Mesh();

        static std::shared_ptr<Mesh> create(GLuint vsize, GLuint isize = 0);
        
		void setSize(GLuint vsize, GLuint isize = 0);

        void setGenerateNormals(bool flag);
        bool generateNormals();
        
        void setVertexAttrId(GLuint id);
        void setColorAttrId(GLuint id);
        void setNormalAttrId(GLuint id);
        void setTexCoordAttrId(GLuint id);
        
        void begin(GLuint primType);
        void vertex3f(GLfloat x, GLfloat y, GLfloat z);
        void vertex3d(GLdouble x, GLdouble y, GLdouble z);
        void vertex2f(GLfloat x, GLfloat y);
        void vertex2d(GLdouble x, GLdouble y);
		void vertex3d(const glm::dvec3& v);
        void vertex3f(const glm::vec3 v);
        void tex2f(GLfloat s, GLfloat t);
        void normal3f(GLfloat vx, GLfloat vy, GLfloat vz);
        void normal3d(GLdouble vx, GLdouble vy, GLdouble vz);
        void color3f(GLfloat r, GLfloat g, GLfloat b);
        void color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
		void index1i(GLuint i0);
		void index2i(GLuint i0, GLuint i1);
		void index3i(GLuint i0, GLuint i1, GLuint i2);
		void index4i(GLuint i0, GLuint i1, GLuint i2, GLuint i3);

		void triQuad(GLdouble w, GLdouble h, GLdouble offset, GLdouble vx, GLdouble vy, GLdouble vz);

        void end();
        
        void draw();
        void drawAsPrim(GLuint prim);

        void setPos(glm::vec3 pos);
        glm::vec3 pos();

        std::shared_ptr<Normals> normals();
        std::shared_ptr<Vertices> vertices();
    };

    typedef std::shared_ptr<Mesh> MeshPtr;

};

#endif
