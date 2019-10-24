// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>

namespace react {
namespace uwp {

typedef winrt::Windows::UI::Xaml::DependencyObject XamlView;

inline int64_t GetTag(XamlView view)
{
  return view.GetValue(winrt::Windows::UI::Xaml::FrameworkElement::TagProperty())
    .as<winrt::Windows::Foundation::IPropertyValue>()
    .GetInt64();
}

inline void SetTag(XamlView view, int64_t tag)
{
  view.SetValue(
    winrt::Windows::UI::Xaml::FrameworkElement::TagProperty(),
    winrt::Windows::Foundation::PropertyValue::CreateInt64(tag));
}

inline void SetTag(XamlView view, winrt::Windows::Foundation::IInspectable tag)
{
  SetTag(view, tag.as<winrt::Windows::Foundation::IPropertyValue>().GetInt64());
}

inline bool IsValidTag(winrt::Windows::Foundation::IPropertyValue value)
{
  assert(value);
  return (value.Type() == winrt::Windows::Foundation::PropertyType::Int64);
}

inline int64_t GetTag(winrt::Windows::Foundation::IPropertyValue value)
{
  assert(value);
  return value.GetInt64();
}

inline winrt::Windows::Foundation::IPropertyValue GetTagAsPropertyValue(winrt::Windows::UI::Xaml::FrameworkElement fe)
{
  assert(fe);
  return fe.GetValue(winrt::Windows::UI::Xaml::FrameworkElement::TagProperty())
    .try_as<winrt::Windows::Foundation::IPropertyValue>();
}

} // namespace uwp
} // namespace react
