#pragma once

#include <ivf/composite_node.h>

namespace ivf {

	class Arrow : public CompositeNode {
	private:

	public:

		Arrow();

		static std::shared_ptr<Arrow> create();

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<CompositeNode> ArrowPtr;

}


