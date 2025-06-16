#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

    /**
     * @class Box
     * @brief Node representing a 3D box (cuboid) mesh with configurable size and segmentation.
     *
     * The Box class provides a 3D box geometry that can be used in a scene. The box size and the number
     * of segments along each axis can be customized for more detailed geometry. Inherits from MeshNode,
     * allowing transformation and integration into a scene graph.
     */
	class Box : public MeshNode {
    private:
        glm::vec3 m_size{1.0f, 1.0f, 1.0f};      ///< Size of the box along each axis (width, height, depth).
        glm::uvec3 m_segments{8, 8, 8};          ///< Number of segments along each axis.

	public:
        /**
         * @brief Constructor.
         * @param size Size of the box (width, height, depth). Default is (1, 1, 1).
         * @param segments Number of segments along each axis. Default is (8, 8, 8).
         */
		Box(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

        /**
         * @brief Constructor with explicit dimensions and segment counts.
         * @param w Width of the box.
         * @param h Height of the box.
         * @param d Depth of the box.
         * @param i Number of segments along the X axis.
         * @param j Number of segments along the Y axis.
         * @param k Number of segments along the Z axis.
         */
        Box(double w, double h, double d, int i, int j, int k);

        /**
         * @brief Destructor.
         */
		virtual ~Box();

        /**
         * @brief Copy constructor.
         * @param other Box to copy from.
         */
        Box(const Box& other);

        /**
         * @brief Copy assignment operator.
         * @param other Box to copy from.
         * @return Reference to this box.
         */
		Box& operator=(const Box& other);

        /**
         * @brief Move constructor.
         * @param other Box to move from.
         */
		Box(Box&& other);

        /**
         * @brief Move assignment operator.
         * @param other Box to move from.
         * @return Reference to this box.
         */
		Box& operator=(Box&& other);

        /**
         * @brief Factory method to create a shared pointer to a Box instance.
         * @param size Size of the box (width, height, depth). Default is (1, 1, 1).
         * @param segments Number of segments along each axis. Default is (8, 8, 8).
         * @return std::shared_ptr<Box> New Box instance.
         */
		static std::shared_ptr<Box> create(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

        /**
         * @brief Set the size and segments of the box.
         * @param size New size of the box (width, height, depth).
         * @param segments New number of segments along each axis.
         */
		void set(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

        /**
         * @brief Set the size of the box using explicit dimensions.
         * @param w Width of the box.
         * @param h Height of the box.
         * @param d Depth of the box.
         */
		void setSize(double w, double h, double d);

        /**
         * @brief Set the size of the box.
         * @param size New size (width, height, depth).
         */
		void setSize(glm::vec3 size);

        /**
         * @brief Get the current size of the box.
         * @return glm::vec3 The current size (width, height, depth).
         */
		glm::vec3 size();

        /**
         * @brief Set the number of segments along each axis.
         * @param i Segments along the X axis.
         * @param j Segments along the Y axis.
         * @param k Segments along the Z axis.
         */
		void setSegments(int i, int j, int k);

        /**
         * @brief Set the number of segments along each axis.
         * @param segments Number of segments as a glm::uvec3.
         */
		void setSegments(glm::uvec3 segments);

        /**
         * @brief Get the current number of segments along each axis.
         * @return glm::uvec3 The current segment counts.
         */
		glm::uvec3 segments();

	protected:
        /**
         * @brief Internal setup method for initializing the box geometry.
         */
		virtual void doSetup();
	};

    /**
     * @typedef BoxPtr
     * @brief Shared pointer type for Box.
     */
	typedef std::shared_ptr<Box> BoxPtr;

}
