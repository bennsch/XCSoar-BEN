// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/canvas/Color.hpp"

#ifdef XCSOAR_TESTING
static constexpr Color COLOR_XCSOAR_LIGHT = Color(0xed, 0x90, 0x90);
static constexpr Color COLOR_XCSOAR = Color(0xd0, 0x17, 0x17);
static constexpr Color COLOR_XCSOAR_DARK = Color(0x5d, 0x0a, 0x0a);
#else
static constexpr Color COLOR_XCSOAR_LIGHT = Color(0xd6, 0x88, 0xfa);
static constexpr Color COLOR_XCSOAR = Color(0xbf, 0x45, 0xf7);
static constexpr Color COLOR_XCSOAR_DARK = Color(0xb0, 0x1a, 0xf5);
#endif

static constexpr uint8_t ALPHA_OVERLAY = 0xA0;
