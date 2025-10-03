#include "uistyle.h"
#include "uixmldocument.h"
#include "uiutils.h"
#include "uiwidget.h"
#include "uipalette.h"
#include "uimetrics.h"
#include <list>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
# define tprintf wprintf
#else
# define tprintf printf
#endif

typedef std::unordered_map<tstring, DWORD> ThemeColors;

struct WidgetColorEntry
{
    Palette::Role role;
    Palette::State state;
    tstring value;
};

struct WidgetStyle
{
    std::vector<WidgetColorEntry> colors;
    std::vector<std::pair<Metrics::Role, int>> metrics;
    std::vector<std::pair<SizePolicy::Properties, SizePolicy::SizeBehavior>> properties;
    Size baseSize{-1,-1};
};

static void toLowerCase(tstring &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
}

class UIStyle::UIStylePrivate
{
public:
    void setMetrics(UIWidget *w) const
    {
        tstring id = w->objectGroupId();
        if (!id.empty()) {
            auto it = styles.find(id);
            if (it != styles.end()) {
                const WidgetStyle& ws = it->second;
                for (const auto &prop : ws.properties) {
                    w->setSizePolicy(prop.first, prop.second);
                }
                for (const auto &mt : ws.metrics) {
                    w->metrics()->setMetrics(mt.first, mt.second);
                }
                if (ws.baseSize.width != -1 && ws.baseSize.height != -1) {
                    w->setBaseSize(ws.baseSize.width, ws.baseSize.height);
                }
            }
        }
    }

    void setPalette(UIWidget *w) const
    {
        tstring id = w->objectGroupId();
        if (!id.empty()) {
            auto it = styles.find(id);
            if (it != styles.end()) {
                const WidgetStyle& ws = it->second;
                for (const auto &color_entry : ws.colors) {
                    int color = -1;
                    if (themeColors) {
                        auto it_color = themeColors->find(color_entry.value);
                        if (it_color != themeColors->end()) {
                            color = it_color->second;
                        }
                    }
                    if (color == -1) {
                        const tstring &color_str = color_entry.value;
                        try {
                            color = std::stoi((!color_str.empty() && color_str[0] == _T('#')) ? color_str.substr(1) : color_str, nullptr, 16);
                        } catch (const std::exception& e) {
                            tprintf(_T("[STYLE ERROR] Color not valid: %s\n"), color_entry.value.c_str());
                            fflush(stdout);
                        }
                    }
                    if (color > -1) {
                        w->palette()->setColor(color_entry.role, color_entry.state, color);
                    }
                }
            }
        }
    }

    void parseThemes(const XmlDocument &reader)
    {
        XmlNode root = reader.root();
        if (!root.isValid())
            return;
        tstring tag = root.getTagName();
        toLowerCase(tag);
        if (tag != _T("themes"))
            return;
        auto themes_ent = root.getChildren();
        for (const XmlNode &theme : themes_ent) {
            tag = theme.getTagName();
            toLowerCase(tag);
            if (tag != _T("theme"))
                continue;
            tstring id = theme.getAttributeValue(_T("id"));
            if (id.empty())
                continue;
            auto colors_arr = theme.getChildren();
            for (const XmlNode &colors : colors_arr) {
                tag = colors.getTagName();
                toLowerCase(tag);
                if (tag == _T("colors")) {
                    ThemeColors colors_map;
                    auto attrs = colors.getAttributes();
                    for (const auto &attr : attrs) {
                        if (!attr.first.empty() && !attr.second.empty()) {
                            int color = -1;
                            const tstring &color_str = attr.second;
                            try {
                                color = std::stoi((!color_str.empty() && color_str[0] == _T('#')) ? color_str.substr(1) : color_str, nullptr, 16);
                            } catch (const std::exception& e) {
                                printf("[STYLE ERROR] Color not valid: %s\n", e.what());
                                fflush(stdout);
                            }
                            if (color > -1) {
                                colors_map[attr.first] = color;
                            }
                        }
                    }
                    if (!colors_map.empty())
                        themes[id] = colors_map;
                }
            }
        }
    }

    void parseStyles(const XmlDocument &reader)
    {
        XmlNode root = reader.root();
        if (!root.isValid())
            return;
        tstring tag = root.getTagName();
        toLowerCase(tag);
        if (tag != _T("styles"))
            return;
        auto widgets = root.getChildren();
        for (const XmlNode &widget : widgets) {
            tag = widget.getTagName();
            toLowerCase(tag);
            if (tag != _T("widget"))
                continue;
            tstring id = widget.getAttributeValue(_T("id"));
            if (id.empty())
                continue;
            WidgetStyle widgetStyle;
            auto style_attrs = widget.getChildren();
            for (const XmlNode &style_attr : style_attrs) {
                tag = style_attr.getTagName();
                toLowerCase(tag);
                if (tag == _T("color")) {
                    Palette::Role role = Palette::PALETTE_ROLE_LAST;
                    tstring normal_val, hover_val, pressed_val, disabled_val;
                    auto attrs = style_attr.getAttributes();
                    for (const auto &attr : attrs) {
                        auto key = attr.first;
                        toLowerCase(key);
                        const auto &val = attr.second;
                        if (key == _T("background")) {
                            role = Palette::Background;
                            normal_val = val;
                        } else
                        if (key == _T("border")) {
                            role = Palette::Border;
                            normal_val = val;
                        } else
                        if (key == _T("base")) {
                            role = Palette::Base;
                            normal_val = val;
                        } else
                        if (key == _T("alternatebase")) {
                            role = Palette::AlternateBase;
                            normal_val = val;
                        } else
                        if (key == _T("text")) {
                            role = Palette::Text;
                            normal_val = val;
                        } else
                        if (key == _T("primitive")) {
                            role = Palette::Primitive;
                            normal_val = val;
                        } else
                        if (key == _T("alternateprimitive")) {
                            role = Palette::AlternatePrimitive;
                            normal_val = val;
                        } else
                        if (key == _T("hover")) {
                            hover_val = val;
                        } else
                        if (key == _T("pressed")) {
                            pressed_val = val;
                        } else
                        if (key == _T("disabled")) {
                            disabled_val = val;
                        }
                    }
                    if (role != Palette::PALETTE_ROLE_LAST && !normal_val.empty()) {
                        widgetStyle.colors.push_back({role, Palette::Normal, normal_val});
                        if (!hover_val.empty()) {
                            widgetStyle.colors.push_back({role, Palette::Hover, hover_val});
                        }
                        if (!pressed_val.empty()) {
                            widgetStyle.colors.push_back({role, Palette::Pressed, pressed_val});
                        }
                        if (!disabled_val.empty()) {
                            widgetStyle.colors.push_back({role, Palette::Disabled, disabled_val});
                        }
                    } else {
                        tprintf(_T("[STYLE ERROR] Cannot parse color for: %s\n"), id.c_str());
                        fflush(stdout);
                    }
                } else
                if (tag == _T("basesize")) {
                    int width = -1, height = -1;
                    auto attrs = style_attr.getAttributes();
                    for (const auto &attr : attrs) {
                        auto key = attr.first;
                        toLowerCase(key);
                        if (key == _T("width")) {
                            try {
                                width = std::stoi(attr.second);
                            } catch (const std::exception& e) {
                                printf("[STYLE ERROR] Width not valid: %s\n", e.what());
                                fflush(stdout);
                            }
                        } else
                        if (key == _T("height")) {
                            try {
                                height = std::stoi(attr.second);
                            } catch (const std::exception& e) {
                                printf("[STYLE ERROR] Height not valid: %s\n", e.what());
                                fflush(stdout);
                            }
                        }
                    }
                    widgetStyle.baseSize = Size(width, height);
                } else
                if (tag == _T("sizepolicy")) {
                    auto attrs = style_attr.getAttributes();
                    for (const auto &attr : attrs) {
                        auto key = attr.first;
                        toLowerCase(key);
                        auto val = attr.second;
                        toLowerCase(val);
                        if (key == _T("hsizebehavior")) {
                            if (val == _T("fixed")) {
                                widgetStyle.properties.emplace_back(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
                            } else
                            if (val == _T("expanding")) {
                                widgetStyle.properties.emplace_back(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
                            }
                        } else
                        if (key == _T("vsizebehavior")) {
                            if (val == _T("fixed")) {
                                widgetStyle.properties.emplace_back(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
                            } else
                            if (val == _T("expanding")) {
                                widgetStyle.properties.emplace_back(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
                            }
                        }
                    }
                } else
                if (tag == _T("metrics")) {
                    auto attrs = style_attr.getAttributes();
                    for (const auto &attr : attrs) {
                        auto key = attr.first;
                        toLowerCase(key);
                        Metrics::Role role = Metrics::METRICS_LAST;
                        if (key == _T("borderwidth")) {
                            role = Metrics::BorderWidth;
                        } else
                        if (key == _T("borderradius")) {
                            role = Metrics::BorderRadius;
                        } else
                        if (key == _T("iconwidth")) {
                            role = Metrics::IconWidth;
                        } else
                        if (key == _T("iconheight")) {
                            role = Metrics::IconHeight;
                        } else
                        if (key == _T("iconmarginleft")) {
                            role = Metrics::IconMarginLeft;
                        } else
                        if (key == _T("iconmargintop")) {
                            role = Metrics::IconMarginTop;
                        } else
                        if (key == _T("iconmarginright")) {
                            role = Metrics::IconMarginRight;
                        } else
                        if (key == _T("iconmarginbottom")) {
                            role = Metrics::IconMarginBottom;
                        } else
                        if (key == _T("iconalignment")) {
                            role = Metrics::IconAlignment;
                        } else
                        if (key == _T("primitivewidth")) {
                            role = Metrics::PrimitiveWidth;
                        } else
                        if (key == _T("alternateprimitivewidth")) {
                            role = Metrics::AlternatePrimitiveWidth;
                        } else
                        if (key == _T("primitiveradius")) {
                            role = Metrics::PrimitiveRadius;
                        } else
                        if (key == _T("shadowwidth")) {
                            role = Metrics::ShadowWidth;
                        } else
                        if (key == _T("shadowradius")) {
                            role = Metrics::ShadowRadius;
                        } else
                        if (key == _T("textmarginleft")) {
                            role = Metrics::TextMarginLeft;
                        } else
                        if (key == _T("textmargintop")) {
                            role = Metrics::TextMarginTop;
                        } else
                        if (key == _T("textmarginright")) {
                            role = Metrics::TextMarginRight;
                        } else
                        if (key == _T("textmarginbottom")) {
                            role = Metrics::TextMarginBottom;
                        } else
                        if (key == _T("textalignment")) {
                            role = Metrics::TextAlignment;
                        }
                        if (role != Metrics::METRICS_LAST) {
                            auto val = attr.second;
                            int value = -1;
                            try {
                                value = std::stoi(val);
                            } catch (const std::exception& e) {
                                printf("[STYLE INFO] The metric uses a non-numeric value. This is expected. %s\n", e.what());
                                fflush(stdout);
                            }
                            if (value == -1 && (role == Metrics::TextAlignment || role == Metrics::IconAlignment)) {
                                toLowerCase(val);
                                int align = Metrics::AlignNone;
                                if (val.find(_T("alignhleft")) != std::string::npos) {
                                    align |= Metrics::AlignHLeft;
                                }
                                if (val.find(_T("alignhcenter")) != std::string::npos) {
                                    align |= Metrics::AlignHCenter;
                                }
                                if (val.find(_T("alignhright")) != std::string::npos) {
                                    align |= Metrics::AlignHRight;
                                }
                                if (val.find(_T("alignvtop")) != std::string::npos) {
                                    align |= Metrics::AlignVTop;
                                }
                                if (val.find(_T("alignvcenter")) != std::string::npos) {
                                    align |= Metrics::AlignVCenter;
                                }
                                if (val.find(_T("alignvbottom")) != std::string::npos) {
                                    align |= Metrics::AlignVBottom;
                                }
                                if (val.find(_T("aligncenter")) != std::string::npos) {
                                    align |= Metrics::AlignCenter;
                                }
                                if (align != Metrics::AlignNone)
                                    value = align;
                            }
                            if (value != -1) {
                                widgetStyle.metrics.emplace_back(role, value);
                            } else {
                                tprintf(_T("[STYLE ERROR] Wrong metric value: %s\n"), val.c_str());
                                fflush(stdout);
                            }
                        }
                    }
                }
            }
            styles[id] = widgetStyle;
        }
    }

    void updateThemeColorsPtr()
    {
        auto it = themes.find(themeId);
        themeColors = (it != themes.end()) ? &it->second : nullptr;
    }

    tstring themeId;
    ThemeColors *themeColors = nullptr;
    std::unordered_map<tstring, ThemeColors> themes;
    std::unordered_map<tstring, WidgetStyle> styles;
    std::list<UIWidget*> widgets;
};

UIStyle::UIStyle() :
    pimpl(new UIStylePrivate)
{}

UIStyle::~UIStyle()
{
    delete pimpl, pimpl = nullptr;
}

UIStyle& UIStyle::instance()
{
    static UIStyle inst;
    return inst;
}

void UIStyle::registerWidget(UIWidget *w)
{
    pimpl->widgets.push_back(w);
}

void UIStyle::unregisterWidget(UIWidget *w)
{
    pimpl->widgets.remove(w);
}

#ifdef _WIN32
void UIStyle::loadThemesFromResource(int id)
{
    tstring xml;
    UIUtils::loadStringResource(xml, id);
    if (xml.empty())
        return;
    XmlDocument reader;
    if (!reader.loadFromXml(xml))
        return;
    pimpl->parseThemes(reader);
    if (!pimpl->themeId.empty())
        pimpl->updateThemeColorsPtr();
}

void UIStyle::loadStylesFromResource(int id)
{
    tstring xml;
    UIUtils::loadStringResource(xml, id);
    if (xml.empty())
        return;
    XmlDocument reader;
    if (!reader.loadFromXml(xml))
        return;
    pimpl->parseStyles(reader);
}
#else
void UIStyle::loadThemesFromResource(GResource *res, const tstring &id)
{
    if (res) {
        tstring xml;
        UIUtils::loadStringResource(xml, res, id.c_str());
        if (xml.empty())
            return;
        XmlDocument reader;
        if (!reader.loadFromXml(xml))
            return;
        pimpl->parseThemes(reader);
        if (!pimpl->themeId.empty())
            pimpl->updateThemeColorsPtr();
    }
}

void UIStyle::loadStylesFromResource(GResource *res, const tstring &id)
{
    if (res) {
        tstring xml;
        UIUtils::loadStringResource(xml, res, id.c_str());
        if (xml.empty())
            return;
        XmlDocument reader;
        if (!reader.loadFromXml(xml))
            return;
        pimpl->parseStyles(reader);
    }
}
#endif

void UIStyle::loadThemesFromFile(const tstring &filePath)
{
    if (filePath.empty())
        return;
    XmlDocument reader;
    if (!reader.loadFromFile(filePath))
        return;
    pimpl->parseThemes(reader);
    if (!pimpl->themeId.empty())
        pimpl->updateThemeColorsPtr();
}

void UIStyle::loadStylesFromFile(const tstring &filePath)
{
    if (filePath.empty())
        return;
    XmlDocument reader;
    if (!reader.loadFromFile(filePath))
        return;
    pimpl->parseStyles(reader);
}

void UIStyle::setDefaultTheme(const tstring &theme)
{
    pimpl->themeId = theme;
    pimpl->updateThemeColorsPtr();
}

void UIStyle::setTheme(const tstring &theme)
{
    pimpl->themeId = theme;
    pimpl->updateThemeColorsPtr();
    for (UIWidget *w : pimpl->widgets) {
        pimpl->setPalette(w);
        w->update();
    }
}

void UIStyle::setStyle(UIWidget *w) const
{
    pimpl->setMetrics(w);
    pimpl->setPalette(w);
}

DWORD UIStyle::themeColor(const tstring &tag) const
{
    DWORD color = 0;
    if (pimpl->themeColors) {
        auto it_color = pimpl->themeColors->find(tag);
        if (it_color != pimpl->themeColors->end()) {
            color = it_color->second;
        }
    }
    return color;
}
