#include "InfoBoxes/Content/Multi.hpp"

#include "Look/Look.hpp"
#include "ui/canvas/Canvas.hpp"
#include "Interface.hpp"
#include "InfoBoxes/Data.hpp"
#include "UIGlobals.hpp"



void
InfoBoxContentMultiDummy::Update(InfoBoxData &data) noexcept
{
  const DerivedInfo &calculated = CommonInterface::Calculated();

  // if (!calculated.altitude_agl_valid) {
    // data.SetInvalid();
    // return;
  // }

  data.SetValueFromAltitude(calculated.altitude_agl);
  data.SetComment("comment");
  data.SetCustom(calculated.altitude_agl + 1);
}

void
InfoBoxContentMultiDummy::OnCustomPaint(Canvas &canvas, const PixelRect &rc) noexcept
{
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const Look &look = UIGlobals::GetLook();

  canvas.SetTextColor(COLOR_BLACK);
  canvas.Select(look.info_box.title_font);

  StaticString<32> text("Hello");

  PixelSize tsize = canvas.CalcTextSize(text);

  int x = std::max(1, (rc.left + rc.right - (int)tsize.width) / 2);
  int y = rc.top;
  canvas.TextAutoClipped({x, y}, text);

  canvas.DrawHLine(rc.GetTopLeft().x, rc.GetTopRight().x, rc.GetCenter().y, COLOR_GREEN);
}