/*
 * Copyright (c) 2024 . All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 *
 * BOULBALAH LAHCEN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. BOULBALAH LAHCEN
 * SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT
 * OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 * Devlopper : BOULBALAH Lahcen
 * Email : lahcen.boulbalah@alten.com
 * Email : lahcen.boulbalah@gmail.com
 */

#include <hidapi/hidapi.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <string>
#include <random>

/**
 * @class G29
 * @brief Represents a Logitech G29 steering wheel controller.
 *
 * This class provides an interface to interact with a Logitech G29 steering wheel,
 * including reading input states and controlling force feedback.
 */
class G29 {
public:
    /**
     * @brief Constructor for the G29 class.
     * 
     * Initializes the HIDAPI library and opens the G29 device.
     * @throw std::runtime_error if initialization or device opening fails.
     */
    G29();

    /**
     * @brief Destructor for the G29 class.
     * 
     * Closes the device and finalizes the HIDAPI library.
     */
    ~G29();

    /**
     * @brief Establishes a connection with the G29 device.
     * 
     * Performs initial setup and resets the device.
     */
    void connect();

    /**
     * @brief Resets the G29 device to its default state.
     */
    void reset();

    /**
     * @brief Sets a constant force feedback effect.
     * 
     * @param val The strength of the effect, ranging from 0.0 to 1.0.
     * @throw std::out_of_range if val is outside the valid range.
     */
    void forceFeedbackConstant(float val);

    /**
     * @brief Sets the auto-centering effect of the steering wheel.
     * 
     * @param strength The strength of the centering effect, ranging from 0.0 to 1.0.
     * @param rate The rate at which the centering effect is applied, ranging from 0.0 to 1.0.
     * @throw std::out_of_range if either parameter is outside the valid range.
     */
    void setAutocenter(float strength, float rate);

    /**
     * @brief Turns off all force feedback effects.
     */
    void forceOff();

    /**
     * @brief Reads data from the G29 device.
     * 
     * @param timeout The maximum time to wait for data, in seconds.
     * @return The number of bytes read.
     */
    size_t pump(int timeout);

    /**
     * @brief Reads input from the G29 and updates the device state.
     */
    void readLoop();

    /**
     * @brief Gets the current state of the G29 device.
     * 
     * @return An unordered map containing the current state of various inputs.
     */
    std::unordered_map<std::string, uint8_t> getState() const;

    /**
     * @brief Checks if a specific button is currently pressed.
     * 
     * @param button The name of the button to check.
     * @return true if the button is pressed, false otherwise.
     */
    bool isButtonPressed(const std::string& button) const;

    /**
     * @brief Gets the name of a currently pressed button.
     * 
     * @return The name of a pressed button, or an empty string if no button is pressed.
     */
    std::string getPressedButton();

private:
    hid_device* device;  ///< Pointer to the HID device.
    std::vector<uint8_t> cache;  ///< Buffer for storing raw input data.
    std::unordered_map<std::string, uint8_t> state;  ///< Current state of analog inputs.
    std::unordered_map<std::string, bool> buttonState;  ///< Current state of buttons.

    /**
     * @brief Updates the device state based on raw input data.
     * 
     * @param byteArray The raw input data from the device.
     */
    void updateState(const std::vector<uint8_t>& byteArray);

    /**
     * @brief Calculates the steering wheel position.
     * 
     * @param start The starting value of the steering position.
     * @param end The ending value of the steering position.
     * @return The calculated steering wheel position.
     */
    uint8_t calculateSteering(uint8_t start, uint8_t end) const;

    /**
     * @brief Updates the state of all buttons based on raw input data.
     * 
     * @param byteArray The raw input data from the device.
     * @return The name of the first pressed button found, or an empty string if no button is pressed.
     */
    std::string updateButtonState(const std::vector<uint8_t>& byteArray);
};