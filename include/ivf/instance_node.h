#pragma once

#include <ivf/transform_node.h>

namespace ivf {

	class InstanceNode : public TransformNode {
	private:
		std::shared_ptr<TransformNode> m_node;
	public:
		InstanceNode();

		static std::shared_ptr<InstanceNode> create();

		void setNode(std::shared_ptr<TransformNode> node);
		std::shared_ptr<TransformNode> node();

	protected:
		virtual void doDraw();
	};

	typedef std::shared_ptr<InstanceNode> InstanceNodePtr;
};


