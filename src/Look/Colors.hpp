// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/canvas/Color.hpp"

#ifdef XCSOAR_TESTING
static constexpr Color COLOR_XCSOAR_LIGHT = Color(0x2a, 0xcf, 0x1f);
static constexpr Color COLOR_XCSOAR = Color(0x1e, 0x90, 0x16);
static constexpr Color COLOR_XCSOAR_DARK = Color(0x15, 0x67, 0x10);
#else
static constexpr Color COLOR_XCSOAR_LIGHT = Color(0xaa, 0xc9, 0xe4);
static constexpr Color COLOR_XCSOAR = Color(0x3f, 0x76, 0xa8);
static constexpr Color COLOR_XCSOAR_DARK = Color(0x00, 0x31, 0x5e);
#endif

static constexpr uint8_t ALPHA_OVERLAY = 0xA0;
