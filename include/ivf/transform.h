#pragma once

#include <ivf/composite_node.h>

namespace ivf {

	class Transform : public CompositeNode {
	private:
	public:
		Transform();

		static std::shared_ptr<Transform> create();
	};

	typedef std::shared_ptr<Transform> TransformPtr;
}


