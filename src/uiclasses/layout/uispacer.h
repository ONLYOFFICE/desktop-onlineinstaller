#ifndef UISPACER_H
#define UISPACER_H

#include "uidefines.h"
#include "uicommon.h"

class DECL_VISUALUI UISpacer
{
public:
    explicit UISpacer(int w, int h, int hSizeBehavior = SizePolicy::Expanding, int vSizeBehavior = SizePolicy::Expanding);
    ~UISpacer();

    void setSizePolicy(SizePolicy::Properties property, int val);
    int sizePolicy(SizePolicy::Properties);

private:
    friend class UILayoutItem;
    int m_size_behaviors[SizePolicy::PROPERTIES_LAST];
    int x,
        y,
        width,
        height;
};

#endif // UISPACER_H
