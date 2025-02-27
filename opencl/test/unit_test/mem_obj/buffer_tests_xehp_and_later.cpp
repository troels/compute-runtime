/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/command_container/implicit_scaling.h"
#include "shared/source/command_stream/command_stream_receiver.h"
#include "shared/source/device/sub_device.h"
#include "shared/source/gmm_helper/client_context/gmm_client_context.h"
#include "shared/source/gmm_helper/gmm.h"
#include "shared/source/gmm_helper/gmm_helper.h"
#include "shared/source/helpers/basic_math.h"
#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/helpers/variable_backup.h"
#include "shared/test/common/mocks/mock_device.h"

#include "opencl/source/cl_device/cl_device.h"
#include "opencl/source/helpers/memory_properties_helpers.h"
#include "opencl/source/mem_obj/buffer.h"
#include "opencl/test/unit_test/mocks/mock_buffer.h"
#include "opencl/test/unit_test/mocks/mock_context.h"
#include "opencl/test/unit_test/mocks/mock_gmm.h"
#include "opencl/test/unit_test/mocks/mock_platform.h"
#include "test.h"

#include <functional>

using namespace NEO;

typedef ::testing::Test XeHPAndLaterBufferTests;

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenContextTypeDefaultWhenBufferIsWritableAndOnlyOneTileIsAvailableThenRemainFlagsToTrue) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.CreateMultipleSubDevices.set(1);
    initPlatform();
    EXPECT_EQ(0u, platform()->getClDevice(0)->getNumGenericSubDevices());
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context(platform()->getClDevice(0));
    context.contextType = ContextType::CONTEXT_TYPE_DEFAULT;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            CL_MEM_READ_WRITE,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    surfaceState.setDisableSupportForMultiGpuPartialWrites(false);
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenDebugFlagSetWhenProgramingSurfaceStateThenForceCompressionFormat) {
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;

    DebugManagerStateRestore restorer;
    uint32_t compressionFormat = 3;

    MockContext context;

    auto gmmContext = context.getDevice(0)->getGmmHelper()->getClientContext();
    uint32_t defaultCompressionFormat = gmmContext->getSurfaceStateCompressionFormat(GMM_RESOURCE_FORMAT::GMM_FORMAT_GENERIC_8BIT);

    auto retVal = CL_SUCCESS;
    auto gmm = new Gmm(context.getDevice(0)->getGmmHelper()->getClientContext(), nullptr, 1, 0, false);
    gmm->isCompressionEnabled = true;

    auto buffer = std::unique_ptr<Buffer>(Buffer::create(&context, CL_MEM_READ_WRITE, 1, nullptr, retVal));
    buffer->getGraphicsAllocation(0)->setGmm(gmm, 0);
    buffer->getGraphicsAllocation(0)->setAllocationType(GraphicsAllocation::AllocationType::BUFFER_COMPRESSED);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    {
        buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);
        EXPECT_EQ(defaultCompressionFormat, surfaceState.getCompressionFormat());
    }

    {
        DebugManager.flags.ForceBufferCompressionFormat.set(compressionFormat);
        buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);
        EXPECT_EQ(compressionFormat, surfaceState.getCompressionFormat());
    }
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenContextTypeDefaultWhenBufferIsWritableThenFlipPartialFlagsToFalse) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.CreateMultipleSubDevices.set(4);
    initPlatform();

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context(platform()->getClDevice(0));
    context.contextType = ContextType::CONTEXT_TYPE_DEFAULT;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            CL_MEM_READ_WRITE,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), true, true);

    EXPECT_FALSE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_FALSE(surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenContextTypeUnrestrictiveWhenBufferIsWritableThenFlipPartialFlagsToFalse) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.CreateMultipleSubDevices.set(4);
    initPlatform();

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context(platform()->getClDevice(0));
    context.contextType = ContextType::CONTEXT_TYPE_UNRESTRICTIVE;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            CL_MEM_READ_WRITE,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), true, true);

    EXPECT_FALSE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_FALSE(surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenContextTypeDefaultWhenBufferIsNotWritableThenRemainPartialFlagsToTrue) {
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context;
    context.contextType = ContextType::CONTEXT_TYPE_DEFAULT;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;

    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        CL_MEM_READ_ONLY,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    surfaceState.setDisableSupportForMultiGpuPartialWrites(false);
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), true, false);

    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenContextTypeSpecializedWhenBufferIsWritableThenRemainPartialFlagsToTrue) {
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context;
    context.contextType = ContextType::CONTEXT_TYPE_SPECIALIZED;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;

    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        CL_MEM_READ_WRITE,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    surfaceState.setDisableSupportForMultiGpuPartialWrites(false);
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenDebugFlagForMultiTileSupportWhenSurfaceStateIsSetThenValuesMatch) {
    DebugManagerStateRestore restore;
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context;
    context.contextType = ContextType::CONTEXT_TYPE_SPECIALIZED;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;

    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        CL_MEM_READ_WRITE,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    DebugManager.flags.ForceMultiGpuAtomics.set(0);
    DebugManager.flags.ForceMultiGpuPartialWrites.set(0);

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_EQ(0u, surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_EQ(0u, surfaceState.getDisableSupportForMultiGpuPartialWrites());

    DebugManager.flags.ForceMultiGpuAtomics.set(1);
    DebugManager.flags.ForceMultiGpuPartialWrites.set(1);

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_EQ(1u, surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_EQ(1u, surfaceState.getDisableSupportForMultiGpuPartialWrites());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenNullContextWhenBufferAllocationIsNullThenRemainPartialFlagsToTrue) {
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    auto device = std::unique_ptr<MockDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));

    auto size = MemoryConstants::pageSize;
    auto ptr = alignedMalloc(size, MemoryConstants::pageSize);

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    surfaceState.setDisableSupportForMultiGpuPartialWrites(false);
    Buffer::setSurfaceState(device.get(), &surfaceState, false, false, size, ptr, 0, nullptr, 0, 0, false, false);

    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuPartialWrites());

    alignedFree(ptr);
}

struct MultiGpuGlobalAtomicsBufferTest : public XeHPAndLaterBufferTests,
                                         public ::testing::WithParamInterface<std::tuple<unsigned int, unsigned int, bool, bool, bool>> {
};

HWCMDTEST_P(IGFX_XE_HP_CORE, MultiGpuGlobalAtomicsBufferTest, givenSetArgStatefulCalledThenDisableSupportForMultiGpuAtomicsIsSetCorrectly) {
    unsigned int numAvailableDevices, bufferFlags;
    bool useGlobalAtomics, areMultipleSubDevicesInContext, enableMultiGpuAtomicsOptimization;
    std::tie(numAvailableDevices, bufferFlags, useGlobalAtomics, areMultipleSubDevicesInContext, enableMultiGpuAtomicsOptimization) = GetParam();

    DebugManagerStateRestore restorer;
    DebugManager.flags.CreateMultipleSubDevices.set(numAvailableDevices);
    DebugManager.flags.EnableMultiGpuAtomicsOptimization.set(enableMultiGpuAtomicsOptimization);
    initPlatform();

    if (numAvailableDevices == 1) {
        EXPECT_EQ(0u, platform()->getClDevice(0)->getNumGenericSubDevices());
    } else {
        EXPECT_EQ(numAvailableDevices, platform()->getClDevice(0)->getNumGenericSubDevices());
    }
    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    MockContext context(platform()->getClDevice(0));
    context.contextType = ContextType::CONTEXT_TYPE_DEFAULT;

    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            bufferFlags,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), useGlobalAtomics, areMultipleSubDevicesInContext);

    DeviceBitfield deviceBitfield{static_cast<uint32_t>(maxNBitValue(numAvailableDevices))};
    bool implicitScaling = ImplicitScalingHelper::isImplicitScalingEnabled(deviceBitfield, true);
    bool enabled = implicitScaling;

    if (enableMultiGpuAtomicsOptimization) {
        enabled = useGlobalAtomics && (enabled || areMultipleSubDevicesInContext);
    }

    EXPECT_EQ(!enabled, surfaceState.getDisableSupportForMultiGpuAtomics());
}

HWCMDTEST_P(IGFX_XE_HP_CORE, MultiGpuGlobalAtomicsBufferTest, givenSetSurfaceStateCalledThenDisableSupportForMultiGpuAtomicsIsSetCorrectly) {
    unsigned int numAvailableDevices, bufferFlags;
    bool useGlobalAtomics, areMultipleSubDevicesInContext, enableMultiGpuAtomicsOptimization;
    std::tie(numAvailableDevices, bufferFlags, useGlobalAtomics, areMultipleSubDevicesInContext, enableMultiGpuAtomicsOptimization) = GetParam();

    DebugManagerStateRestore restorer;
    DebugManager.flags.CreateMultipleSubDevices.set(numAvailableDevices);
    DebugManager.flags.EnableMultiGpuAtomicsOptimization.set(enableMultiGpuAtomicsOptimization);
    initPlatform();
    if (numAvailableDevices == 1) {
        EXPECT_EQ(0u, platform()->getClDevice(0)->getNumGenericSubDevices());
    } else {
        EXPECT_EQ(numAvailableDevices, platform()->getClDevice(0)->getNumGenericSubDevices());
    }

    auto size = MemoryConstants::pageSize;
    auto ptr = alignedMalloc(size, MemoryConstants::pageSize);
    MockGraphicsAllocation gfxAllocation(ptr, size);
    gfxAllocation.setMemObjectsAllocationWithWritableFlags(bufferFlags == CL_MEM_READ_WRITE);

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;
    EXPECT_TRUE(surfaceState.getDisableSupportForMultiGpuAtomics());

    surfaceState.setDisableSupportForMultiGpuAtomics(false);
    Buffer::setSurfaceState(&platform()->getClDevice(0)->getDevice(), &surfaceState, false, false, 0, nullptr, 0, &gfxAllocation, bufferFlags, 0, useGlobalAtomics, areMultipleSubDevicesInContext);

    DeviceBitfield deviceBitfield{static_cast<uint32_t>(maxNBitValue(numAvailableDevices))};
    bool implicitScaling = ImplicitScalingHelper::isImplicitScalingEnabled(deviceBitfield, true);
    bool enabled = implicitScaling;

    if (enableMultiGpuAtomicsOptimization) {
        enabled = useGlobalAtomics && (enabled || areMultipleSubDevicesInContext);
    }

    EXPECT_EQ(!enabled, surfaceState.getDisableSupportForMultiGpuAtomics());

    alignedFree(ptr);
}

static unsigned int numAvailableDevices[] = {1, 2};
static unsigned int bufferFlags[] = {CL_MEM_READ_ONLY, CL_MEM_READ_WRITE};

INSTANTIATE_TEST_CASE_P(MultiGpuGlobalAtomicsBufferTest,
                        MultiGpuGlobalAtomicsBufferTest,
                        ::testing::Combine(
                            ::testing::ValuesIn(numAvailableDevices),
                            ::testing::ValuesIn(bufferFlags),
                            ::testing::Bool(),
                            ::testing::Bool(),
                            ::testing::Bool()));

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenBufferAllocationInDeviceMemoryWhenStatelessCompressionIsEnabledThenSetSurfaceStateWithCompressionSettings) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.EnableLocalMemory.set(1);
    DebugManager.flags.EnableStatelessCompressionWithUnifiedMemory.set(1);

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    using AUXILIARY_SURFACE_MODE = typename RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE;

    MockContext context;
    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            CL_MEM_READ_WRITE,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    auto &device = context.getDevice(0)->getDevice();
    auto allocation = buffer->getGraphicsAllocation(device.getRootDeviceIndex());
    auto gmm = new MockGmm(device.getGmmClientContext());
    gmm->isCompressionEnabled = true;
    allocation->setDefaultGmm(gmm);

    EXPECT_TRUE(!MemoryPool::isSystemMemoryPool(allocation->getMemoryPool()));

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    buffer->setArgStateful(&surfaceState, false, false, false, false, device, false, false);

    EXPECT_EQ(RENDER_SURFACE_STATE::COHERENCY_TYPE_GPU_COHERENT, surfaceState.getCoherencyType());
    EXPECT_TRUE(EncodeSurfaceState<FamilyType>::isAuxModeEnabled(&surfaceState, allocation->getDefaultGmm()));
    EXPECT_EQ(static_cast<uint32_t>(DebugManager.flags.FormatForStatelessCompressionWithUnifiedMemory.get()), surfaceState.getCompressionFormat());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenBufferAllocationInHostMemoryWhenStatelessCompressionIsEnabledThenDontSetSurfaceStateWithCompressionSettings) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.EnableStatelessCompressionWithUnifiedMemory.set(1);

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    using AUXILIARY_SURFACE_MODE = typename RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE;

    MockContext context;
    size_t size = 0x1000;
    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(
        Buffer::create(
            &context,
            CL_MEM_READ_WRITE,
            size,
            nullptr,
            retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    EXPECT_TRUE(MemoryPool::isSystemMemoryPool(buffer->getGraphicsAllocation(context.getDevice(0)->getRootDeviceIndex())->getMemoryPool()));

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_EQ(RENDER_SURFACE_STATE::COHERENCY_TYPE_GPU_COHERENT, surfaceState.getCoherencyType());
    EXPECT_EQ(AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_NONE, surfaceState.getAuxiliarySurfaceMode());
    EXPECT_EQ(0u, surfaceState.getCompressionFormat());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenBufferAllocationWithoutGraphicsAllocationWhenStatelessCompressionIsEnabledThenDontSetSurfaceStateWithCompressionSettings) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.EnableStatelessCompressionWithUnifiedMemory.set(1);

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    using AUXILIARY_SURFACE_MODE = typename RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE;

    MockContext context;
    cl_float srcMemory[] = {1.0f, 2.0f, 3.0f, 4.0f};
    std::unique_ptr<Buffer> buffer(Buffer::createBufferHw(
        &context,
        MemoryPropertiesHelper::createMemoryProperties(0, 0, 0, &context.getDevice(0)->getDevice()),
        0,
        0,
        sizeof(srcMemory),
        srcMemory,
        srcMemory,
        0,
        false,
        false,
        false));
    ASSERT_NE(nullptr, buffer);

    RENDER_SURFACE_STATE surfaceState = FamilyType::cmdInitRenderSurfaceState;

    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    EXPECT_EQ(RENDER_SURFACE_STATE::COHERENCY_TYPE_GPU_COHERENT, surfaceState.getCoherencyType());
    EXPECT_EQ(AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_NONE, surfaceState.getAuxiliarySurfaceMode());
    EXPECT_EQ(0u, surfaceState.getCompressionFormat());
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenDebugVariableForcingL1CachingWhenBufferSurfaceStateIsSetThenItIsCachedInL1) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.ForceL1Caching.set(1u);
    MockContext context;
    const auto size = MemoryConstants::pageSize;
    const auto flags = CL_MEM_READ_WRITE;

    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        flags,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    typename FamilyType::RENDER_SURFACE_STATE surfaceState = {};
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    const auto expectedMocs = context.getDevice(0)->getGmmHelper()->getMOCS(GMM_RESOURCE_USAGE_OCL_BUFFER_CONST);
    const auto actualMocs = surfaceState.getMemoryObjectControlState();
    EXPECT_EQ(expectedMocs, actualMocs);
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenDebugVariableForcingL1CachingDisabledWhenBufferSurfaceStateIsSetThenItIsCachedInL3) {
    DebugManagerStateRestore restorer;
    DebugManager.flags.ForceL1Caching.set(0u);
    MockContext context;
    const auto size = MemoryConstants::pageSize;
    const auto flags = CL_MEM_READ_WRITE;

    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        flags,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    typename FamilyType::RENDER_SURFACE_STATE surfaceState = {};
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    const auto expectedMocs = context.getDevice(0)->getGmmHelper()->getMOCS(GMM_RESOURCE_USAGE_OCL_BUFFER);
    const auto actualMocs = surfaceState.getMemoryObjectControlState();
    EXPECT_EQ(expectedMocs, actualMocs);
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenBufferWhenArgumentIsConstAndAuxModeIsOnThenL3DisabledPolicyIsChoosen) {
    MockContext context;
    const auto size = MemoryConstants::pageSize;
    const auto flags = CL_MEM_READ_ONLY;

    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        flags,
        size,
        nullptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    typename FamilyType::RENDER_SURFACE_STATE surfaceState = {};
    buffer->setArgStateful(&surfaceState, true, true, false, false, context.getDevice(0)->getDevice(), false, false);

    const auto expectedMocs = context.getDevice(0)->getGmmHelper()->getMOCS(GMM_RESOURCE_USAGE_OCL_BUFFER_CACHELINE_MISALIGNED);
    const auto actualMocs = surfaceState.getMemoryObjectControlState();
    EXPECT_EQ(expectedMocs, actualMocs);
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenBufferSetSurfaceThatMemoryPtrAndSizeIsAlignedToCachelineThenL1CacheShouldBeOn) {
    MockContext context;

    auto size = MemoryConstants::pageSize;
    auto ptr = (void *)alignedMalloc(size * 2, MemoryConstants::pageSize);

    using RENDER_SURFACE_STATE = typename FamilyType::RENDER_SURFACE_STATE;
    RENDER_SURFACE_STATE surfaceState = {};

    auto device = std::unique_ptr<MockDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));

    Buffer::setSurfaceState(device.get(), &surfaceState, false, false, size, ptr, 0, nullptr, 0, 0, false, false);

    auto mocs = surfaceState.getMemoryObjectControlState();
    auto gmmHelper = device.get()->getGmmHelper();
    EXPECT_EQ(gmmHelper->getMOCS(GMM_RESOURCE_USAGE_OCL_BUFFER_CONST), mocs);

    alignedFree(ptr);
}

HWCMDTEST_F(IGFX_XE_HP_CORE, XeHPAndLaterBufferTests, givenAlignedCacheableNonReadOnlyBufferThenChooseOclBufferPolicy) {
    MockContext context;
    const auto size = MemoryConstants::pageSize;
    const auto ptr = (void *)alignedMalloc(size * 2, MemoryConstants::pageSize);
    const auto flags = CL_MEM_USE_HOST_PTR;

    auto retVal = CL_SUCCESS;
    auto buffer = std::unique_ptr<Buffer>(Buffer::create(
        &context,
        flags,
        size,
        ptr,
        retVal));
    EXPECT_EQ(CL_SUCCESS, retVal);

    typename FamilyType::RENDER_SURFACE_STATE surfaceState = {};
    buffer->setArgStateful(&surfaceState, false, false, false, false, context.getDevice(0)->getDevice(), false, false);

    const auto expectedMocs = context.getDevice(0)->getDevice().getGmmHelper()->getMOCS(GMM_RESOURCE_USAGE_OCL_BUFFER_CONST);
    const auto actualMocs = surfaceState.getMemoryObjectControlState();
    EXPECT_EQ(expectedMocs, actualMocs);

    alignedFree(ptr);
}
