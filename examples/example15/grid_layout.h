#pragma once

#include <ivf/nodes.h>

class GridLayout {
private:
    size_t m_rows;
    size_t m_cols;
    size_t m_layers;

    float m_xSpacing;
    float m_ySpacing;
    float m_zSpacing;

public:
    GridLayout(size_t rows, size_t cols, size_t layers, float xSpacing, float ySpacing, float zSpacing);
    ~GridLayout();
    void setSpacing(float xSpacing, float ySpacing, float zSpacing);
    void setSpacing(float spacing);
    void setSize(size_t rows, size_t cols, size_t layers);
    void setRows(size_t rows);
    void setCols(size_t cols);
    void setLayers(size_t layers);
    size_t rows();
    size_t cols();
    size_t layers();
    float xSpacing();
    float ySpacing();
    float zSpacing();
    size_t size();

    void apply(ivf::CompositeNode *composite);
    void apply(ivf::CompositeNodePtr &composite);
};
