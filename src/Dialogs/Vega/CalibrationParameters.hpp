/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_VEGA_CALIBRATION_PARAMETERS
#define XCSOAR_VEGA_CALIBRATION_PARAMETERS

#include "VegaParametersWidget.hpp"
#include "Language/Language.hpp"

static constexpr
VegaParametersWidget::StaticParameter calibration_parameters[] = {
  { DataField::Type::INTEGER, "TotalEnergyMixingRatio",
    N_("TE mixing"),
    N_("Proportion of TE probe value used in total energy mixing with pitot/static total energy."),
    NULL, 0, 8, 1, _T("%d/8"),
  },
  { DataField::Type::INTEGER, "CalibrationAirSpeed",
    N_("ASI cal."),
    N_("Calibration factor applied to measured airspeed to obtain indicated airspeed."),
    NULL, 0, 200, 1, _T("%d %%"),
  },
  { DataField::Type::INTEGER, "CalibrationTEStatic",
    N_("TE static cal."),
    N_("Calibration factor applied to static pressure used in total energy calculation."),
    NULL, 0, 200, 1, _T("%d %%"),
  },
  { DataField::Type::INTEGER, "CalibrationTEDynamic",
    N_("TE dynamic cal."),
    N_("Calibration factor applied to dynamic pressure used in total energy calculation."),
    NULL, 0, 200, 1, _T("%d %%"),
  },

  /* sentinel */
  { DataField::Type::BOOLEAN }
};

#endif
