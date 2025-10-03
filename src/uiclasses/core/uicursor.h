#ifndef UICURSOR_H
#define UICURSOR_H

#include "uidefines.h"


class DECL_VISUALUI UICursor
{
public:
    UICursor();
    ~UICursor();

    static void globalPos(int &x, int &y);
};

#endif // UICURSOR_H
