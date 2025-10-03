#include <ivfui/ui_input_dialog.h>
#include <imgui.h>
#include <cstring>
#include <algorithm>

using namespace ivfui;

UiInputDialog::UiInputDialog(const std::string& title, const std::string& label, InputType type)
    : m_title(title)
    , m_label(label)
    , m_type(type)
    , m_isOpen(false)
    , m_shouldOpen(false)
    , m_floatValue(0.0f)
    , m_intValue(0)
    , m_stringValue("")
    , m_doubleValue(0.0)
    , m_minFloat(-FLT_MAX)
    , m_maxFloat(FLT_MAX)
    , m_minInt(INT_MIN)
    , m_maxInt(INT_MAX)
    , m_minDouble(-DBL_MAX)
    , m_maxDouble(DBL_MAX)
    , m_floatStep(0.01f)
    , m_floatStepFast(0.1f)
    , m_intStep(1)
    , m_doubleStep(0.01)
    , m_doubleStepFast(0.1)
    , m_maxStringLength(DEFAULT_STRING_BUFFER_SIZE - 1)
    , m_hasConstraints(false)
    , m_stringBuffer(nullptr)
{
    initStringBuffer();
}

UiInputDialog::~UiInputDialog()
{
    cleanupStringBuffer();
}

std::shared_ptr<UiInputDialog> UiInputDialog::create(const std::string& title, 
                                                     const std::string& label, 
                                                     InputType type)
{
    return std::make_shared<UiInputDialog>(title, label, type);
}

void UiInputDialog::openFloat(float initialValue, DialogCallback callback, 
                              float minValue, float maxValue, 
                              float step, float stepFast)
{
    m_type = InputType::Float;
    m_floatValue = initialValue;
    m_minFloat = minValue;
    m_maxFloat = maxValue;
    m_floatStep = step;
    m_floatStepFast = stepFast;
    m_hasConstraints = (minValue > -FLT_MAX || maxValue < FLT_MAX);
    m_callback = callback;
    m_shouldOpen = true;
    m_isOpen = false;
}

void UiInputDialog::openInt(int initialValue, DialogCallback callback,
                            int minValue, int maxValue, int step)
{
    m_type = InputType::Int;
    m_intValue = initialValue;
    m_minInt = minValue;
    m_maxInt = maxValue;
    m_intStep = step;
    m_hasConstraints = (minValue > INT_MIN || maxValue < INT_MAX);
    m_callback = callback;
    m_shouldOpen = true;
    m_isOpen = false;
}

void UiInputDialog::openDouble(double initialValue, DialogCallback callback,
                               double minValue, double maxValue,
                               double step, double stepFast)
{
    m_type = InputType::Double;
    m_doubleValue = initialValue;
    m_minDouble = minValue;
    m_maxDouble = maxValue;
    m_doubleStep = step;
    m_doubleStepFast = stepFast;
    m_hasConstraints = (minValue > -DBL_MAX || maxValue < DBL_MAX);
    m_callback = callback;
    m_shouldOpen = true;
    m_isOpen = false;
}

void UiInputDialog::openString(const std::string& initialValue, DialogCallback callback,
                               size_t maxLength)
{
    m_type = InputType::String;
    m_stringValue = initialValue;
    m_maxStringLength = std::min(maxLength, DEFAULT_STRING_BUFFER_SIZE - 1);
    
    // Copy initial value to buffer
    std::strncpy(m_stringBuffer, initialValue.c_str(), m_maxStringLength);
    m_stringBuffer[m_maxStringLength] = '\0';
    
    m_hasConstraints = false;
    m_callback = callback;
    m_shouldOpen = true;
    m_isOpen = false;
}

bool UiInputDialog::draw()
{
    // Open the popup if requested
    if (m_shouldOpen)
    {
        ImGui::OpenPopup(m_title.c_str());
        m_shouldOpen = false;
        m_isOpen = true;
    }

    bool isOpen = true;
    if (ImGui::BeginPopupModal(m_title.c_str(), &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", m_label.c_str());
        ImGui::Separator();
        
        bool inputChanged = false;
        
        switch (m_type)
        {
            case InputType::Float:
                ImGui::SetNextItemWidth(200.0f);
                if (m_hasConstraints)
                {
                    inputChanged = ImGui::SliderFloat("##input", &m_floatValue, m_minFloat, m_maxFloat, "%.3f");
                }
                else
                {
                    inputChanged = ImGui::InputFloat("##input", &m_floatValue, m_floatStep, m_floatStepFast, "%.3f");
                }
                break;
                
            case InputType::Int:
                ImGui::SetNextItemWidth(200.0f);
                if (m_hasConstraints)
                {
                    inputChanged = ImGui::SliderInt("##input", &m_intValue, m_minInt, m_maxInt);
                }
                else
                {
                    inputChanged = ImGui::InputInt("##input", &m_intValue, m_intStep);
                }
                break;
                
            case InputType::Double:
                ImGui::SetNextItemWidth(200.0f);
                if (m_hasConstraints)
                {
                    inputChanged = ImGui::SliderScalar("##input", ImGuiDataType_Double, &m_doubleValue, 
                                                     &m_minDouble, &m_maxDouble, "%.6f");
                }
                else
                {
                    inputChanged = ImGui::InputScalar("##input", ImGuiDataType_Double, &m_doubleValue, 
                                                    &m_doubleStep, &m_doubleStepFast, "%.6f");
                }
                break;
                
            case InputType::String:
                ImGui::SetNextItemWidth(200.0f);
                inputChanged = ImGui::InputText("##input", m_stringBuffer, m_maxStringLength + 1);
                if (inputChanged)
                {
                    m_stringValue = std::string(m_stringBuffer);
                }
                break;
        }
        
        ImGui::Separator();
        
        // Handle Enter key for acceptance
        bool enterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused();
        bool escapePressed = ImGui::IsKeyPressed(ImGuiKey_Escape);
        
        if (ImGui::Button("OK", ImVec2(80, 0)) || enterPressed)
        {
            handleAccept();
            ImGui::CloseCurrentPopup();
            m_isOpen = false;
        }
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(80, 0)) || escapePressed)
        {
            handleCancel();
            ImGui::CloseCurrentPopup();
            m_isOpen = false;
        }
        
        ImGui::EndPopup();
    }
    else if (m_isOpen)
    {
        // Popup was closed externally
        m_isOpen = false;
        handleCancel();
    }
    
    return m_isOpen;
}

bool UiInputDialog::isOpen() const
{
    return m_isOpen;
}

void UiInputDialog::close()
{
    if (m_isOpen)
    {
        handleCancel();
        m_isOpen = false;
    }
}

void UiInputDialog::setTitle(const std::string& title)
{
    m_title = title;
}

void UiInputDialog::setLabel(const std::string& label)
{
    m_label = label;
}

void UiInputDialog::initStringBuffer()
{
    m_stringBuffer = new char[DEFAULT_STRING_BUFFER_SIZE];
    std::memset(m_stringBuffer, 0, DEFAULT_STRING_BUFFER_SIZE);
}

void UiInputDialog::cleanupStringBuffer()
{
    delete[] m_stringBuffer;
    m_stringBuffer = nullptr;
}

void UiInputDialog::handleAccept()
{
    if (m_callback)
    {
        InputValue value;
        
        switch (m_type)
        {
            case InputType::Float:
                // Apply constraints if enabled
                if (m_hasConstraints)
                {
                    m_floatValue = std::max(m_minFloat, std::min(m_maxFloat, m_floatValue));
                }
                value = &m_floatValue;
                break;
                
            case InputType::Int:
                // Apply constraints if enabled
                if (m_hasConstraints)
                {
                    m_intValue = std::max(m_minInt, std::min(m_maxInt, m_intValue));
                }
                value = &m_intValue;
                break;
                
            case InputType::Double:
                // Apply constraints if enabled
                if (m_hasConstraints)
                {
                    m_doubleValue = std::max(m_minDouble, std::min(m_maxDouble, m_doubleValue));
                }
                value = &m_doubleValue;
                break;
                
            case InputType::String:
                m_stringValue = std::string(m_stringBuffer);
                value = &m_stringValue;
                break;
        }
        
        m_callback(true, value);
    }
}

void UiInputDialog::handleCancel()
{
    if (m_callback)
    {
        InputValue value;
        
        // Create default values for cancelled dialog
        switch (m_type)
        {
            case InputType::Float:
                value = &m_floatValue;
                break;
            case InputType::Int:
                value = &m_intValue;
                break;
            case InputType::Double:
                value = &m_doubleValue;
                break;
            case InputType::String:
                value = &m_stringValue;
                break;
        }
        
        m_callback(false, value);
    }
}