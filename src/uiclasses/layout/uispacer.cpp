#include "uispacer.h"


UISpacer::UISpacer(int w, int h, int hSizeBehavior, int vSizeBehavior) :
    x(0),
    y(0),
    width(w),
    height(h)
{
    m_size_behaviors[SizePolicy::HSizeBehavior] = hSizeBehavior;
    m_size_behaviors[SizePolicy::VSizeBehavior] = vSizeBehavior;
}

UISpacer::~UISpacer()
{

}

int UISpacer::sizePolicy(SizePolicy::Properties property)
{
    return m_size_behaviors[property];
}

void UISpacer::setSizePolicy(SizePolicy::Properties property, int val)
{
    m_size_behaviors[property] = val;
}
