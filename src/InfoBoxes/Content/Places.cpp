// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Places.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "InfoBoxes/Panel/ATCReference.hpp"
#include "InfoBoxes/Panel/ATCSetup.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Formatter/Units.hpp"
#include "Components.hpp"
#include "DataComponents.hpp"
#include "Task/RoutePlannerGlue.hpp"

void
UpdateInfoBoxHomeDistance(InfoBoxData &data) noexcept
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const CommonStats &common_stats = CommonInterface::Calculated().common_stats;

  if (!common_stats.vector_home.IsValid()) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromDistance(common_stats.vector_home.distance);

  if (basic.track_available) {
    Angle bd = common_stats.vector_home.bearing - basic.track;
    data.SetCommentFromBearingDifference(bd);
  } else
    data.SetCommentInvalid();
}

void
UpdateInfoBoxTakeoffDistance(InfoBoxData &data) noexcept
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const FlyingState &flight = CommonInterface::Calculated().flight;

  if (!basic.location_available || !flight.flying ||
      !flight.takeoff_location.IsValid()) {
    data.SetInvalid();
    return;
  }

  const GeoVector vector(basic.location, flight.takeoff_location);
  data.SetValueFromDistance(vector.distance);

  if (basic.track_available)
    data.SetCommentFromBearingDifference(vector.bearing - basic.track);
  else
    data.SetCommentInvalid();
}

void
UpdateInfoBoxTakeoffAltitudeDiff(InfoBoxData &data) noexcept
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated(); 
  const ComputerSettings &computer_settings = CommonInterface::GetComputerSettings();

  const FlyingState &flight = calculated.flight;
  const GlideSettings &glide_settings= computer_settings.task.glide;
  const GlidePolar &glide_polar_task = computer_settings.polar.glide_polar_task;
  const GlidePolar &glide_polar_safety = calculated.glide_polar_safety;
  const SpeedVector &wind = calculated.GetWindOrZero(); //TODO: reference ok here?
  const double &height_min_working = calculated.common_stats.height_min_working;
  const std::optional<double> altitude = basic.GetAnyAltitude();

  if (!basic.location_available
   || !flight.flying 
   || !flight.takeoff_location.IsValid()
   || !altitude.has_value()) {
    data.SetInvalid();
    return;
  }

  //TODO: does it detect airspace intersection as well?

  // TODO: get values from user config
  RoutePlannerConfig route_config = {
    .mode = RoutePlannerConfig::Mode::BOTH,
    .allow_climb = true,
    .use_ceiling = false,
    .safety_height_terrain = 150,
    .reach_calc_mode = RoutePlannerConfig::ReachMode::STRAIGHT,
    .reach_polar_mode = RoutePlannerConfig::Polar::SAFETY,
  };

  RoutePlannerGlue route_planner;
  route_planner.Reset(); //TODO: call required?
  route_planner.SetTerrain(&(*data_components->terrain));

  route_planner.UpdatePolar(
    glide_settings,
    route_config, 
    glide_polar_task, 
    glide_polar_safety, 
    wind, 
    height_min_working);

  auto intersection = route_planner.Intersection(
    AGeoPoint(basic.location, altitude.value()),
    AGeoPoint(flight.takeoff_location, flight.takeoff_altitude)
  );

  // data.SetTitle(takeoff_waypoint_name)
  data.SetValueFromArrival(433.0);

  if (intersection.IsValid()){
    data.SetComment("Intercept!");
    data.SetValueColor(2);
  }else{
    data.SetCommentInvalid();
    data.SetValueColor(0);
  }
}

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel atc_infobox_panels[] = {
  { N_("Reference"), LoadATCReferencePanel },
  { N_("Setup"), LoadATCSetupPanel },
  { nullptr, nullptr }
};

void
UpdateInfoBoxATCRadial(InfoBoxData &data) noexcept
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const GeoPoint &reference =
    CommonInterface::GetComputerSettings().poi.atc_reference;
  const Angle &declination =
    CommonInterface::GetComputerSettings().poi.magnetic_declination;

  if (!basic.location_available || !reference.IsValid()) {
    data.SetInvalid();
    return;
  }

  const GeoVector vector(reference, basic.location);

  const Angle mag_bearing = vector.bearing - declination;
  data.SetValue(mag_bearing);

  FormatDistance(data.comment.buffer(), vector.distance,
                 Unit::NAUTICAL_MILES, true, 1);
}
