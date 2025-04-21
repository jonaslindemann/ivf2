#pragma once

#include <ivfui/ui_window.h>

class LissajouWindow : public ivfui::UiWindow {
private:
    // 1.0, 1.0, 1.0, 0.7, 3.0, 0.4, 1.0, 2.0, 1.0
    float m_a{1.0};
    float m_b{1.0};
    float m_c{1.0};
    float m_d{0.7};
    float m_e{3.0};
    float m_f{0.4};
    float m_g{1.0};
    float m_h{2.0};
    float m_i{1.0};
    float m_speed{1.0};
    bool m_dirty{true};
    int m_size{300};

public:
    LissajouWindow();

    static std::shared_ptr<LissajouWindow> create();

    void get_params(float &a, float &b, float &c, float &d, float &e, float &f, float &g, float &h, float &i);

    float speed() const;
    size_t size() const;

    bool is_dirty();

    void doDraw() override;
};

typedef std::shared_ptr<LissajouWindow> LissajouWindowPtr;
