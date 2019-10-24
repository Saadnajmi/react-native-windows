// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include "ReactRootView.h"

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.h>

namespace winrt {
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media;
} // namespace winrt

namespace react::uwp {

ReactRootView::ReactRootView(XamlView rootView)
  : m_rootView(rootView)
{
}

XamlView ReactRootView::GetXamlView() const noexcept
{
  return m_rootView;
}


void ReactRootView::SetJSComponentName(std::string&& mainComponentName) noexcept
{
  m_jsComponentName = std::move(mainComponentName);
}

void ReactRootView::ResetView()
{
}

std::string ReactRootView::JSComponentName() const noexcept
{
  return m_jsComponentName;
}

int64_t ReactRootView::GetActualHeight() const
{
  auto element = m_rootView.as<winrt::FrameworkElement>();
  return static_cast<int64_t>(element.ActualHeight());
}

int64_t ReactRootView::GetActualWidth() const
{
  auto element = m_rootView.as<winrt::FrameworkElement>();
  return static_cast<int64_t>(element.ActualWidth());
}

int64_t ReactRootView::GetTag() const
{
  return m_tag;
}

void ReactRootView::SetTag(int64_t tag)
{
  m_tag = tag;
}

void ReactRootView::blur(XamlView const&) noexcept
{
  void; // todo
}

}
