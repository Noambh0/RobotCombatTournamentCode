#include <Arduino.h>
#include <Bluepad32.h>

#define AIN1 26
#define AIN2 27
#define PWMA 25
#define STBY 33

#define DEADZONE 40

ControllerPtr controller = nullptr;

void stopMotor() {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(0, 0);
}

void setMotor(int speed) {
    speed = constrain(speed, -255, 255);

    if (speed == 0) {
        stopMotor();
        return;
    }

    if (speed > 0) {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    } else {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
    }

    ledcWrite(0, abs(speed));
}

void onConnectedController(ControllerPtr ctl) {
    if (controller == nullptr) {
        controller = ctl;
        Serial.println("DualSense connected!");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    if (controller == ctl) {
        controller = nullptr;
        stopMotor();
        Serial.println("DualSense disconnected!");
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(STBY, OUTPUT);

    ledcSetup(0, 20000, 8);
    ledcAttachPin(PWMA, 0);

    stopMotor();
    digitalWrite(STBY, HIGH);

    BP32.setup(&onConnectedController, &onDisconnectedController);

    Serial.println("Waiting for DualSense...");
}

void loop() {
    BP32.update();

    if (controller != nullptr && controller->isConnected()) {
        int joystickY = controller->axisY();
        int speed = 0;

        if (abs(joystickY) > 504) {
            speed = joystickY < 0 ? 255 : -255;
        }
        else if (abs(joystickY) > DEADZONE) {
            int magnitude = map(
                abs(joystickY),
                DEADZONE, 504,
                0, 255
            );

            speed = joystickY < 0 ? magnitude : -magnitude;
        }

        setMotor(speed);

        Serial.printf(
            "Joystick: %d | Speed: %d%%\n",
            joystickY,
            map(abs(speed), 0, 255, 0, 100)
        );

    } else {
        stopMotor();
    }

    delay(20);
}