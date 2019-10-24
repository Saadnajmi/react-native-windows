// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include "ShadowNodeBase.h"

#include "NativeUIManager.h"
#include <ViewManager.h>
#include "ViewManagerBase.h"
#include <Windows.UI.Composition.h>
#include <Windows.UI.Xaml.h>
#include <WindowsNumerics.h>

namespace WFN = winrt::Windows::Foundation::Numerics;
namespace WUC = winrt::Windows::UI::Composition;
namespace WUX = winrt::Windows::UI::Xaml;

namespace react::uwp
{

ShadowNodeBase::ShadowNodeBase() : m_view(nullptr)
{
}

ViewManagerBase* ShadowNodeBase::GetViewManager() const
{
  return static_cast<ViewManagerBase*>(m_viewManager);
}

void ShadowNodeBase::updateProperties(const folly::dynamic&& props)
{
  GetViewManager()->UpdateProperties(this, props);
}

void ShadowNodeBase::createView()
{
  m_view = GetViewManager()->CreateView(this->m_tag);
}

bool ShadowNodeBase::NeedsForceLayout()
{
  return false;
}

void ShadowNodeBase::dispatchCommand(
  int64_t commandId,
  const folly::dynamic& commandArgs)
{
  GetViewManager()->DispatchCommand(GetView(), commandId, commandArgs);
}

void ShadowNodeBase::removeAllChildren()
{
  GetViewManager()->RemoveAllChildren(GetView());
}

void ShadowNodeBase::AddView(ShadowNode& child, int64_t index)
{
  this->GetViewManager()->AddView(
    GetView(), static_cast<ShadowNodeBase&>(child).GetView(), index);
}

void ShadowNodeBase::RemoveChildAt(int64_t indexToRemove)
{
  GetViewManager()->RemoveChildAt(GetView(), indexToRemove);
}

void ShadowNodeBase::onDropViewInstance()
{
}

void ShadowNodeBase::ReplaceView(XamlView view)
{
  SetTag(view, GetTag(m_view));

  m_view = view;
}

void ShadowNodeBase::ReplaceChild(
  XamlView oldChildView,
  XamlView newChildView)
{
  GetViewManager()->ReplaceChild(m_view, oldChildView, newChildView);
}

void ShadowNodeBase::ReparentView(XamlView view)
{
  GetViewManager()->TransferProperties(m_view, view);
  if (const auto nativeUIManager = GetViewManager()->NativeUIManager())
  {
    int64_t parentTag = GetParent();
    auto host = nativeUIManager->getHost();
    auto pParentNode =
      static_cast<ShadowNodeBase*>(host->FindShadowNodeForTag(parentTag));
    if (pParentNode != nullptr)
    {
      pParentNode->ReplaceChild(m_view, view);
    }
  }
  ReplaceView(view);
}

WUC::CompositionPropertySet ShadowNodeBase::EnsureTransformPS()
{
  if (m_transformPS == nullptr)
  {
    m_transformPS = WUX::Window::Current().Compositor().CreatePropertySet();
    UpdateTransformPS();
  }

  return m_transformPS;
}

// Create a PropertySet that will hold two properties:
// "center":  This is the center of the UIElement
// "transform": This will hold the un-centered TransformMatrix we want to apply
void ShadowNodeBase::UpdateTransformPS()
{
  if (m_transformPS != nullptr)
  {
    // First build up an ExpressionAnimation to compute the "center" property,
    // like so: The input to the expression is UIElement.ActualSize/2, output is
    // a vector3 with [cx, cy, 0].
    auto uielement = m_view.try_as<WUX::UIElement>();
    assert(uielement != nullptr);
    m_transformPS = EnsureTransformPS();
    m_transformPS.InsertVector3(L"center", { 0, 0, 0 });

    // Now insert the "transform" property with an initial value of identity.
    // The caller will handle populating this with the appropriate value (either
    // a static or animated value).
    WFN::float4x4 unused;
    // Take care not to stomp over any transform value we currently have set, as
    // we will use this value in the scenario where a View changed its backing
    // XAML element, here we will just transfer existing value to a new backing
    // XAML element.
    if (m_transformPS.TryGetMatrix4x4(L"transform", unused) ==
      WUC::CompositionGetValueStatus::NotFound)
    {
      m_transformPS.InsertMatrix4x4(
        L"transform",
        WFN::float4x4::identity());
    }
  }
}

}
