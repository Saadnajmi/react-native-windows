// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <folly/dynamic.h>

#include "ShadowNodeBase.h"

namespace react::uwp {

inline bool
TryUpdateMouseEvents(ShadowNodeBase* node, const std::string& propertyName, const folly::dynamic& propertyValue)
{
  if (propertyName == "onMouseEnter")
    node->m_onMouseEnter = !propertyValue.isNull() && propertyValue.asBool();
  else if (propertyName == "onMouseLeave")
    node->m_onMouseLeave = !propertyValue.isNull() && propertyValue.asBool();
  else if (propertyName == "onMouseMove")
    node->m_onMouseMove = !propertyValue.isNull() && propertyValue.asBool();
  else
    return false;

  return true;
}

inline bool IsValidColorValue(const folly::dynamic& d)
{
  return d.isObject() ? (d.find("windowsbrush") != d.items().end()) : d.isNumber();
}

inline BYTE GetAFromArgb(DWORD v)
{
  return ((BYTE) ((v & 0xFF000000) >> 24));
}
inline BYTE GetRFromArgb(DWORD v)
{
  return ((BYTE) ((v & 0x00FF0000) >> 16));
}
inline BYTE GetGFromArgb(DWORD v)
{
  return ((BYTE) ((v & 0x0000FF00) >> 8));
}
inline BYTE GetBFromArgb(DWORD v)
{
  return ((BYTE) ((v & 0x000000FF)));
}

inline winrt::Windows::UI::Color ColorFrom(const folly::dynamic& d)
{
  UINT argb = static_cast<UINT>(d.asInt());
  return winrt::Windows::UI::Color {
    GetAFromArgb(argb),
    GetRFromArgb(argb),
    GetGFromArgb(argb),
    GetBFromArgb(argb),
  };
}

inline winrt::Windows::UI::Xaml::Media::SolidColorBrush
SolidColorBrushFrom(const folly::dynamic& d)
{
  const auto color = d.isNumber() ? ColorFrom(d) : winrt::Windows::UI::Colors::Transparent();

  winrt::Windows::UI::Xaml::Media::SolidColorBrush brush(color);
  return brush;
}

winrt::Windows::UI::Xaml::Media::Brush BrushFrom(const folly::dynamic& d)
{
  return SolidColorBrushFrom(d);
}

template <class T>
inline bool TryUpdateBackgroundBrush(const T& element, const std::string& propertyName, const folly::dynamic& propertyValue)
{
  if (propertyName == "backgroundColor")
  {
    if (IsValidColorValue(propertyValue))
      element.Background(BrushFrom(propertyValue));
    else if (propertyValue.isNull())
      element.ClearValue(T::BackgroundProperty());

    return true;
  }

  return false;
}

inline double DefaultOrOverride(double defaultValue, double x)
{
  return x != -1 ? x : defaultValue;
};

inline winrt::Windows::UI::Xaml::Thickness GetThickness(double thicknesses[ShadowEdges::CountEdges], bool isRTL)
{
  const double defaultWidth = std::max<double>(0, thicknesses[ShadowEdges::AllEdges]);
  double startWidth = DefaultOrOverride(thicknesses[ShadowEdges::Left], thicknesses[ShadowEdges::Start]);
  double endWidth = DefaultOrOverride(thicknesses[ShadowEdges::Right], thicknesses[ShadowEdges::End]);
  if (isRTL)
    std::swap(startWidth, endWidth);

  // Compute each edge.  Most specific setting wins, so fill from broad to
  // narrow: all, horiz/vert, start/end, left/right
  winrt::Windows::UI::Xaml::Thickness thickness = { defaultWidth, defaultWidth, defaultWidth, defaultWidth };

  if (thicknesses[ShadowEdges::Horizontal] != c_UndefinedEdge)
    thickness.Left = thickness.Right = thicknesses[ShadowEdges::Horizontal];
  if (thicknesses[ShadowEdges::Vertical] != c_UndefinedEdge)
    thickness.Top = thickness.Bottom = thicknesses[ShadowEdges::Vertical];

  if (startWidth != c_UndefinedEdge)
    thickness.Left = startWidth;
  if (endWidth != c_UndefinedEdge)
    thickness.Right = endWidth;
  if (thicknesses[ShadowEdges::Top] != c_UndefinedEdge)
    thickness.Top = thicknesses[ShadowEdges::Top];
  if (thicknesses[ShadowEdges::Bottom] != c_UndefinedEdge)
    thickness.Bottom = thicknesses[ShadowEdges::Bottom];

  return thickness;
}

template <class T>
void SetBorderThickness(ShadowNodeBase* node, const T& element, ShadowEdges edge, double margin)
{
  node->m_border[edge] = margin;
  winrt::Windows::UI::Xaml::Thickness thickness =
    GetThickness(node->m_border, element.FlowDirection() == winrt::Windows::UI::Xaml::FlowDirection::RightToLeft);
  element.BorderThickness(thickness);
}

template <class T>
bool TryUpdateBorderProperties(
  ShadowNodeBase* node,
  const T& element,
  const std::string& propertyName,
  const folly::dynamic& propertyValue)
{
  bool isBorderProperty = true;

  if (propertyName == "borderColor")
  {
    if (IsValidColorValue(propertyValue))
      element.BorderBrush(BrushFrom(propertyValue));
    else if (propertyValue.isNull())
      element.ClearValue(T::BorderBrushProperty());
  }
  else if (propertyName == "borderLeftWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::Left, propertyValue.asDouble());
  }
  else if (propertyName == "borderTopWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::Top, propertyValue.asDouble());
  }
  else if (propertyName == "borderRightWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::Right, propertyValue.asDouble());
  }
  else if (propertyName == "borderBottomWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::Bottom, propertyValue.asDouble());
  }
  else if (propertyName == "borderStartWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::Start, propertyValue.asDouble());
  }
  else if (propertyName == "borderEndWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::End, propertyValue.asDouble());
  }
  else if (propertyName == "borderWidth")
  {
    if (propertyValue.isNumber())
      SetBorderThickness(node, element, ShadowEdges::AllEdges, propertyValue.asDouble());
  }
  else
  {
    isBorderProperty = false;
  }

  return isBorderProperty;
}

inline void
UpdateCornerRadiusValueOnNode(ShadowNodeBase* node, ShadowCorners corner, const folly::dynamic& propertyValue)
{
  if (propertyValue.isNumber())
    node->m_cornerRadius[corner] = propertyValue.asDouble();
  else
    node->m_cornerRadius[corner] = c_UndefinedEdge;
}

bool TryUpdateCornerRadiusOnNode(
  ShadowNodeBase* node,
  const std::string& propertyName,
  const folly::dynamic& propertyValue)
{
  if (propertyName == "borderTopLeftRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::TopLeft, propertyValue);
  }
  else if (propertyName == "borderTopRightRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::TopRight, propertyValue);
  }
  else if (propertyName == "borderTopStartRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::TopStart, propertyValue);
  }
  else if (propertyName == "borderTopEndRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::TopEnd, propertyValue);
  }
  else if (propertyName == "borderBottomRightRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::BottomRight, propertyValue);
  }
  else if (propertyName == "borderBottomLeftRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::BottomLeft, propertyValue);
  }
  else if (propertyName == "borderBottomStartRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::BottomStart, propertyValue);
  }
  else if (propertyName == "borderBottomEndRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::BottomEnd, propertyValue);
  }
  else if (propertyName == "borderRadius")
  {
    UpdateCornerRadiusValueOnNode(node, ShadowCorners::AllCorners, propertyValue);
  }
  else
  {
    return false;
  }

  return true;
}

inline winrt::Windows::UI::Xaml::CornerRadius GetCornerRadius(
  double (&cornerRadii)[ShadowCorners::CountCorners],
  bool isRTL)
{
  winrt::Windows::UI::Xaml::CornerRadius cornerRadius;
  const double defaultRadius = std::max<double>(0, cornerRadii[ShadowCorners::AllCorners]);
  double topStartRadius = DefaultOrOverride(cornerRadii[ShadowCorners::TopLeft], cornerRadii[ShadowCorners::TopStart]);
  double topEndRadius = DefaultOrOverride(cornerRadii[ShadowCorners::TopRight], cornerRadii[ShadowCorners::TopEnd]);
  double bottomStartRadius =
    DefaultOrOverride(cornerRadii[ShadowCorners::BottomLeft], cornerRadii[ShadowCorners::BottomStart]);
  double bottomEndRadius =
    DefaultOrOverride(cornerRadii[ShadowCorners::BottomRight], cornerRadii[ShadowCorners::BottomEnd]);
  if (isRTL)
  {
    std::swap(topStartRadius, topEndRadius);
    std::swap(bottomStartRadius, bottomEndRadius);
  }

  cornerRadius.TopLeft = DefaultOrOverride(defaultRadius, topStartRadius);
  cornerRadius.TopRight = DefaultOrOverride(defaultRadius, topEndRadius);
  cornerRadius.BottomLeft = DefaultOrOverride(defaultRadius, bottomStartRadius);
  cornerRadius.BottomRight = DefaultOrOverride(defaultRadius, bottomEndRadius);

  return cornerRadius;
}

template <class T>
inline void UpdateCornerRadiusOnElement(ShadowNodeBase* node, const T& element)
{
  winrt::Windows::UI::Xaml::CornerRadius cornerRadius =
    GetCornerRadius(node->m_cornerRadius, element.FlowDirection() == winrt::Windows::UI::Xaml::FlowDirection::RightToLeft);
  element.CornerRadius(cornerRadius);
}


}
