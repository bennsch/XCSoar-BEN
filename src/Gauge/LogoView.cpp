// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LogoView.hpp"
#include "ui/canvas/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Look/FontDescription.hpp"
#include "util/Compiler.h"
#include "Resources.hpp"
#include "Version.hpp"

#include <algorithm>

LogoView::LogoView() noexcept try
  :logo(IDB_LOGO), big_logo(IDB_LOGO_HD),
   title(IDB_TITLE), big_title(IDB_TITLE_HD)
{
#ifndef USE_GDI
  font.Load(FontDescription(Layout::FontScale(10)));
#endif
} catch (...) {
  /* ignore Bitmap/Font loader exceptions */
}

static int
Center(unsigned canvas_size, unsigned element_size)
{
  /* cast to int to force signed integer division, just in case the
     difference is negative */
  return int(canvas_size - element_size) / 2;
}

enum class LogoViewOrientation {
    LANDSCAPE, PORTRAIT, SQUARE,
};

static constexpr PixelSize
EstimateLogoViewSize(LogoViewOrientation orientation,
                     PixelSize logo_size,
                     PixelSize title_size,
                     unsigned spacing) noexcept
{
  switch (orientation) {
  case LogoViewOrientation::LANDSCAPE:
    return {
      logo_size.width + spacing + title_size.width,
      logo_size.height,
    };

  case LogoViewOrientation::PORTRAIT:
    return {
      title_size.width,
      logo_size.height + spacing + title_size.height,
    };

  case LogoViewOrientation::SQUARE:
    return logo_size;
  }

  gcc_unreachable();
}

void
LogoView::draw(Canvas &canvas, const PixelRect &rc) noexcept
{
  if (!big_logo.IsDefined() || !big_title.IsDefined())
    return;

  const unsigned width = rc.GetWidth(), height = rc.GetHeight();

  LogoViewOrientation orientation;
  if (width == height)
    orientation = LogoViewOrientation::SQUARE;
  else if (width > height)
    orientation = LogoViewOrientation::LANDSCAPE;
  else
    orientation = LogoViewOrientation::PORTRAIT;

  /* load bitmaps */
  const bool use_big =
    (orientation == LogoViewOrientation::LANDSCAPE && width >= 510 && height >= 170) ||
    (orientation == LogoViewOrientation::PORTRAIT && width >= 330 && height >= 250) ||
    (orientation == LogoViewOrientation::SQUARE && width >= 210 && height >= 210);
  const Bitmap &bitmap_logo = use_big ? big_logo : logo;
  const Bitmap &bitmap_title = use_big ? big_title : title;

  // Determine logo size
  PixelSize logo_size = bitmap_logo.GetSize();

  // Determine title image size
  PixelSize title_size = bitmap_title.GetSize();

  unsigned spacing = title_size.height / 2;

  const auto estimated_size = EstimateLogoViewSize(orientation, logo_size,
                                                   title_size, spacing);

  const unsigned magnification =
    std::min((width - 16u) / estimated_size.width,
             (height - 16u) / estimated_size.height);

  if (magnification > 1) {
    logo_size.width *= magnification;
    logo_size.height *= magnification;
    title_size.width *= magnification;
    title_size.height *= magnification;
    spacing *= magnification;
  }

  PixelPoint logo_position, title_position;

  // Determine logo and title positions
  switch (orientation) {
  case LogoViewOrientation::LANDSCAPE:
    logo_position.x = Center(width, logo_size.width + spacing + title_size.width);
    logo_position.y = Center(height, logo_size.height);
    title_position.x = logo_position.x + logo_size.width + spacing;
    title_position.y = Center(height, title_size.height);
    break;
  case LogoViewOrientation::PORTRAIT:
    logo_position.x = Center(width, logo_size.width);
    logo_position.y = Center(height, logo_size.height + spacing + title_size.height);
    title_position.x = Center(width, title_size.width);
    title_position.y = logo_position.y + logo_size.height + spacing;
    break;
  case LogoViewOrientation::SQUARE:
    logo_position.x = Center(width, logo_size.width);
    logo_position.y = Center(height, logo_size.height);
    // not needed - silence compiler "may be used uninitialized"
    title_position.x = 0;
    title_position.y = 0;
    break;
  default:
    gcc_unreachable();
  }

  // Draw 'XCSoar N.N' title
  if (orientation != LogoViewOrientation::SQUARE)
    canvas.Stretch(title_position, title_size, bitmap_title);

  // Draw XCSoar swift logo
  canvas.Stretch(logo_position, logo_size, bitmap_logo);

  // Draw full XCSoar version number

#ifndef USE_GDI
  if (!font.IsDefined())
    return;

  canvas.Select(font);
#endif

  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();
  canvas.DrawText({2, 2}, XCSoar_VersionFull);
  canvas.DrawText({2, 2 + (int)canvas.GetFontHeight()}, XCSoar_GitSuffix);
}
