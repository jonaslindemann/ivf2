#pragma once

/**
 * @file ui_input_dialog.h
 * @brief Declares the UiInputDialog class for generic input dialogs in the ivfui library.
 */

#include <memory>
#include <string>
#include <functional>
#include <variant>
#include <imgui.h>

namespace ivfui {

/**
 * @enum InputType
 * @brief Enumeration of supported input types for the dialog.
 */
enum class InputType {
    Float,
    Int,
    String,
    Double
};

/**
 * @typedef InputValue
 * @brief Variant type that can hold any supported input value type.
 */
typedef std::variant<float*, int*, std::string*, double*> InputValue;

/**
 * @typedef DialogCallback
 * @brief Callback function type for dialog completion.
 * @param accepted True if the user clicked OK, false if cancelled.
 * @param value The input value (valid only if accepted is true).
 */
typedef std::function<void(bool accepted, const InputValue& value)> DialogCallback;

/**
 * @class UiInputDialog
 * @brief Generic input dialog class for various input types.
 *
 * The UiInputDialog class provides a modal dialog for getting user input of various types
 * including float, int, string, and double values. It supports validation, step values for
 * numeric inputs, and callback functions for handling the result.
 */
class UiInputDialog {
private:
    std::string m_title;        ///< Dialog title.
    std::string m_label;        ///< Input field label.
    InputType m_type;           ///< Type of input expected.
    bool m_isOpen;              ///< Whether the dialog is currently open.
    bool m_shouldOpen;          ///< Whether to open the dialog on next frame.
    
    // Input values for different types
    float m_floatValue;         ///< Temporary float value.
    int m_intValue;             ///< Temporary int value.
    std::string m_stringValue;  ///< Temporary string value.
    double m_doubleValue;       ///< Temporary double value.
    
    // Input constraints
    float m_minFloat, m_maxFloat;         ///< Min/max values for float input.
    int m_minInt, m_maxInt;               ///< Min/max values for int input.
    double m_minDouble, m_maxDouble;      ///< Min/max values for double input.
    float m_floatStep, m_floatStepFast;   ///< Step values for float input.
    int m_intStep;                        ///< Step value for int input.
    double m_doubleStep, m_doubleStepFast; ///< Step values for double input.
    size_t m_maxStringLength;             ///< Maximum string length.
    
    // Validation
    bool m_hasConstraints;      ///< Whether input constraints are enabled.
    
    // Callback
    DialogCallback m_callback;  ///< Callback function for dialog completion.
    
    // Internal state
    char* m_stringBuffer;       ///< Buffer for string input.
    static const size_t DEFAULT_STRING_BUFFER_SIZE = 256;

public:
    /**
     * @brief Construct a UiInputDialog.
     * @param title Dialog title.
     * @param label Input field label.
     * @param type Type of input expected.
     */
    UiInputDialog(const std::string& title = "Input", 
                  const std::string& label = "Value:", 
                  InputType type = InputType::Float);

    /**
     * @brief Destructor. Cleans up resources.
     */
    virtual ~UiInputDialog();

    /**
     * @brief Factory method to create a shared pointer to a UiInputDialog instance.
     * @param title Dialog title.
     * @param label Input field label.
     * @param type Type of input expected.
     * @return std::shared_ptr<UiInputDialog> New UiInputDialog instance.
     */
    static std::shared_ptr<UiInputDialog> create(const std::string& title = "Input",
                                                  const std::string& label = "Value:",
                                                  InputType type = InputType::Float);

    /**
     * @brief Open the dialog for float input.
     * @param initialValue Initial value to display.
     * @param callback Callback function for result.
     * @param minValue Minimum allowed value (optional).
     * @param maxValue Maximum allowed value (optional).
     * @param step Step size for +/- buttons (optional).
     * @param stepFast Fast step size for +/- buttons (optional).
     */
    void openFloat(float initialValue, 
                   DialogCallback callback,
                   float minValue = -FLT_MAX, 
                   float maxValue = FLT_MAX,
                   float step = 0.01f,
                   float stepFast = 0.1f);

    /**
     * @brief Open the dialog for int input.
     * @param initialValue Initial value to display.
     * @param callback Callback function for result.
     * @param minValue Minimum allowed value (optional).
     * @param maxValue Maximum allowed value (optional).
     * @param step Step size for +/- buttons (optional).
     */
    void openInt(int initialValue,
                 DialogCallback callback,
                 int minValue = INT_MIN,
                 int maxValue = INT_MAX,
                 int step = 1);

    /**
     * @brief Open the dialog for double input.
     * @param initialValue Initial value to display.
     * @param callback Callback function for result.
     * @param minValue Minimum allowed value (optional).
     * @param maxValue Maximum allowed value (optional).
     * @param step Step size for +/- buttons (optional).
     * @param stepFast Fast step size for +/- buttons (optional).
     */
    void openDouble(double initialValue,
                    DialogCallback callback,
                    double minValue = -DBL_MAX,
                    double maxValue = DBL_MAX,
                    double step = 0.01,
                    double stepFast = 0.1);

    /**
     * @brief Open the dialog for string input.
     * @param initialValue Initial value to display.
     * @param callback Callback function for result.
     * @param maxLength Maximum string length (optional).
     */
    void openString(const std::string& initialValue,
                    DialogCallback callback,
                    size_t maxLength = DEFAULT_STRING_BUFFER_SIZE - 1);

    /**
     * @brief Draw the dialog (should be called each frame).
     * @return bool True if the dialog is still open.
     */
    bool draw();

    /**
     * @brief Check if the dialog is currently open.
     * @return bool True if open.
     */
    bool isOpen() const;

    /**
     * @brief Close the dialog without accepting input.
     */
    void close();

    /**
     * @brief Set the dialog title.
     * @param title New title.
     */
    void setTitle(const std::string& title);

    /**
     * @brief Set the input field label.
     * @param label New label.
     */
    void setLabel(const std::string& label);

private:
    /**
     * @brief Initialize string buffer.
     */
    void initStringBuffer();

    /**
     * @brief Clean up string buffer.
     */
    void cleanupStringBuffer();

    /**
     * @brief Handle dialog acceptance (OK button).
     */
    void handleAccept();

    /**
     * @brief Handle dialog cancellation (Cancel button).
     */
    void handleCancel();
};

/**
 * @typedef UiInputDialogPtr
 * @brief Shared pointer type for UiInputDialog.
 */
typedef std::shared_ptr<UiInputDialog> UiInputDialogPtr;

} // namespace ivfui