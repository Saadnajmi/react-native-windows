#include "stdafx.h"
#include "ViewManagerBase.h"

#include "NativeUIManager.h"
#include "ShadowNodeBase.h"

namespace react::uwp {

ViewManagerBase::ViewManagerBase(const std::shared_ptr<react::uwp::NativeUIManager>& nativeUIManager)
  :m_wkNativeUIManager(nativeUIManager)
{
}

XamlView ViewManagerBase::CreateView(int64_t tag)
{
  XamlView view = CreateViewCore(tag);

  view.SetValue(winrt::Windows::UI::Xaml::FrameworkElement::TagProperty(), winrt::Windows::Foundation::PropertyValue::CreateInt64(tag));

  return view;
}

folly::dynamic ViewManagerBase::GetExportedViewConstants() const
{
  return folly::dynamic::object();
}

folly::dynamic ViewManagerBase::GetCommands() const
{
  return folly::dynamic::object();
}

folly::dynamic ViewManagerBase::GetNativeProps() const
{
  folly::dynamic props = folly::dynamic::object();
  props.update(folly::dynamic::object("onLayout", "function")("keyDownEvents", "array")("keyUpEvents", "array"));
  return props;
}

facebook::react::ShadowNode* ViewManagerBase::createShadow() const
{
  // This class is the default ShadowNode that most view managers can use. If
  // they need special functionality
  //  they should override this function and create their own ShadowNodeBase
  //  sub-class.
  return new ShadowNodeBase();
}

void ViewManagerBase::destroyShadow(facebook::react::ShadowNode* node) const
{
  delete node;
}

folly::dynamic ViewManagerBase::GetConstants() const
{
  folly::dynamic constants = folly::dynamic::object("Constants", GetExportedViewConstants())("Commands", GetCommands())(
    "NativeProps", GetNativeProps());

  const auto bubblingEventTypesConstants = GetExportedCustomBubblingEventTypeConstants();
  if (!bubblingEventTypesConstants.empty())
    constants["bubblingEventTypes"] = std::move(bubblingEventTypesConstants);
  const auto directEventTypesConstants = GetExportedCustomDirectEventTypeConstants();
  if (!directEventTypesConstants.empty())
    constants["directEventTypes"] = std::move(directEventTypesConstants);

  return constants;
}


folly::dynamic ViewManagerBase::GetExportedCustomBubblingEventTypeConstants() const
{
  const PCSTR standardEventBaseNames[] = {
    // Generic events
    "Press",
    "Change",
    "Focus",
    "Blur",
    "SubmitEditing",
    "EndEditing",
    "KeyPress",

    // Touch events
    "TouchStart",
    "TouchMove",
    "TouchCancel",
    "TouchEnd",

    // Keyboard events
    "KeyUp",
    "KeyDown",
  };

  folly::dynamic bubblingEvents = folly::dynamic::object();
  for (auto& standardEventBaseName : standardEventBaseNames)
  {
    using namespace std::string_literals;

    std::string eventName = "top"s + standardEventBaseName;
    std::string bubbleName = "on"s + standardEventBaseName;

    folly::dynamic registration = folly::dynamic::object();
    registration["captured"] = bubbleName + "Capture";
    registration["bubbled"] = std::move(bubbleName);

    bubblingEvents[std::move(eventName)] = folly::dynamic::object("phasedRegistrationNames", std::move(registration));
  }

  return bubblingEvents;
}

folly::dynamic ViewManagerBase::GetExportedCustomDirectEventTypeConstants() const
{
  folly::dynamic eventTypes = folly::dynamic::object();
  eventTypes.update(folly::dynamic::object("topLayout", folly::dynamic::object("registrationName", "onLayout"))(
    "topMouseEnter", folly::dynamic::object("registrationName", "onMouseEnter"))(
      "topMouseLeave", folly::dynamic::object("registrationName", "onMouseLeave"))(
        "topAccessibilityAction", folly::dynamic::object("registrationName", "onAccessibilityAction"))
    //    ("topMouseMove",
    //    folly::dynamic::object("registrationName",
    //    "onMouseMove"))
  );
  return eventTypes;
}


void ViewManagerBase::AddView(XamlView parent, XamlView child, int64_t index)
{
  // ASSERT: Child must either implement or not allow children.
  assert(false);
}

void ViewManagerBase::RemoveChildAt(XamlView parent, int64_t index)
{
  // ASSERT: Child must either implement or not allow children.
  assert(false);
}

void ViewManagerBase::RemoveAllChildren(XamlView parent)
{
}

void ViewManagerBase::ReplaceChild(XamlView parent, XamlView oldChild, XamlView newChild)
{
  // ASSERT: Child must either implement or not allow children.
  assert(false);
}

void ViewManagerBase::UpdateProperties(ShadowNodeBase* nodeToUpdate, const folly::dynamic& reactDiffMap)
{
  // Directly dirty this node since non-layout changes like the text property do
  // not trigger relayout
  //  There isn't actually a yoga node for RawText views, but it will invalidate
  //  the ancestors which
  //  will include the containing Text element. And that's what matters.
  int64_t tag = GetTag(nodeToUpdate->GetView());
  auto nativeUIManager = NativeUIManager();
  if (nativeUIManager != nullptr)
    nativeUIManager->DirtyYogaNode(tag);

  for (const auto& pair : reactDiffMap.items())
  {
    const std::string& propertyName = pair.first.getString();
    const folly::dynamic& propertyValue = pair.second;

    if (propertyName == "onLayout")
    {
      nodeToUpdate->m_onLayout = !propertyValue.isNull() && propertyValue.asBool();
    }
    else if (propertyName == "keyDownEvents")
    {
      nodeToUpdate->UpdateHandledKeyboardEvents(propertyName, propertyValue);
    }
    else if (propertyName == "keyUpEvents")
    {
      nodeToUpdate->UpdateHandledKeyboardEvents(propertyName, propertyValue);
    }
  }
}


void ViewManagerBase::DispatchCommand(XamlView viewToUpdate, int64_t commandId, const folly::dynamic& commandArgs)
{
  assert(false); // View did not handle its command
}

void ViewManagerBase::SetLayoutProps(
  ShadowNodeBase& nodeToUpdate,
  XamlView viewToUpdate,
  float left,
  float top,
  float width,
  float height)
{
  auto element = viewToUpdate.as<winrt::Windows::UI::Xaml::UIElement>();
  if (element == nullptr)
  {
    // TODO: Assert
    return;
  }
  auto fe = element.as<winrt::Windows::UI::Xaml::FrameworkElement>();

  fe.Width(width);
  fe.Height(height);

  // Fire Events
  if (nodeToUpdate.m_onLayout)
  {
    int64_t tag = GetTag(viewToUpdate);
    folly::dynamic layout = folly::dynamic::object("x", left)("y", top)("height", height)("width", width);

    folly::dynamic eventData = folly::dynamic::object("target", tag)("layout", std::move(layout));
  }
}

void ViewManagerBase::TransferProperties(XamlView /*oldView*/, XamlView /*newView*/)
{
}

YGMeasureFunc ViewManagerBase::GetYogaCustomMeasureFunc() const
{
  return nullptr;
}

bool ViewManagerBase::RequiresYogaNode() const
{
  return true;
}

bool ViewManagerBase::IsNativeControlWithSelfLayout() const
{
  return GetYogaCustomMeasureFunc() != nullptr;
}

}
