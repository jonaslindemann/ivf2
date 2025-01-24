#include "grid_layout.h"

GridLayout::GridLayout(size_t rows = 10, size_t layers = 10, size_t cols = 10, float xSpacing = 1.0,
                       float ySpacing = 1.0, float zSpacing = 1.0)
    : m_rows(rows), m_cols(cols), m_layers(layers), m_xSpacing(xSpacing), m_ySpacing(ySpacing), m_zSpacing(zSpacing)
{}

GridLayout::~GridLayout()
{}

void GridLayout::setSpacing(float xSpacing, float ySpacing, float zSpacing)
{}

void GridLayout::setSpacing(float spacing)
{}

void GridLayout::setSize(size_t rows, size_t cols, size_t layers)
{}

void GridLayout::setRows(size_t rows)
{}

void GridLayout::setCols(size_t cols)
{}

void GridLayout::setLayers(size_t layers)
{}

size_t GridLayout::rows()
{
    return size_t();
}

size_t GridLayout::cols()
{
    return size_t();
}

size_t GridLayout::layers()
{
    return size_t();
}

float GridLayout::xSpacing()
{
    return 0.0f;
}

float GridLayout::ySpacing()
{
    return 0.0f;
}

float GridLayout::zSpacing()
{
    return 0.0f;
}

size_t GridLayout::size()
{
    return m_cols * m_rows * m_layers;
}

void GridLayout::apply(ivf::CompositeNode *composite)
{
    size_t index = 0;

    float xOffset = m_xSpacing * (m_cols - 1) / 2.0f;
    float yOffset = m_ySpacing * (m_layers - 1) / 2.0f;
    float zOffset = m_zSpacing * (m_rows - 1) / 2.0f;

    for (size_t i = 0; i < m_cols; i++)
    {
        for (size_t j = 0; j < m_rows; j++)
        {
            for (size_t k = 0; k < m_layers; k++)
            {
                auto node = composite->at(index);
                auto transformNode = dynamic_cast<ivf::TransformNode *>(node.get());

                if (transformNode)
                    transformNode->setPos(
                        glm::vec3(i * m_xSpacing - xOffset, k * m_ySpacing - yOffset, j * m_zSpacing - zOffset));

                index++;
            }
        }
    }
}

void GridLayout::apply(ivf::CompositeNodePtr &composite)
{
    this->apply(composite.get());
}
