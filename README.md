# G29 Controller Library
Overview

This library provides an interface to interact with a Logitech G29 steering wheel controller. It allows reading input states and controlling force feedback for the device.
Features

- Initialize and connect to the G29 device
- Read input states from the device
- Control force feedback effects
- Handle auto-centering effects
- Query button states

# Installation

To use this library, you need to have hidapi installed. You can install it using the following commands:
``` bash
# On Debian/Ubuntu
sudo apt-get install libhidapi-dev

# On macOS using Homebrew
brew install hidapi

```
# Usage

Here is an example of how to use the G29 library:

``` cpp
#include "G29.h"

int main() {
    try {
        G29 g29;
        g29.connect();
        
        g29.forceFeedbackConstant(0.5);
        g29.setAutocenter(0.7, 0.5);
        
        while (true) {
            g29.readLoop();
            auto state = g29.getState();
            if (g29.isButtonPressed("button_1")) {
                std::cout << "Button 1 is pressed." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        g29.forceOff();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}


```

![Rust::G29rs](https://github.com/misarb/g29rs)

# Contact

For any inquiries, please contact:

  - Developer: BOULBALAH Lahcen
  - Email: lahcen.boulbalah@gmail.com
