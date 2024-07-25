// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Places.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "InfoBoxes/Panel/ATCReference.hpp"
#include "InfoBoxes/Panel/ATCSetup.hpp"
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Widget/CallbackWidget.hpp"
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

static WaypointPtr takeoff_wp = NULL;


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
  // Display altitude difference to takeoff location. Assuming straight glide
  // and using the task glide polar (current MC). Shows warning if terrain
  // would be in glide path.


  //TODO: Detect airspace intersection
  //TODO: Display distance to takeoff waypoint

  const MoreData &more_data = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const ComputerSettings &computer = CommonInterface::GetComputerSettings();
  const auto &waypoints = *data_components->waypoints;

  const FlyingState &flight = calculated.flight;
  const GlideSettings &glide_settings= computer.task.glide;
  const GlidePolar &glide_polar_task = computer.polar.glide_polar_task;
  const GlidePolar &glide_polar_safety = calculated.glide_polar_safety;
  const SpeedVector &wind = calculated.GetWindOrZero();

  // Find takeoff waypoint
  if (takeoff_wp == NULL && flight.flying && flight.HasTakenOff()){
    takeoff_wp = waypoints.GetNearestLandable(flight.takeoff_location, 5000);
    if (takeoff_wp == NULL){
      // We did not take off near a landable waypoint. Find the auto generated
      // takeoff waypoint.
      takeoff_wp = waypoints.LookupName(_T("(takeoff)"));
    }
  }

  // Check if all required data is available
  if (takeoff_wp == NULL 
      || !more_data.location_available 
      || !more_data.NavAltitudeAvailable()){
    data.SetInvalid();
    return;
  }

  // Calculate arrival altitude difference
  auto target_alt = takeoff_wp->GetElevationOrZero() + computer.task.safety_height_arrival;
  auto target_vector = GeoVector(more_data.location, takeoff_wp->location); 
  const MacCready mac_cready(computer.task.glide, glide_polar_task);
  const GlideState glide_state(
    target_vector,
    target_alt,
    more_data.nav_altitude,
    wind);
  GlideResult glide_result = mac_cready.SolveStraight(glide_state);

  // RoutePlannerConfig &route_config = computer.task.route_planner;
  RoutePlannerConfig route_config = {
    .mode = RoutePlannerConfig::Mode::TERRAIN,
    .allow_climb = false,
    .use_ceiling = false,
    .safety_height_terrain = computer.task.route_planner.safety_height_terrain,
    .reach_calc_mode = RoutePlannerConfig::ReachMode::STRAIGHT,
    .reach_polar_mode = RoutePlannerConfig::Polar::TASK,
  };

  // Check if glide path would intersect with terrain
  RoutePlannerGlue route_planner;
  route_planner.Reset();
  route_planner.SetTerrain(&(*data_components->terrain));
  route_planner.UpdatePolar(
    glide_settings,
    route_config, 
    glide_polar_task, 
    glide_polar_safety, 
    wind, 
    calculated.common_stats.height_min_working);

  auto terrain_intersect = route_planner.Intersection(
    AGeoPoint(more_data.location, more_data.nav_altitude),
    AGeoPoint(takeoff_wp->location, takeoff_wp->elevation)
  );

  // Update InfoBox
  data.SetTitle(_T(takeoff_wp->name.c_str()));
  if (glide_result.IsOk()){
    auto alt_diff = glide_result.pure_glide_altitude_difference;
    data.SetValueFromArrival(alt_diff);
    data.SetCommentFromDistance(target_vector.distance);
    if (alt_diff <= 0.0){
      // Below glide path.
      data.SetValueColor(1);
      data.SetCommentInvalid();
    }else if (terrain_intersect.IsValid()){
      // Above glide path, but glide would intersect terrain.
      data.SetValueColor(5);
      data.SetCommentColor(5);
      data.SetComment("Terrain!");
    }else{
      // Above glide path and no terrain intersection.
      data.SetValueColor(3);
    }
  }else {
    data.SetValueInvalid();
    data.SetCommentColor(1);
    data.SetComment("Glide result not ok");
  }
}

static void
ShowTakeoffAltitudeDiffDetails() noexcept
{
  if (takeoff_wp == NULL){
    return;
  }
  dlgWaypointDetailsShowModal(data_components->waypoints.get(),
                              std::move(takeoff_wp), false);
}

static std::unique_ptr<Widget>
LoadTakeoffAltitudeDiffDetailsPanel([[maybe_unused]] unsigned id) noexcept
{
  return std::make_unique<CallbackWidget>(ShowTakeoffAltitudeDiffDetails);
}

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel takeoff_alt_diff_infobox_panels[] = {
  { N_("Details"), LoadTakeoffAltitudeDiffDetailsPanel },
  { nullptr, nullptr }
};

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
