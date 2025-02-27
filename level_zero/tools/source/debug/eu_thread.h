/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/debug_settings/debug_settings_manager.h"

#include <level_zero/ze_api.h>

#include <atomic>
#include <limits>
#include <sstream>
#include <string>
namespace L0 {

class EuThread {
  public:
    enum class State {
        Running,
        Stopped,
        Unavailable
    };

    struct ThreadId {
        union {
            struct {
                uint64_t thread : 4;
                uint64_t eu : 5;
                uint64_t subslice : 10;
                uint64_t slice : 10;
                uint64_t tileIndex : 2;
                uint64_t reserved : 33;
            };
            uint64_t packed;
        };

        ThreadId(uint32_t tile, uint32_t slice, uint32_t subslice, uint32_t eu, uint32_t thread) {
            this->packed = 0;
            this->tileIndex = tile;
            this->slice = slice;
            this->subslice = subslice;
            this->eu = eu;
            this->thread = thread;
        }

        ThreadId(uint32_t tile, ze_device_thread_t thread) {
            this->packed = 0;
            this->tileIndex = tile;
            this->slice = thread.slice;
            this->subslice = thread.subslice;
            this->eu = thread.eu;
            this->thread = thread.thread;
        }

        operator uint64_t() const {
            return packed;
        }
    };

    virtual ~EuThread() = default;

    EuThread(ThreadId threadId) : threadId(threadId) {}

    bool stopThread(uint64_t memHandle) {
        memoryHandle = memHandle;
        if (state == State::Stopped) {
            return false;
        }
        state = State::Stopped;
        return true;
    }

    bool verifyStopped(uint8_t newCounter) {

        PRINT_DEBUGGER_INFO_LOG("EuThread::verifyStopped() Thread: %s newCounter == %d oldCounter == %d", toString().c_str(), (int32_t)newCounter, (int32_t)systemRoutineCounter);

        if (newCounter == systemRoutineCounter) {
            if (newCounter % 2 != 0) {
                if (state == State::Running) {
                    PRINT_DEBUGGER_ERROR_LOG("Thread: %s state RUNNING when thread is stopped. Switching to STOPPED", toString().c_str());
                    DEBUG_BREAK_IF(true);
                }
                state = State::Stopped;
                return true;
            }
        }

        if (newCounter == (systemRoutineCounter + 2)) {
            state = State::Stopped;
            systemRoutineCounter = newCounter;
            return true;
        } else if (newCounter > systemRoutineCounter + 2) {
            PRINT_DEBUGGER_ERROR_LOG("Thread: %s state out of sync.", toString().c_str());
            DEBUG_BREAK_IF(true);
        }

        if (newCounter % 2 == 0) {
            if (state == State::Stopped) {
                PRINT_DEBUGGER_ERROR_LOG("Thread: %s state STOPPED when thread is running. Switching to RUNNING", toString().c_str());
                DEBUG_BREAK_IF(true);
            }
            state = State::Running;
            systemRoutineCounter = newCounter;

            return false;
        }

        state = State::Stopped;
        systemRoutineCounter = newCounter;
        return true;
    }

    bool resumeThread() {
        memoryHandle = invalidHandle;
        if (state != State::Stopped) {
            return false;
        }
        state = State::Running;
        return true;
    }

    bool isStopped() {
        return state == State::Stopped;
    }

    bool isRunning() {
        return state != State::Stopped;
    }

    static std::string toString(ThreadId threadId) {
        std::stringstream threadString;
        threadString << "device index = " << threadId.tileIndex << " slice = " << threadId.slice << " subslice = " << threadId.subslice << " eu = " << threadId.eu
                     << " thread = " << threadId.thread;
        return threadString.str();
    }

    std::string toString() {
        return toString(threadId);
    }

    ThreadId getThreadId() {
        return threadId;
    }

    uint64_t getMemoryHandle() { return memoryHandle; }

  public:
    static constexpr uint64_t invalidHandle = std::numeric_limits<uint64_t>::max();

  protected:
    ThreadId threadId;
    State state = State::Unavailable;
    uint8_t systemRoutineCounter = 0;
    std::atomic<uint64_t> memoryHandle = invalidHandle;
};

static_assert(sizeof(EuThread::ThreadId) == sizeof(uint64_t));
} // namespace L0
