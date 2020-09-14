/*
 * Copyright (C) 2019-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/page_fault_manager/cpu_page_fault_manager.h"

#include "shared/source/helpers/debug_helpers.h"
#include "shared/source/helpers/ptr_math.h"
#include "shared/source/memory_manager/unified_memory_manager.h"

#include <mutex>

namespace NEO {
void PageFaultManager::insertAllocation(void *ptr, size_t size, SVMAllocsManager *unifiedMemoryManager, void *cmdQ) {
    std::unique_lock<SpinLock> lock{mtx};
    this->memoryData.insert(std::make_pair(ptr, PageFaultData{size, unifiedMemoryManager, cmdQ, false}));
}

void PageFaultManager::removeAllocation(void *ptr) {
    std::unique_lock<SpinLock> lock{mtx};
    auto alloc = memoryData.find(ptr);
    if (alloc != memoryData.end()) {
        auto &pageFaultData = alloc->second;
        if (pageFaultData.isInGpuDomain) {
            allowCPUMemoryAccess(ptr, pageFaultData.size);
        }
        this->memoryData.erase(ptr);
    }
}

void PageFaultManager::moveAllocationToGpuDomain(void *ptr) {
    std::unique_lock<SpinLock> lock{mtx};
    auto alloc = memoryData.find(ptr);
    if (alloc != memoryData.end()) {
        auto &pageFaultData = alloc->second;
        if (pageFaultData.isInGpuDomain == false) {
            this->setAubWritable(false, ptr, pageFaultData.unifiedMemoryManager);
            this->transferToGpu(ptr, pageFaultData.cmdQ);
            this->protectCPUMemoryAccess(ptr, pageFaultData.size);
            pageFaultData.isInGpuDomain = true;
        }
    }
}

void PageFaultManager::moveAllocationsWithinUMAllocsManagerToGpuDomain(SVMAllocsManager *unifiedMemoryManager) {
    std::unique_lock<SpinLock> lock{mtx};
    for (auto &alloc : this->memoryData) {
        auto allocPtr = alloc.first;
        auto &pageFaultData = alloc.second;
        if (pageFaultData.unifiedMemoryManager == unifiedMemoryManager && pageFaultData.isInGpuDomain == false) {
            this->setAubWritable(false, allocPtr, pageFaultData.unifiedMemoryManager);
            this->transferToGpu(allocPtr, pageFaultData.cmdQ);
            this->protectCPUMemoryAccess(allocPtr, pageFaultData.size);
            pageFaultData.isInGpuDomain = true;
        }
    }
}

bool PageFaultManager::verifyPageFault(void *ptr) {
    std::unique_lock<SpinLock> lock{mtx};
    for (auto &alloc : this->memoryData) {
        auto allocPtr = alloc.first;
        auto &pageFaultData = alloc.second;
        if (ptr >= allocPtr && ptr < ptrOffset(allocPtr, pageFaultData.size)) {
            this->broadcastWaitSignal();
            this->allowCPUMemoryAccess(allocPtr, pageFaultData.size);
            this->setAubWritable(true, allocPtr, pageFaultData.unifiedMemoryManager);
            this->transferToCpu(allocPtr, pageFaultData.size, pageFaultData.cmdQ);
            pageFaultData.isInGpuDomain = false;
            return true;
        }
    }
    return false;
}

void PageFaultManager::setAubWritable(bool writable, void *ptr, SVMAllocsManager *unifiedMemoryManager) {
    UNRECOVERABLE_IF(ptr == nullptr);
    auto gpuAlloc = unifiedMemoryManager->getSVMAlloc(ptr)->gpuAllocations.getDefaultGraphicsAllocation();
    gpuAlloc->setAubWritable(writable, GraphicsAllocation::allBanks);
}

void PageFaultManager::waitForCopy() {
    std::unique_lock<SpinLock> lock{mtx};
}

} // namespace NEO
