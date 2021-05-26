#pragma once

#include <ivf/base.h>
#include <ivf/meshnode.h>

#include <memory>

namespace ivf {

	class NormalFactory : public Base {
	private:

	public:
		NormalFactory();

		static std::shared_ptr<NormalFactory> create();

		std::shared_ptr<MeshNode> create(std::shared_ptr<MeshNode> meshNode);
	};

	typedef std::shared_ptr<NormalFactory> NormalFactoryPtr;

}


