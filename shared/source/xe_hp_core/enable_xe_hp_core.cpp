/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/os_interface/hw_info_config.h"
#include "shared/source/xe_hp_core/hw_cmds.h"

#include "opencl/source/helpers/enable_product.inl"

namespace NEO {

#ifdef SUPPORT_XE_HP_SDV
static EnableGfxProductHw<IGFX_XE_HP_SDV> enableGfxProductHwXEHP;
#endif
} // namespace NEO
