#ifndef UIPALETTE_H
#define UIPALETTE_H

#ifdef _WIN32
# include <Windows.h>
#else
# include <cstdint>
  typedef unsigned char BYTE;
  typedef uint16_t WORD;
  typedef uint32_t DWORD;
  typedef DWORD COLORREF;
#endif
#include "uidefines.h"


class DECL_VISUALUI Palette
{
public:
    Palette();
    ~Palette();

    enum Role : BYTE {
        Background = 0,
        Border,
        Base,
        AlternateBase,
        Text,
        Primitive,
        AlternatePrimitive,
        PALETTE_ROLE_LAST
    };

    enum State : BYTE {
        Disabled = 0,
        Normal,
        Hover,
        Pressed,
        PALETTE_STATE_LAST
    };

    COLORREF color(Role) const noexcept;
    void setColor(Role, State, DWORD) noexcept;
    void setCurrentState(State) noexcept;

private:
    DWORD palette[PALETTE_ROLE_LAST][PALETTE_STATE_LAST];
    DWORD currentColors[PALETTE_ROLE_LAST];
    State currentState;
};

#endif // UIPALETTE_H
