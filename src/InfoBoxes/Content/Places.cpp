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
#include "BackendComponents.hpp"
#include "Task/RoutePlannerGlue.hpp"
#include "Engine/GlideSolvers/MacCready.hpp"
#include "Engine/GlideSolvers/GlideState.hpp"
#include "Engine/GlideSolvers/GlideResult.hpp"
#include "Engine/Waypoint/Waypoints.hpp"

#include <iostream>

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
  const ComputerSettings &computer = CommonInterface::GetComputerSettings();

  const FlyingState &flight = calculated.flight;
  const GlideSettings &glide_settings= computer.task.glide;
  const GlidePolar &glide_polar_task = computer.polar.glide_polar_task;
  const GlidePolar &glide_polar_safety = calculated.glide_polar_safety;
  const SpeedVector &wind = calculated.GetWindOrZero(); //TODO: reference ok here?

  static WaypointPtr takeoff = NULL;

  // Determine takeoff waypoint
  if (takeoff == NULL && 
      flight.flying &&
      flight.HasTakenOff()){
    takeoff = (*data_components->waypoints).GetNearestLandable(flight.takeoff_location, 5000);
    if (takeoff == NULL){
      Waypoint wp(flight.takeoff_location);
      wp.elevation = flight.takeoff_altitude;
      wp.has_elevation = true;
      wp.name = "Takeoff";
      wp.type = Waypoint::Type::OUTLANDING;
      takeoff = std::make_unique<Waypoint>(wp);
    }
  }

  // Check if all required data is available
  if (takeoff == NULL 
      || !basic.location_available 
      || !basic.GetAnyAltitude().has_value()){
    data.SetInvalid();
    return;
  }

  //TODO: chose glide polar from user config
  //TODO: don't show "Details" in InfoBox (currently still showing details for Next AltD)
  auto target_alt = takeoff->GetElevationOrZero() + computer.task.safety_height_arrival;
  auto target_vector = GeoVector(basic.location, takeoff->location); 

  const MacCready mac_cready(computer.task.glide, glide_polar_task);
  const GlideState glide_state(
    target_vector,
    target_alt,
    basic.GetAnyAltitude().value(),
    wind);
  // TODO: Should we use SolveStraight() instead?
  GlideResult glide_result = mac_cready.Solve(glide_state);
  // GlideResult glide_result = mac_cready.SolveStraight(glide_state) 


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
  // route_planner.Synchronise(
  //   *data_components->airspaces,
  //   backend_components->GetAirspaceWarnings(), 
  //   AGeoPoint(basic.location, altitude.value()), 
  //   AGeoPoint(takeoff_location, takeoff_altitude)); //TODO: do I need to add safety height to takeoff_altitude?

  route_planner.UpdatePolar(
    glide_settings,
    route_config, 
    glide_polar_task, 
    glide_polar_safety, 
    wind, 
    calculated.common_stats.height_min_working);

  auto intersection = route_planner.Intersection(
    AGeoPoint(basic.location, basic.GetAnyAltitude().value()),
    AGeoPoint(takeoff->location, takeoff->elevation) //TODO: do I need to add safety height to takeoff_altitude?
  );

  // Update InfoBox
  data.SetTitle(_T(takeoff->name.c_str()));
  if (glide_result.IsOk()){
    // auto alt_diff = glide_result.GetPureGlideAltitudeDifference(basic.GetAnyAltitude().value());
    auto alt_diff = glide_result.pure_glide_altitude_difference;
    data.SetValueFromArrival(alt_diff);
    if (alt_diff <= 0.0){
      data.SetValueColor(1);
      data.SetCommentInvalid();
    }else if (intersection.IsValid()){
      data.SetValueColor(5);
      data.SetCommentColor(5);
      data.SetComment("Terrain!");
    }else{
      data.SetValueColor(3);
      data.SetCommentInvalid();
    }
  }else {
    data.SetValueInvalid();
    data.SetCommentColor(1);
    data.SetComment("Glide result not ok");
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
