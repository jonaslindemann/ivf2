#include <ivf/grid_layout.h>

using namespace ivf;

GridLayout::GridLayout(size_t rows, size_t cols, size_t layers, float xSpacing, float ySpacing, float zSpacing)
    : m_rows(rows), m_cols(cols), m_layers(layers), m_xSpacing(xSpacing), m_ySpacing(ySpacing), m_zSpacing(zSpacing)
{}

GridLayout::~GridLayout()
{}

void GridLayout::setSpacing(float xSpacing, float ySpacing, float zSpacing)
{
    m_xSpacing = xSpacing;
    m_ySpacing = ySpacing;
    m_zSpacing = zSpacing;
}

void GridLayout::setSpacing(float spacing)
{
    setSpacing(spacing, spacing, spacing);
}

void GridLayout::setSize(size_t rows, size_t cols, size_t layers)
{
    m_rows = rows;
    m_cols = cols;
    m_layers = layers;
}

void GridLayout::setRows(size_t rows)
{
    m_rows = rows;
}

void GridLayout::setCols(size_t cols)
{
    m_cols = cols;
}

void GridLayout::setLayers(size_t layers)
{
    m_layers = layers;
}

size_t GridLayout::rows()
{
    return m_rows;
}

size_t GridLayout::cols()
{
    return m_cols;
}

size_t GridLayout::layers()
{
    return m_layers;
}

float GridLayout::xSpacing()
{
    return m_xSpacing;
}

float GridLayout::ySpacing()
{
    return m_ySpacing;
}

float GridLayout::zSpacing()
{
    return m_zSpacing;
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
                if (index >= composite->count())
                    return;

                auto node = composite->at(index);
                auto transformNode = dynamic_cast<ivf::TransformNode *>(node.get());

                if (transformNode)
                    transformNode->setPos(glm::vec3(static_cast<float>(i) * m_xSpacing - xOffset,
                                                    static_cast<float>(k) * m_ySpacing - yOffset,
                                                    static_cast<float>(j) * m_zSpacing - zOffset));

                index++;
            }
        }
    }
}

void GridLayout::apply(ivf::CompositeNodePtr &composite)
{
    this->apply(composite.get());
}
