// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <IReactRootView.h>
#include <folly/dynamic.h>
#include "XamlView.h"

#include <stdint.h>
#include <memory>
#include <string>

namespace react {
namespace uwp {


struct IXamlReactControl
{
  virtual void blur(XamlView const& xamlView) noexcept = 0;
};

struct IReactInstance;
class ReactControl;

class ReactRootView : public std::enable_shared_from_this<ReactRootView>,
  public react::uwp::IXamlReactControl,
  public facebook::react::IReactRootView
{
public:
  ReactRootView(XamlView rootView);

  std::shared_ptr<IReactInstance> GetReactInstance() const noexcept;
  XamlView GetXamlView() const noexcept;
  void SetJSComponentName(std::string&& mainComponentName) noexcept;
  void SetInitialProps(folly::dynamic&& initialProps) noexcept;
  void AttachRoot() noexcept;
  void DetachRoot() noexcept;
  std::shared_ptr<::react::uwp::IXamlReactControl> GetXamlReactControl() const noexcept;

  // IReactRootView implementations
  virtual void ResetView() override;
  virtual std::string JSComponentName() const noexcept override;
  virtual int64_t GetActualHeight() const override;
  virtual int64_t GetActualWidth() const override;
  virtual int64_t GetTag() const override;
  virtual void SetTag(int64_t tag) override;

  // IXamlReactControl
  virtual void blur(XamlView const& xamlView) noexcept override;

private:
  int64_t m_tag;
  std::string m_jsComponentName;
  XamlView m_rootView;
};

} // namespace uwp
} // namespace react
