#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/G29.hpp"  

// Mock class for hid_device
class MockHidDevice {
public:
    MOCK_METHOD(int, hid_write, (hid_device*, const unsigned char*, size_t));
    MOCK_METHOD(int, hid_read_timeout, (hid_device*, unsigned char*, size_t, int));
};

// Mock class for hidapi functions
class MockHidApi {
public:
    MOCK_METHOD(int, hid_init, ());
    MOCK_METHOD(hid_device*, hid_open, (unsigned short, unsigned short, const wchar_t*));
    MOCK_METHOD(void, hid_close, (hid_device*));
    MOCK_METHOD(int, hid_exit, ());
    MOCK_METHOD(int, hid_read, (hid_device*, unsigned char*, size_t));
};

// Global mock objects
MockHidDevice* g_mockHidDevice;
MockHidApi* g_mockHidApi;

// Override global functions to use our mocks
extern "C" {
    int hid_init() { return g_mockHidApi->hid_init(); }
    hid_device* hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t* serial_number) {
        return g_mockHidApi->hid_open(vendor_id, product_id, serial_number);
    }
    int hid_read(hid_device* device, unsigned char* data, size_t length) {
        return g_mockHidApi->hid_read(device, data, length);
    }
    void hid_close(hid_device* device) { g_mockHidApi->hid_close(device); }
    int hid_exit() { return g_mockHidApi->hid_exit(); }
    int hid_write(hid_device* device, const unsigned char* data, size_t length) {
        return g_mockHidDevice->hid_write(device, data, length);
    }
    int hid_read_timeout(hid_device* device, unsigned char* data, size_t length, int milliseconds) {
        return g_mockHidDevice->hid_read_timeout(device, data, length, milliseconds);
    }
}

class G29Test : public ::testing::Test {
protected:
    void SetUp() override {
        g_mockHidDevice = new MockHidDevice();
        g_mockHidApi = new MockHidApi();
        EXPECT_CALL(*g_mockHidApi, hid_close(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(*g_mockHidApi, hid_exit()).Times(testing::AnyNumber());
    }

    void TearDown() override {
        delete g_mockHidDevice;
        delete g_mockHidApi;
    }
};

TEST_F(G29Test, ConstructorInitializesCorrectly) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

    G29 g29;
}

TEST_F(G29Test, DestructorClosesDeviceAndExits) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    EXPECT_CALL(*g_mockHidApi, hid_close(testing::_));
    EXPECT_CALL(*g_mockHidApi, hid_exit());

    {
        G29 g29;
    }
}

TEST_F(G29Test, ResetSendsResetCommand) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    EXPECT_CALL(*g_mockHidDevice, hid_write(testing::_, testing::_, testing::_)).Times(testing::AtLeast(1));

    G29 g29;
    g29.reset();
}

TEST_F(G29Test, ForceFeedbackConstantSetsForce) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    EXPECT_CALL(*g_mockHidDevice, hid_write(testing::_, testing::_, testing::_)).Times(testing::AtLeast(1));

    G29 g29;
    EXPECT_NO_THROW(g29.forceFeedbackConstant(0.5f));
    EXPECT_THROW(g29.forceFeedbackConstant(1.5f), std::out_of_range);
}

TEST_F(G29Test, SetAutocenterSetsEffect) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    EXPECT_CALL(*g_mockHidDevice, hid_write(testing::_, testing::_, testing::_)).Times(testing::AtLeast(1));

    G29 g29;
    EXPECT_NO_THROW(g29.setAutocenter(0.5f, 0.5f));
    EXPECT_THROW(g29.setAutocenter(1.5f, 0.5f), std::out_of_range);
    EXPECT_THROW(g29.setAutocenter(0.5f, 1.5f), std::out_of_range);
}

TEST_F(G29Test, ForceOffTurnsOffForce) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    EXPECT_CALL(*g_mockHidDevice, hid_write(testing::_, testing::_, testing::_)).Times(testing::AtLeast(1));

    G29 g29;
    g29.forceOff();
}

TEST_F(G29Test, PumpReadsData) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_))
        .WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

    // Prepare mock data that matches the expected format
    std::vector<unsigned char> mockData(16, 0);
    // Fill mockData with appropriate test values
    EXPECT_CALL(*g_mockHidApi, hid_read(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArrayArgument<1>(mockData.begin(), mockData.end()),
            testing::Return(static_cast<int>(mockData.size()))
        ));

    G29 g29;
    ASSERT_NO_THROW({
        
        size_t bytesRead = g29.pump(100);
        EXPECT_EQ(bytesRead, mockData.size());
        std::cout << "Pump method read " << bytesRead << " bytes" << std::endl;
    });
}

TEST_F(G29Test, GetStateReturnsCurrentState) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

    G29 g29;
    auto state = g29.getState();
    EXPECT_FALSE(state.empty());
}

TEST_F(G29Test, IsButtonPressedChecksButtonState) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

    G29 g29;
    EXPECT_FALSE(g29.isButtonPressed("X"));

    g29.updateButtonState({0x28, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05});
    EXPECT_TRUE(g29.isButtonPressed("Square"));

    g29.updateButtonState({0x18, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05});

    EXPECT_TRUE(g29.isButtonPressed("X"));
}

// // Test case: No button pressed
// TEST_F(G29Test, NoButtonPressedReturnsEmptyString) {
//     EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
//     EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));
    

//     G29 g29;
//     g29.updateButtonState({55});
//     EXPECT_EQ(g29.getPressedButton(), "");
// }

// Test case: DPadUp button pressed
TEST_F(G29Test, DPadUpButtonPressedReturnsDPadUp) {
    EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
    EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

    G29 g29;
    g29.updateButtonState({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    EXPECT_EQ(g29.getPressedButton(), "DPadUp");
}

// // Test case: Plus button pressed
// TEST_F(G29Test, PlusButtonPressedReturnsPlusButton) {
//     EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
//     EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

//     G29 g29;
//     g29.updateButtonState({0xC8, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
//     EXPECT_EQ(g29.getPressedButton(), "PlusButton");
// }

// // Test case: L2 button pressed
// TEST_F(G29Test, L2ButtonPressedReturnsL2) {
//     EXPECT_CALL(*g_mockHidApi, hid_init()).WillOnce(testing::Return(0));
//     EXPECT_CALL(*g_mockHidApi, hid_open(0x046d, 0xc24f, testing::_)).WillOnce(testing::Return(reinterpret_cast<hid_device*>(1)));

//     G29 g29;
//     g29.updateButtonState({0xC8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
//     EXPECT_EQ(g29.getPressedButton(), "L2");
// }





int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}