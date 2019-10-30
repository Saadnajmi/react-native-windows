// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include "ViewViewManager.h"

#include "ShadowNodeBase.h"
#include "PropertyUtils.h"
#include "NativeUIManager.h"

#include <winrt/Windows.System.h>

namespace react {
namespace uwp {

// ViewShadowNode

class ViewShadowNode : public ShadowNodeBase
{
  using Super = ShadowNodeBase;

public:
  ViewShadowNode() = default;

  void createView() override
  {
    Super::createView();

    auto panel = GetViewPanel();
  }

  bool IsControl()
  {
    return m_isControl;
  }
  void IsControl(bool isControl)
  {
    m_isControl = isControl;
  }

  bool EnableFocusRing()
  {
    return m_enableFocusRing;
  }
  void EnableFocusRing(bool enable)
  {
    m_enableFocusRing = enable;

    if (IsControl())
      GetControl().UseSystemFocusVisuals(m_enableFocusRing);
  }

  int32_t TabIndex()
  {
    return m_tabIndex;
  }
  void TabIndex(int32_t tabIndex)
  {
    m_tabIndex = tabIndex;

    if (IsControl())
      GetControl().TabIndex(m_tabIndex);
  }

  bool OnClick()
  {
    return m_onClick;
  }
  void OnClick(bool isSet)
  {
    m_onClick = isSet;
  }

  void AddView(ShadowNode& child, int64_t index) override
  {
    GetViewPanel().Children().InsertAt(
      static_cast<uint32_t>(index), static_cast<ShadowNodeBase&>(child).GetView().as<winrt::Windows::UI::Xaml::UIElement>());
  }

  void RemoveChildAt(int64_t indexToRemove) override
  {
    if (indexToRemove == static_cast<uint32_t>(indexToRemove))
      GetViewPanel().Children().RemoveAt(static_cast<uint32_t>(indexToRemove));
  }

  void removeAllChildren() override
  {
    GetViewPanel().Children().Clear();

    XamlView current = m_view;

    // TODO NOW: Why do we do this? Removal of children doesn't seem to imply we
    // tear down the infrastr
    if (IsControl())
    {
      auto control = m_view.as<winrt::Windows::UI::Xaml::Controls::ContentControl>();
      current = control.Content().as<XamlView>();
      control.Content(nullptr);
    }
  }

  void ReplaceChild(XamlView oldChildView, XamlView newChildView) override
  {
    auto pPanel = GetViewPanel();
    if (pPanel != nullptr)
    {
      uint32_t index;
      if (pPanel.Children().IndexOf(oldChildView.as<winrt::Windows::UI::Xaml::UIElement>(), index))
      {
        pPanel.Children().RemoveAt(index);
        pPanel.Children().InsertAt(index, newChildView.as<winrt::Windows::UI::Xaml::UIElement>());
      }
      else
      {
        assert(false);
      }
    }
  }

  void RefreshProperties()
  {
    // The view may have been replaced, so transfer properties stored on the
    // shadow node to the view
    EnableFocusRing(EnableFocusRing());
    TabIndex(TabIndex());
    static_cast<FrameworkElementViewManager*>(GetViewManager())->RefreshTransformMatrix(this);
  }

  winrt::Windows::UI::Xaml::Controls::RelativePanel GetViewPanel()
  {
    XamlView current = m_view;

    if (IsControl())
    {
      auto control = m_view.as<winrt::Windows::UI::Xaml::Controls::ContentControl>();
      current = control.Content().as<XamlView>();
    }

    auto panel = current.try_as<winrt::Windows::UI::Xaml::Controls::RelativePanel>();
    assert(panel != nullptr);

    return panel;
  }

  winrt::Windows::UI::Xaml::Controls::UserControl GetControl()
  {
    return IsControl() ? m_view.as<winrt::Windows::UI::Xaml::Controls::UserControl>() : nullptr;
  }

  XamlView CreateViewControl()
  {
    winrt::Windows::UI::Xaml::Controls::UserControl contentControl;

    m_contentControlGotFocusRevoker = contentControl.GotFocus(winrt::auto_revoke, [=](auto&&, auto&& args)
      {
        if (args.OriginalSource().try_as<winrt::Windows::UI::Xaml::UIElement>() == contentControl.as<winrt::Windows::UI::Xaml::UIElement>())
        {
          auto tag = m_tag;
          DispatchEvent("topFocus", std::move(folly::dynamic::object("target", tag)));
        }
      });

    m_contentControlLostFocusRevoker = contentControl.LostFocus(winrt::auto_revoke, [=](auto&&, auto&& args)
      {
        if (args.OriginalSource().try_as<winrt::Windows::UI::Xaml::UIElement>() == contentControl.as<winrt::Windows::UI::Xaml::UIElement>())
        {
          auto tag = m_tag;
          DispatchEvent("topBlur", std::move(folly::dynamic::object("target", tag)));
        }
      });

    return contentControl.try_as<XamlView>();
  }

  void DispatchEvent(std::string eventName, folly::dynamic&& eventData)
  {
    //auto instance = GetViewManager()->GetReactInstance().lock();
    //if (instance != nullptr)
    //  instance->DispatchEvent(m_tag, eventName, std::move(eventData));
  }

private:
  bool m_isControl = false;
  
  bool m_enableFocusRing = true;
  bool m_onClick = false;
  int32_t m_tabIndex = std::numeric_limits<std::int32_t>::max();

  winrt::Windows::UI::Xaml::Controls::ContentControl::GotFocus_revoker m_contentControlGotFocusRevoker {};
  winrt::Windows::UI::Xaml::Controls::ContentControl::LostFocus_revoker m_contentControlLostFocusRevoker {};
};


// ViewViewManager

ViewViewManager::ViewViewManager(const std::shared_ptr<react::uwp::NativeUIManager>& spNativeUIManager) : Super(spNativeUIManager)
{
}

const char* ViewViewManager::GetName() const
{
  return "RCTView";
}

folly::dynamic ViewViewManager::GetExportedCustomDirectEventTypeConstants() const
{
  auto directEvents = Super::GetExportedCustomDirectEventTypeConstants();
  directEvents["topClick"] = folly::dynamic::object("registrationName", "onClick");
  directEvents["topAccessibilityTap"] = folly::dynamic::object("registrationName", "onAccessibilityTap");

  return directEvents;
}

facebook::react::ShadowNode* ViewViewManager::createShadow() const
{
  return new ViewShadowNode();
}

XamlView ViewViewManager::CreateViewCore(int64_t tag)
{
  winrt::Windows::UI::Xaml::Controls::RelativePanel panel;
  panel.VerticalAlignment(winrt::Windows::UI::Xaml::VerticalAlignment::Stretch);
  panel.HorizontalAlignment(winrt::Windows::UI::Xaml::HorizontalAlignment::Stretch);

  return panel.as<XamlView>();
}

folly::dynamic ViewViewManager::GetNativeProps() const
{
  auto props = Super::GetNativeProps();

  props.update(folly::dynamic::object("pointerEvents", "string")("onClick", "function")("onMouseEnter", "function")(
    "onMouseLeave", "function")
    //("onMouseMove", "function")
    ("acceptsKeyboardFocus", "boolean")("enableFocusRing", "boolean")("tabIndex", "number"));

  return props;
}

void ViewViewManager::UpdateProperties(ShadowNodeBase* nodeToUpdate, const folly::dynamic& reactDiffMap)
{
  auto* pViewShadowNode = static_cast<ViewShadowNode*>(nodeToUpdate);
  bool shouldBeControl = pViewShadowNode->IsControl();
  bool finalizeBorderRadius { false };

  auto pPanel = pViewShadowNode->GetViewPanel();

  if (pPanel != nullptr)
  {
    for (const auto& pair : reactDiffMap.items())
    {
      const std::string& propertyName = pair.first.getString();
      const folly::dynamic& propertyValue = pair.second;

      if (TryUpdateBackgroundBrush(pPanel.as<winrt::Windows::UI::Xaml::Controls::Panel>(), propertyName, propertyValue))
      {
        continue;
      }
      else if (TryUpdateBorderProperties(nodeToUpdate, pPanel, propertyName, propertyValue))
      {
        continue;
      }
      else if (TryUpdateCornerRadiusOnNode(nodeToUpdate, propertyName, propertyValue))
      {
        finalizeBorderRadius = true;
        continue;
      }
      else if (TryUpdateMouseEvents(nodeToUpdate, propertyName, propertyValue))
      {
        continue;
      }
      else if (propertyName == "onClick")
      {
        pViewShadowNode->OnClick(!propertyValue.isNull() && propertyValue.asBool());
      }
      else if (propertyName == "pointerEvents")
      {
        if (propertyValue.isString())
        {
          bool hitTestable = propertyValue.getString() != "none";
          pPanel.IsHitTestVisible(hitTestable);
        }
      }
      else if (propertyName == "acceptsKeyboardFocus")
      {
        if (propertyValue.isBool())
          shouldBeControl = propertyValue.getBool();
      }
      else if (propertyName == "enableFocusRing")
      {
        if (propertyValue.isBool())
          pViewShadowNode->EnableFocusRing(propertyValue.getBool());
        else if (propertyValue.isNull())
          pViewShadowNode->EnableFocusRing(false);
      }
      else if (propertyName == "tabIndex")
      {
        if (propertyValue.isNumber())
        {
          auto tabIndex = propertyValue.asDouble();
          if (tabIndex == static_cast<int32_t>(tabIndex))
          {
            pViewShadowNode->TabIndex(static_cast<int32_t>(tabIndex));
          }
        }
        else if (propertyValue.isNull())
        {
          pViewShadowNode->TabIndex(-1);
        }
      }
    }
  }

  Super::UpdateProperties(nodeToUpdate, reactDiffMap);

  if (finalizeBorderRadius)
    UpdateCornerRadiusOnElement(nodeToUpdate, pPanel);
  
  TryUpdateView(pViewShadowNode, pPanel, shouldBeControl);
}

void ViewViewManager::TryUpdateView(
  ViewShadowNode* pViewShadowNode,
  winrt::Windows::UI::Xaml::Controls::RelativePanel& pPanel,
  bool useControl)
{
  bool isControl = pViewShadowNode->IsControl();

  // This short-circuits all of the update code when we have the same hierarchy
  if (isControl == useControl)
    return;

  //
  // 1. Ensure we have the new 'root' and do the child replacement
  //      This is first to ensure that we can re-parent the Border or ViewPanel
  //      we already have
  // 2. Transfer properties
  //      There are likely some complexities to handle here
  // 3. Do any sub=parenting
  //      This means Panel under Border and/or Border under Control
  //

  XamlView oldXamlView(pViewShadowNode->GetView());
  XamlView newXamlView(nullptr);

  //
  // 1. Determine new view & clean up any parent-child relationships
  //

  // If we need a Control then get existing reference or create it
  if (useControl)
  {
    newXamlView = pViewShadowNode->GetControl().try_as<XamlView>();
    if (newXamlView == nullptr)
    {
      newXamlView = pViewShadowNode->CreateViewControl();
    }
  }

  // Clean up child of Control if needed
  if (isControl && !useControl)
  {
    pViewShadowNode->GetControl().Content(nullptr);
  }

  // If don't need a control, then set Outer Border or the Panel as the view
  // root
  if (!useControl)
  {
    newXamlView = pPanel.try_as<XamlView>();
  }

  // ASSERT: One of the scenarios should be true, so we should have a root view
  assert(newXamlView != nullptr);

  //
  // 2. Transfer needed properties from old to new view
  //

  // Transfer properties from old XamlView to the new one
  TransferProperties(oldXamlView, newXamlView);

  // Update the meta-data in the shadow node
  pViewShadowNode->IsControl(useControl);

  //
  // 3. Setup any new parent-child relationships
  //

  // If we need to change the root of our view, do it now
  if (oldXamlView != newXamlView)
  {
    auto nativeUIManager = NativeUIManager();
    if (nativeUIManager == nullptr)
      return;

    // Inform the parent ShadowNode of this change so the hierarchy can be
    // updated
    int64_t parentTag = pViewShadowNode->GetParent();
    auto host = nativeUIManager->getHost();
    auto* pParentNode = static_cast<ShadowNodeBase*>(host->FindShadowNodeForTag(parentTag));
    if (pParentNode != nullptr)
      pParentNode->ReplaceChild(oldXamlView, newXamlView);

    // Update the ShadowNode with the new XamlView
    pViewShadowNode->ReplaceView(newXamlView);
    pViewShadowNode->RefreshProperties();

    // Inform the NativeUIManager of this change so the yoga layout can be
    // updated
    nativeUIManager->ReplaceView(*pViewShadowNode);
  }

  // Ensure parenting is setup properly
  auto visualRoot = pPanel.try_as<winrt::Windows::UI::Xaml::UIElement>();

  if (useControl)
    pViewShadowNode->GetControl().Content(visualRoot);
}

void ViewViewManager::SetLayoutProps(
  ShadowNodeBase& nodeToUpdate,
  XamlView viewToUpdate,
  float left,
  float top,
  float width,
  float height)
{
  // When the View has a ContentControl the ViewPanel must also have the Width &
  // Height set
  // Do this first so that it is setup properly before any events are fired by
  // the Super implementation
  auto* pViewShadowNode = static_cast<ViewShadowNode*>(&nodeToUpdate);
  if (pViewShadowNode->IsControl())
  {
    auto pPanel = pViewShadowNode->GetViewPanel();
    pPanel.Width(width);
    pPanel.Height(height);
  }

  Super::SetLayoutProps(nodeToUpdate, viewToUpdate, left, top, width, height);
}
} // namespace uwp
} // namespace react
