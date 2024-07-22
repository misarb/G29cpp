#include <iostream>
#include <iomanip>
#include "src/G29.hpp" 

void printState(const std::unordered_map<std::string, uint8_t>& state) {
    std::cout << "Steering: " << static_cast<int>(state.at("steering"))
              << " | Throttle: " << static_cast<int>(state.at("throttle"))
              << " | Brake: " << static_cast<int>(state.at("brake"))
              << " | Clutch: " << static_cast<int>(state.at("clutch")) << std::endl;
}

void printButtonStates(const G29& g29) {
    std::vector<std::string> buttons = {"X", "Square", "Triangle", "Circle", "L2", "R2", "L3", "R3",
                                        "Share", "Options", "PS", "DPadUp", "DPadRight", "DPadDown", "DPadLeft",
                                        "PlusButton", "MinusButton", "RotaryDialPress",
                                        "LeftPaddle", "RightPaddle"};

    for (const auto& button : buttons) {
        if (g29.isButtonPressed(button)) {
            std::cout << button << " ";
        }
    }
    std::cout << std::endl;
}

int main() {
    try {
        G29 g29;
        g29.reset();
        // g29.connect();
        std::cout << "G29 connected successfully." << std::endl;
        // g29.reset();
        // std::cout << "G29 reset completed." << std::endl;

        // Set initial force feedback and autocenter
        // g29.forceFeedbackConstant(0.5f);
        // g29.setAutocenter(0.5f, 0.05f);

        std::cout << "Starting main loop. Press Ctrl+C to exit." << std::endl;

        while (true) {
            g29.readLoop();  
            // Get and print the current state
            auto state = g29.getState();
            std::cout << "Current state: ";
            printState(state);

            // Print pressed buttons
            std::cout << g29.getPressedButton() << std::endl;
            // printButtonStates(g29);


            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;

}