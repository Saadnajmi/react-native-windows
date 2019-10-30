// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "NativeUIManager.h"
#include "FrameworkElementViewManager.h"

namespace react::uwp {

class ViewShadowNode;

class ViewViewManager : public FrameworkElementViewManager
{
  using Super = FrameworkElementViewManager;

public:
  explicit ViewViewManager(const std::shared_ptr<react::uwp::NativeUIManager>& spNativeUIManager);

  const char* GetName() const override;

  folly::dynamic GetNativeProps() const override;
  folly::dynamic GetExportedCustomDirectEventTypeConstants() const override;
  facebook::react::ShadowNode* createShadow() const override;

  void UpdateProperties(
    ShadowNodeBase* nodeToUpdate,
    const folly::dynamic& reactDiffMap) override;

  // Yoga Layout
  void SetLayoutProps(
    ShadowNodeBase& nodeToUpdate,
    XamlView viewToUpdate,
    float left,
    float top,
    float width,
    float height) override;

protected:
  XamlView CreateViewCore(int64_t tag) override;
  void TryUpdateView(
    ViewShadowNode* viewShadowNode,
    winrt::Windows::UI::Xaml::Controls::RelativePanel& panel,
    bool useControl);
};

}
