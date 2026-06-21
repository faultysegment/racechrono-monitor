#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../../EventBus.h"

class RealBLEPolicy {
    BLEServer* pServer;
    BLECharacteristic* pMonitorConfigChr;
    BLECharacteristic* pMonitorNotificationChr;
    EventBus* bus;

    class MyServerCallbacks: public BLEServerCallbacks {
        EventBus* bus;
    public:
        MyServerCallbacks(EventBus* bus) : bus(bus) {}
        void onConnect(BLEServer* pServer) {
            bus->push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
        }
        void onDisconnect(BLEServer* pServer) {
            bus->push(Event{EventType::BLE_DISCONNECTED, 0, 0, 0});
        }
    };

    class ConfigCallbacks: public BLECharacteristicCallbacks {
        EventBus* bus;
    public:
        ConfigCallbacks(EventBus* bus) : bus(bus) {}
        void onWrite(BLECharacteristic* pChr) {
            Event e{EventType::BLE_CONFIG_MONITOR, 0, 0, 0, pChr->getValue()};
            bus->push(e);
        }
    };

    class NotificationCallbacks: public BLECharacteristicCallbacks {
        EventBus* bus;
    public:
        NotificationCallbacks(EventBus* bus) : bus(bus) {}
        void onWrite(BLECharacteristic* pChr) {
            Event e{EventType::BLE_MONITOR_UPDATE, 0, 0, 0, pChr->getValue()};
            bus->push(e);
        }
    };

public:
    void init(const char* name, EventBus* b) {
        bus = b;
        BLEDevice::init(name);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks(bus));

        BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x1ff8));

        pMonitorConfigChr = pService->createCharacteristic(
                                BLEUUID((uint16_t)0x0005),
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_INDICATE
                            );
        pMonitorConfigChr->addDescriptor(new BLE2902());
        pMonitorConfigChr->setCallbacks(new ConfigCallbacks(bus));

        pMonitorNotificationChr = pService->createCharacteristic(
                                BLEUUID((uint16_t)0x0006),
                                BLECharacteristic::PROPERTY_WRITE_NR
                            );
        pMonitorNotificationChr->setCallbacks(new NotificationCallbacks(bus));

        pService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(pService->getUUID());
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);  
        pAdvertising->setMinPreferred(0x12);
    }

    void startAdvertising() {
        BLEDevice::startAdvertising();
    }

    int getConnectedCount() {
        return pServer ? pServer->getConnectedCount() : 0;
    }

    bool isConfigIndicating() {
        if (pMonitorConfigChr) {
            BLE2902* pDesc = (BLE2902*)pMonitorConfigChr->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
            if (pDesc != nullptr && pDesc->getIndications()) {
                return true;
            }
        }
        return false;
    }

    void indicateConfig(uint8_t* data, size_t len) {
        if (pMonitorConfigChr) {
            pMonitorConfigChr->setValue(data, len);
            pMonitorConfigChr->indicate();
        }
    }
};
