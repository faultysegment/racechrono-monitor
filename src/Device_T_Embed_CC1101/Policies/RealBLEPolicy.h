#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "BLEPolicyCallback.h"

class RealBLEPolicy {
    BLEServer* pServer;
    BLECharacteristic* pMonitorConfigChr;
    BLECharacteristic* pMonitorNotificationChr;
    BLEPolicyCallback* ctrl;

    class MyServerCallbacks: public BLEServerCallbacks {
        BLEPolicyCallback* c;
    public:
        MyServerCallbacks(BLEPolicyCallback* c) : c(c) {}
        void onConnect(BLEServer* pServer) {}
        void onDisconnect(BLEServer* pServer) { c->onBLEDisconnected(); }
    };

    class ConfigCallbacks: public BLECharacteristicCallbacks {
        BLEPolicyCallback* c;
    public:
        ConfigCallbacks(BLEPolicyCallback* c) : c(c) {}
        void onWrite(BLECharacteristic* pChr) { c->onConfigWrite(pChr->getValue()); }
    };

    class NotificationCallbacks: public BLECharacteristicCallbacks {
        BLEPolicyCallback* c;
    public:
        NotificationCallbacks(BLEPolicyCallback* c) : c(c) {}
        void onWrite(BLECharacteristic* pChr) { c->onNotificationWrite(pChr->getValue()); }
    };

public:
    void init(const char* name, BLEPolicyCallback* controller) {
        ctrl = controller;
        BLEDevice::init(name);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks(ctrl));

        BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x1ff8));

        pMonitorConfigChr = pService->createCharacteristic(
                                BLEUUID((uint16_t)0x0005),
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_INDICATE
                            );
        pMonitorConfigChr->addDescriptor(new BLE2902());
        pMonitorConfigChr->setCallbacks(new ConfigCallbacks(ctrl));

        pMonitorNotificationChr = pService->createCharacteristic(
                                BLEUUID((uint16_t)0x0006),
                                BLECharacteristic::PROPERTY_WRITE_NR
                            );
        pMonitorNotificationChr->setCallbacks(new NotificationCallbacks(ctrl));

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
