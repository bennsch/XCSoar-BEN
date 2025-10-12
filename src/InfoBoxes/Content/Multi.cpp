#include "InfoBoxes/Content/Multi.hpp"

#include "Look/Look.hpp"
#include "ui/canvas/Canvas.hpp"
#include "Interface.hpp"
#include "InfoBoxes/Data.hpp"
#include "UIGlobals.hpp"
#include "Formatter/TimeFormatter.hpp"



void
InfoBoxContentMultiDummy::Update(InfoBoxData &data) noexcept
{
  const DerivedInfo &calculated = CommonInterface::Calculated();

  // if (!calculated.altitude_agl_valid) {
    // data.SetInvalid();
    // return;
  // }

//   data.SetValueFromAltitude(calculated.altitude_agl);
//   data.SetComment("comment");
  static uint64_t change = 1;
  data.SetCustom(change++);
}

void
InfoBoxContentMultiDummy::OnCustomPaint(Canvas &canvas, const PixelRect &rc) noexcept
{
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const Look &look = UIGlobals::GetLook();
  const FlyingState &flyingState = CommonInterface::Calculated().flight;


  auto flightTimeSec = std::chrono::duration_cast<std::chrono::seconds>(flyingState.flight_time);
  StaticString<32> sTime1, sTime2, sText;
    
  if (flightTimeSec.count() <= 0) {
    sTime1 = "---";
  } else {
    FormatTimeTwoLines(sTime1.buffer(), sTime2.buffer(), flightTimeSec);
  }
  sText = "Hello";


  canvas.SetTextColor(COLOR_BLACK);
  canvas.Select(look.info_box.title_font);
  PixelSize szTime1 = canvas.CalcTextSize(sTime1);
  int xTime1 = std::max(1, (rc.left + rc.right - (int)szTime1.width) / 2);
  int yTime1 = rc.top;
  canvas.TextAutoClipped({xTime1, yTime1}, sTime1);

  canvas.DrawHLine(rc.GetTopLeft().x, rc.GetTopRight().x, yTime1 + szTime1.height, COLOR_BLUE);

  canvas.SetTextColor(COLOR_LIGHT_GRAY);
  canvas.Select(look.info_box.title_font);
  PixelSize szText = canvas.CalcTextSize(sText);
  int xText = std::max(1, (rc.left + rc.right - (int)szText.width) / 2);
  int yText = yTime1 + szTime1.height;
  canvas.TextAutoClipped({xText, yText}, sText);

}