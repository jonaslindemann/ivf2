#include <ivf/normal_factory.h>

using namespace ivf;

NormalFactory::NormalFactory()
{

}

std::shared_ptr<NormalFactory> ivf::NormalFactory::create()
{
	return std::make_shared<NormalFactory>();
}

std::shared_ptr<MeshNode> ivf::NormalFactory::create(std::shared_ptr<MeshNode> meshNode)
{
	std::shared_ptr<Normals> normals = meshNode->mesh()->normals();
	std::shared_ptr<Vertices> vertices = meshNode->mesh()->vertices();

	std::shared_ptr<MeshNode> normalNode = std::make_shared<MeshNode>();

	normalNode->newMesh(normals->rows() * 2, 0);

	normalNode->mesh()->begin(GL_LINES);

	for (GLuint i = 0; i < normals->rows(); i++)
	{
		glm::vec3 v = vertices->vertex(i);
		glm::vec3 n = normals->normal(i);

		glm::vec3 p0 = v;
		glm::vec3 p1 = v + n;
		normalNode->mesh()->vertex3f(p0);
		normalNode->mesh()->color3f(1.0f, 1.0f, 1.0f);
		normalNode->mesh()->vertex3f(p1);
		normalNode->mesh()->color3f(1.0f, 1.0f, 1.0f);
	}

	normalNode->mesh()->end();

 	return normalNode;
}
