#include "G29.hpp"
#include <iostream>
#include <algorithm>  

G29::G29() {
    if (hid_init() != 0) {
        throw std::runtime_error("Failed to initialize HIDAPI");
    }

    device = hid_open(0x046d, 0xc24f, nullptr);
    if (!device) {
        throw std::runtime_error("Failed to open G29 device");
        hid_exit();
    }

    cache.resize(16, 0);
    state["steering"] = 255;
    state["throttle"] = 255;
    state["clutch"] = 255;
    state["brake"] = 255;
}

G29::~G29() {
    if (device) {
        hid_close(device);
    }
    hid_exit();
}

void G29::connect() {
    pump(10);
    // reset();
}

void G29::reset() {
    uint8_t msg1[] = {0xf8, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t msg2[] = {0xf8, 0x09, 0x05, 0x01, 0x01, 0x00, 0x00};

    hid_write(device, msg1, sizeof(msg1));
    hid_write(device, msg2, sizeof(msg2));

    std::this_thread::sleep_for(std::chrono::seconds(10));
}

void G29::forceFeedbackConstant(float val) {
    if (val < 0.0f || val > 1.0f) {
        throw std::out_of_range("Value must be in range of 0 to 1");
    }

    uint8_t val_scale = static_cast<uint8_t>(std::round(val * 255.0f));
    uint8_t msg[] = {0x14, 0x00, val_scale, 0x00, 0x00, 0x00, 0x00};
    hid_write(device, msg, sizeof(msg));

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void G29::setAutocenter(float strength, float rate) {
    if (strength < 0.0f || strength > 1.0f) {
        throw std::out_of_range("Strength must be in range of 0 to 1");
    }

    if (rate < 0.0f || rate > 1.0f) {
        throw std::out_of_range("Rate must be in range of 0 to 1");
    }

    uint8_t strength_scale = static_cast<uint8_t>(std::round(strength * 255.0f));
    uint8_t rate_scale = static_cast<uint8_t>(std::round(rate * 255.0f));
    uint8_t msg[] = {0x05, 0x00, strength_scale, rate_scale, 0x00, 0x00, 0x00};
    hid_write(device, msg, sizeof(msg));

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void G29::forceOff() {
    uint8_t msg[] = {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    hid_write(device, msg, sizeof(msg));

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

size_t G29::pump(int timeout) {
    size_t bytes_read = 0;
    auto start = std::chrono::high_resolution_clock::now();

    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < timeout) {
        bytes_read = hid_read(device, cache.data(), cache.size());
        if (bytes_read > 0) {
            break;
        }
    }

    return bytes_read;
}

void G29::readLoop() {
    size_t bytes_read = pump(1);
    if (bytes_read > 0) {
        updateState(cache);
    }
}

std::unordered_map<std::string, uint8_t> G29::getState() const {
    return state;
}

void G29::updateState(const std::vector<uint8_t>& byteArray) {
    if (byteArray.size() != 16) {
        return;
    }

    state["steering"] = calculateSteering(byteArray[4], byteArray[5]);
    state["throttle"] = byteArray[6];
    state["clutch"] = byteArray[8];
    state["brake"] = byteArray[7];

    updateButtonState(byteArray);
}

uint8_t G29::calculateSteering(uint8_t start, uint8_t end) const {
    if (start == 0 && end == 0) {
        return 255;
    }

    uint8_t result = 0;
    if (start > end) {
        result = start - end;
    } else {
        result = end - start;
    }

    return result;
}

std::string G29::updateButtonState(const std::vector<uint8_t>& byteArray) {
    if (byteArray.size() < 16) return "";
    //buttonState.clear();

     // Correct the conditions and ensure they are distinct
    buttonState["X"] = (byteArray[0] & 0x18) == 0x18;
    buttonState["Square"] = (byteArray[0] & 0x28) == 0x28;
    buttonState["Triangle"] = (byteArray[0] & 0x88) == 0x88;
    buttonState["Circle"] = (byteArray[0] & 0x48) == 0x48;

    buttonState["L2"] = (byteArray[1] & 0x08) == 0x08;
    buttonState["R2"] = (byteArray[1] & 0x04) == 0x04;
    buttonState["L3"] = (byteArray[1] & 0x80) == 0x80;
    buttonState["R3"] = (byteArray[1] & 0x40) == 0x40;

    buttonState["DPadUp"] = (byteArray[0] & 0x0F) == 0x00;  // Mask with 0x0F for directional checks
    buttonState["DPadDown"] = (byteArray[0] & 0x0F) == 0x04;
    buttonState["DPadLeft"] = (byteArray[0] & 0x0F) == 0x06;
    buttonState["DPadRight"] = (byteArray[0] & 0x0F) == 0x02;

    buttonState["RotaryDialPress"] = (byteArray[3] & 0x08) == 0x08;

    buttonState["PlusButton"] = (byteArray[2] & 0x80) == 0x80;
    buttonState["MinusButton"] = (byteArray[3] & 0x01) == 0x01;

    buttonState["LeftPaddle"] = (byteArray[1] & 0x02) == 0x02;
    buttonState["RightPaddle"] = (byteArray[1] & 0x01) == 0x01;

    buttonState["Share"] = (byteArray[1] & 0x10) == 0x10;
    buttonState["Options"] = (byteArray[1] & 0x20) == 0x20;
    buttonState["PS"] = (byteArray[3] & 0x10) == 0x10;
    // std::cout << "Button pressed: " << getPressedButton() << std::endl;

    if (byteArray[0] == 0x18) return "X";
    if (byteArray[0] == 0x28) return "Square";
    if (byteArray[0] == 0x88) return "Triangle";
    if (byteArray[0] == 0x48) return "Circle";

    if (byteArray[1] == 0x08) return "L2";
    if (byteArray[1] == 0x04) return "R2";
    if (byteArray[1] == 0x80) return "L3";
    if (byteArray[1] == 0x40) return "R3";

    if (byteArray[0] == 0x00) return "DPadUp";
    if (byteArray[0] == 0x04) return "DPadDown";
    if (byteArray[0] == 0x06) return "DPadLeft";
    if (byteArray[0] == 0x02) return "DPadRight";

    if (byteArray[3] == 0x08) return "RotaryDialPress";

    if (byteArray[2] == 0x80) return "PlusButton";
    if (byteArray[3] == 0x01) return "MinusButton";

    if (byteArray[1] == 0x02) return "LeftPaddle";
    if (byteArray[1] == 0x01) return "RightPaddle";

    if (byteArray[1] == 0x10) return "Share";
    if (byteArray[1] == 0x20) return "Options";
    if (byteArray[3] == 0x10) return "PS";

    return "";
}




bool G29::isButtonPressed(const std::string& button) const {
    auto it = buttonState.find(button);
    if (it != buttonState.end()) {
        return it->second;
    }
    return false;
}

std::string G29::getPressedButton()  {
    return updateButtonState(cache);
}
