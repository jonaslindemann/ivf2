#pragma once

#include <ivf/transform_node.h>

#include <vector>

namespace ivf {

	class CompositeNode : public TransformNode {
	private:
		std::vector<std::shared_ptr<Node>> m_nodes;
	public:
		CompositeNode();

		static std::shared_ptr<CompositeNode> create();

		void add(std::shared_ptr<Node> node);
		std::vector<std::shared_ptr<Node>> nodes();
		void clear();

	protected:
		virtual void doDraw();
	};

	typedef std::shared_ptr<CompositeNode> CompositeNodePtr;
}


