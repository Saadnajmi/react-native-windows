// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <winrt/Windows.UI.Composition.h>
#include "ReactRootView.h"
#include "ViewManagerBase.h"
#include "NativeUIManager.h"

namespace react {
namespace uwp {

class FrameworkElementViewManager : public ViewManagerBase
{
  using Super = ViewManagerBase;

public:
  FrameworkElementViewManager(const std::shared_ptr<react::uwp::NativeUIManager>& spNativeUIManager);

  folly::dynamic GetNativeProps() const override;
  void UpdateProperties(
    ShadowNodeBase* nodeToUpdate,
    const folly::dynamic& reactDiffMap) override;

  // Helper functions related to setting/updating TransformMatrix
  void RefreshTransformMatrix(ShadowNodeBase* shadowNode);
  void StartTransformAnimation(
    winrt::Windows::UI::Xaml::UIElement uielement,
    winrt::Windows::UI::Composition::CompositionPropertySet transformPS);

  virtual void TransferProperties(XamlView oldView, XamlView newView) override;

protected:
  void TransferProperty(
    XamlView oldView,
    XamlView newView,
    winrt::Windows::UI::Xaml::DependencyProperty dp);

  void TransferProperty(
    XamlView oldView,
    XamlView newView,
    winrt::Windows::UI::Xaml::DependencyProperty oldViewDP,
    winrt::Windows::UI::Xaml::DependencyProperty newViewDP);

private:
  void ApplyTransformMatrix(
    winrt::Windows::UI::Xaml::UIElement uielement,
    ShadowNodeBase* shadowNode,
    winrt::Windows::Foundation::Numerics::float4x4 transformMatrix);
};

} // namespace uwp
} // namespace react
