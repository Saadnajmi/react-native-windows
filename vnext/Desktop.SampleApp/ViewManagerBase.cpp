#include "stdafx.h"
#include "ViewManagerBase.h"


namespace react::uwp {

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
