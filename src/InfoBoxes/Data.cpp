// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Data.hpp"

void
InfoBoxData::Clear() noexcept
{
  title.clear();
  SetInvalid();
}

void
InfoBoxData::SetInvalid() noexcept
{
  custom = 0;
  SetAllColors(0);
  SetValueInvalid();
  SetValue2Invalid();
  SetValueUnit(Unit::UNDEFINED);
  SetValue2Unit(Unit::UNDEFINED);
  SetCommentInvalid();
  myText.clear();
}

void
InfoBoxData::SetValueInvalid() noexcept
{
  SetValueColor(0);
  SetValue(_T("---"));
  SetValueUnit(Unit::UNDEFINED);
}

void
InfoBoxData::SetValue2Invalid() noexcept
{
  SetValue2Color(0);
  SetValue2(_T("---"));
  SetValue2Unit(Unit::UNDEFINED);
}

void
InfoBoxData::SetTitle(const TCHAR *_title) noexcept
{
  title = _title;
  title.CropIncompleteUTF8();
}

void
InfoBoxData::SetValue(const TCHAR *_value) noexcept
{
  value = _value;
}

void
InfoBoxData::SetValue2(const TCHAR *_value) noexcept
{
  value2 = _value;
}

void
InfoBoxData::SetMyText(const TCHAR *_value) noexcept
{
  myText = _value;
}

void
InfoBoxData::SetDual(const bool _dual) noexcept
{
  dual = _dual;
}

void
InfoBoxData::SetComment(const TCHAR *_comment) noexcept
{
  comment = _comment;
  comment.CropIncompleteUTF8();
}

void
InfoBoxData::SetAllColors(unsigned color) noexcept
{
  SetTitleColor(color);
  SetValueColor(color);
  SetValue2Color(color);
  SetCommentColor(color);
}

bool
InfoBoxData::CompareTitle(const InfoBoxData &other) const noexcept
{
  return title == other.title &&
    title_color == other.title_color;
}

bool
InfoBoxData::CompareValue(const InfoBoxData &other) const noexcept
{
  return value == other.value &&
    value_unit == other.value_unit &&
    value_color == other.value_color;
}

bool
InfoBoxData::CompareValue2(const InfoBoxData &other) const noexcept
{
  return value2 == other.value2 &&
    value2_unit == other.value2_unit &&
    value2_color == other.value2_color;
}

bool
InfoBoxData::CompareComment(const InfoBoxData &other) const noexcept
{
  return comment == other.comment &&
    comment_color == other.comment_color;
}
