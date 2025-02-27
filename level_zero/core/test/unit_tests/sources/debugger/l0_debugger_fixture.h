/*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/mocks/mock_compilers.h"
#include "shared/test/common/mocks/mock_device.h"
#include "shared/test/common/mocks/mock_memory_operations_handler.h"
#include "shared/test/common/mocks/mock_sip.h"

#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/core/test/unit_tests/mocks/mock_built_ins.h"
#include "level_zero/core/test/unit_tests/mocks/mock_l0_debugger.h"

namespace L0 {
namespace ult {

struct L0DebuggerFixture {
    void SetUp() {
        NEO::MockCompilerEnableGuard mock(true);
        auto executionEnvironment = new NEO::ExecutionEnvironment();
        auto mockBuiltIns = new MockBuiltins();
        executionEnvironment->prepareRootDeviceEnvironments(1);
        executionEnvironment->rootDeviceEnvironments[0]->builtins.reset(mockBuiltIns);
        memoryOperationsHandler = new NEO::MockMemoryOperations();
        executionEnvironment->rootDeviceEnvironments[0]->memoryOperationsInterface.reset(memoryOperationsHandler);
        executionEnvironment->setDebuggingEnabled();

        hwInfo = *NEO::defaultHwInfo.get();
        hwInfo.featureTable.ftrLocalMemory = true;

        auto isHexadecimalArrayPrefered = HwHelper::get(hwInfo.platform.eRenderCoreFamily).isSipKernelAsHexadecimalArrayPreferred();
        if (isHexadecimalArrayPrefered) {
            MockSipData::useMockSip = true;
        }

        executionEnvironment->rootDeviceEnvironments[0]->setHwInfo(&hwInfo);
        executionEnvironment->initializeMemoryManager();

        neoDevice = NEO::MockDevice::create<NEO::MockDevice>(executionEnvironment, 0u);

        NEO::DeviceVector devices;
        devices.push_back(std::unique_ptr<NEO::Device>(neoDevice));
        driverHandle = std::make_unique<Mock<L0::DriverHandleImp>>();
        driverHandle->enableProgramDebugging = true;

        driverHandle->initialize(std::move(devices));
        device = driverHandle->devices[0];
    }

    void TearDown() {
    }

    std::unique_ptr<Mock<L0::DriverHandleImp>> driverHandle;
    NEO::MockDevice *neoDevice = nullptr;
    L0::Device *device = nullptr;
    NEO::HardwareInfo hwInfo;
    MockMemoryOperations *memoryOperationsHandler = nullptr;
    VariableBackup<bool> mockSipCalled{&NEO::MockSipData::called};
    VariableBackup<NEO::SipKernelType> mockSipCalledType{&NEO::MockSipData::calledType};
    VariableBackup<bool> backupSipInitType{&MockSipData::useMockSip};
};

struct L0DebuggerHwFixture : public L0DebuggerFixture {
    void SetUp() {
        L0DebuggerFixture::SetUp();
        debuggerHw = static_cast<DebuggerL0 *>(neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[neoDevice->getRootDeviceIndex()]->debugger.get());
        neoDevice->setPreemptionMode(PreemptionMode::Disabled);
    }

    void TearDown() {
        L0DebuggerFixture::TearDown();
        debuggerHw = nullptr;
    }
    template <typename GfxFamily>
    MockDebuggerL0Hw<GfxFamily> *getMockDebuggerL0Hw() {
        return static_cast<MockDebuggerL0Hw<GfxFamily> *>(debuggerHw);
    }
    DebuggerL0 *debuggerHw = nullptr;
};

} // namespace ult
} // namespace L0
