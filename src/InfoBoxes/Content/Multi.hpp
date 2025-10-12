#pragma once

#include "InfoBoxes/Content/Base.hpp"


class InfoBoxContentMultiDummy: public InfoBoxContent
{
public:
  void Update(InfoBoxData &data) noexcept override;
  void OnCustomPaint(Canvas &canvas, const PixelRect &rc) noexcept override;
  // const InfoBoxPanel *GetDialogContent() noexcept override;
};