/*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/os_interface/device_factory.h"
#include "shared/source/os_interface/linux/drm_neo.h"
#include "shared/source/os_interface/os_interface.h"

#include "test.h"

#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/tools/source/sysman/sysman.h"
#include "level_zero/tools/test/unit_tests/sources/sysman/linux/mock_procfs_access_fixture.h"
#include "level_zero/tools/test/unit_tests/sources/sysman/linux/mock_sysfs_access_fixture.h"

#include "sysman/linux/os_sysman_imp.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using namespace NEO;

namespace L0 {
namespace ult {
constexpr int mockFd = 0;
class SysmanMockDrm : public Drm {
  public:
    SysmanMockDrm(RootDeviceEnvironment &rootDeviceEnvironment) : Drm(std::make_unique<HwDeviceIdDrm>(mockFd, ""), rootDeviceEnvironment) {}
};

class PublicLinuxSysmanImp : public L0::LinuxSysmanImp {
  public:
    using LinuxSysmanImp::mapOfSubDeviceIdToPmtObject;
    using LinuxSysmanImp::pDrm;
    using LinuxSysmanImp::pFsAccess;
    using LinuxSysmanImp::pFwUtilInterface;
    using LinuxSysmanImp::pPmuInterface;
    using LinuxSysmanImp::pProcfsAccess;
    using LinuxSysmanImp::pSysfsAccess;
};

class SysmanDeviceFixture : public DeviceFixture, public ::testing::Test {
  public:
    Mock<LinuxSysfsAccess> *pSysfsAccess = nullptr;
    Mock<LinuxProcfsAccess> *pProcfsAccess = nullptr;
    void SetUp() override {
        DeviceFixture::SetUp();
        neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[device->getRootDeviceIndex()]->osInterface = std::make_unique<NEO::OSInterface>();
        auto &osInterface = device->getOsInterface();
        osInterface.setDriverModel(std::make_unique<SysmanMockDrm>(const_cast<NEO::RootDeviceEnvironment &>(neoDevice->getRootDeviceEnvironment())));
        setenv("ZES_ENABLE_SYSMAN", "1", 1);
        device->setSysmanHandle(new SysmanDeviceImp(device->toHandle()));
        pSysmanDevice = device->getSysmanHandle();
        pSysmanDeviceImp = static_cast<SysmanDeviceImp *>(pSysmanDevice);
        pOsSysman = pSysmanDeviceImp->pOsSysman;
        pLinuxSysmanImp = static_cast<PublicLinuxSysmanImp *>(pOsSysman);

        pSysfsAccess = new NiceMock<Mock<LinuxSysfsAccess>>;
        pProcfsAccess = new NiceMock<Mock<LinuxProcfsAccess>>;
        pLinuxSysmanImp->pSysfsAccess = pSysfsAccess;
        pLinuxSysmanImp->pProcfsAccess = pProcfsAccess;

        ON_CALL(*pSysfsAccess, getRealPath(_, Matcher<std::string &>(_)))
            .WillByDefault(::testing::Invoke(pSysfsAccess, &Mock<LinuxSysfsAccess>::getRealPathVal));
        ON_CALL(*pProcfsAccess, getFileName(_, _, Matcher<std::string &>(_)))
            .WillByDefault(::testing::Invoke(pProcfsAccess, &Mock<LinuxProcfsAccess>::getMockFileName));

        pSysmanDeviceImp->init();
    }
    void TearDown() override {
        DeviceFixture::TearDown();
        unsetenv("ZES_ENABLE_SYSMAN");
    }

    SysmanDevice *pSysmanDevice = nullptr;
    SysmanDeviceImp *pSysmanDeviceImp = nullptr;
    OsSysman *pOsSysman = nullptr;
    PublicLinuxSysmanImp *pLinuxSysmanImp = nullptr;
};

class SysmanMultiDeviceFixture : public MultiDeviceFixture, public ::testing::Test {
  public:
    Mock<LinuxSysfsAccess> *pSysfsAccess = nullptr;
    Mock<LinuxProcfsAccess> *pProcfsAccess = nullptr;
    void SetUp() override {
        MultiDeviceFixture::SetUp();
        device = driverHandle->devices[0];
        neoDevice = device->getNEODevice();
        neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[device->getRootDeviceIndex()]->osInterface = std::make_unique<NEO::OSInterface>();
        auto &osInterface = device->getOsInterface();
        osInterface.setDriverModel(std::make_unique<SysmanMockDrm>(const_cast<NEO::RootDeviceEnvironment &>(neoDevice->getRootDeviceEnvironment())));
        setenv("ZES_ENABLE_SYSMAN", "1", 1);
        device->setSysmanHandle(new SysmanDeviceImp(device->toHandle()));
        pSysmanDevice = device->getSysmanHandle();
        for (auto &subDevice : static_cast<DeviceImp *>(device)->subDevices) {
            static_cast<DeviceImp *>(subDevice)->setSysmanHandle(pSysmanDevice);
        }

        pSysmanDeviceImp = static_cast<SysmanDeviceImp *>(pSysmanDevice);
        pOsSysman = pSysmanDeviceImp->pOsSysman;
        pLinuxSysmanImp = static_cast<PublicLinuxSysmanImp *>(pOsSysman);

        pSysfsAccess = new NiceMock<Mock<LinuxSysfsAccess>>;
        pProcfsAccess = new NiceMock<Mock<LinuxProcfsAccess>>;
        pLinuxSysmanImp->pSysfsAccess = pSysfsAccess;
        pLinuxSysmanImp->pProcfsAccess = pProcfsAccess;

        ON_CALL(*pSysfsAccess, getRealPath(_, Matcher<std::string &>(_)))
            .WillByDefault(::testing::Invoke(pSysfsAccess, &Mock<LinuxSysfsAccess>::getRealPathVal));
        ON_CALL(*pProcfsAccess, getFileName(_, _, Matcher<std::string &>(_)))
            .WillByDefault(::testing::Invoke(pProcfsAccess, &Mock<LinuxProcfsAccess>::getMockFileName));

        pSysmanDeviceImp->init();
        subDeviceCount = numSubDevices;
    }
    void TearDown() override {
        unsetenv("ZES_ENABLE_SYSMAN");
        MultiDeviceFixture::TearDown();
    }

    SysmanDevice *pSysmanDevice = nullptr;
    SysmanDeviceImp *pSysmanDeviceImp = nullptr;
    OsSysman *pOsSysman = nullptr;
    PublicLinuxSysmanImp *pLinuxSysmanImp = nullptr;
    NEO::Device *neoDevice = nullptr;
    L0::Device *device = nullptr;
    uint32_t subDeviceCount = 0u;
};

class PublicFsAccess : public L0::FsAccess {
  public:
    using FsAccess::accessSyscall;
};

class PublicSysfsAccess : public L0::SysfsAccess {
  public:
    using SysfsAccess::accessSyscall;
};

} // namespace ult
} // namespace L0
