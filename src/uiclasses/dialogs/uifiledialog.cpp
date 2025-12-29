#include "uifiledialog.h"
#include "uiwidget.h"
#ifdef _WIN32
# include <Windows.h>
# include <Shlobj.h>
# include <commdlg.h>
# include <algorithm>
#else
# include <string.h>
# include <gtk/gtk.h>
#endif

#ifdef _WIN32
static PIDLIST_ABSOLUTE pidlFromPath(const wchar_t *path)
{
    PIDLIST_ABSOLUTE pidl = nullptr;
    SFGAOF sfgao;
    HRESULT hr = SHParseDisplayName(path, nullptr, &pidl, 0, &sfgao);
    return SUCCEEDED(hr) ? pidl : nullptr;
}

static std::vector<tstring> parseNullSeparatedList(PCZZWSTR fileList)
{
    std::vector<tstring> result;
    if (!fileList || *fileList == L'\0')
        return result;
    const wchar_t* it = fileList;
    std::wstring directory(it);
    it += directory.length() + 1;
    if (*it == L'\0') {
        result.push_back(directory);
    } else {
        while (*it != L'\0') {
            result.push_back(directory + L"\\" + it);
            it += wcslen(it) + 1;
        }
    }
    return result;
}

static std::wstring convertFilter(const std::wstring &input)
{
    std::wstring result;
    result.reserve(input.size() * 2);
    size_t start = 0;
    while (start < input.size()) {
        size_t end = input.find(L";;", start);
        if (end == std::wstring::npos)
            end = input.size();

        std::wstring part = input.substr(start, end - start);
        result.append(part);
        result.push_back(L'\0');

        size_t open = part.find(L'(');
        size_t close = part.find(L')');
        if (open != std::wstring::npos && close != std::wstring::npos && open < close) {
            result.append(part.substr(open + 1, close - open - 1));
        }
        result.push_back(L'\0');
        start = end + (end < input.size() ? 2 : 0);
    }
    result.push_back(L'\0');
    return result;
}

static DWORD filterIndexFromText(const std::wstring &filters, const std::wstring *target)
{
    if (!target)
        return 1;
    size_t start = 0;
    DWORD index = 1;
    while (start < filters.size()) {
        size_t end = filters.find(L";;", start);
        if (end == std::wstring::npos)
            end = filters.size();

        std::wstring part = filters.substr(start, end - start);
        if (part == *target)
            return index;

        ++index;
        start = end + 2;
    }
    return 1;
}

static std::wstring filterTextByIndex(const std::wstring &input, DWORD index)
{
    size_t start = 0;
    DWORD current = 1;
    while (start < input.size()) {
        size_t end = input.find(L";;", start);
        if (end == std::wstring::npos)
            end = input.size();

        if (current == index) {
            return input.substr(start, end - start);
        }

        start = end + 2;
        ++current;
    }
    return L"";
}
#else
static GSList* splitString(const char* str, const char* delim)
{
    GSList *list = nullptr;
    if (!str || *str == '\0')
        return nullptr;
    char *tmp = strdup(str);
    for (char* token = strtok(tmp, delim); token; token = strtok(nullptr, delim))
        list = g_slist_append(list, strdup(token));
    free(tmp);
    return list;
}
#endif

class UIFileDialog::UIFileDialogPrivate
{
public:
    UIFileDialogPrivate(UIWidget *parent) :
        parent(parent)
    {

    }

    ~UIFileDialogPrivate()
    {

    }

#ifdef _WIN32
    tstring selectFolder() const
    {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        WCHAR szDir[MAX_PATH];
        BROWSEINFO bInfo = {0};
        bInfo.hwndOwner = parent ? parent->platformWindow() : nullptr;
        bInfo.pidlRoot = pidlFromPath(dir.c_str());
        bInfo.pszDisplayName = szDir;
        bInfo.lpszTitle = title.c_str();
        bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bInfo.lpfn = NULL;
        bInfo.lParam = 0;
        bInfo.iImage = -1;

        LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
        if (lpItem && SHGetPathFromIDList(lpItem, szDir)) {
            CoUninitialize();
            return szDir;
        }
        CoUninitialize();
        return {};
    }
#endif

    std::vector<tstring> selectFile()
    {
#ifdef _WIN32
        TCHAR fileName[32767] = {};
        swprintf_s(fileName, _countof(fileName) , L"%s", file.c_str());
        tstring native_filter = convertFilter(filter);
        OPENFILENAME ofn = {};
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = parent ? parent->platformWindow() : nullptr;
        ofn.lpstrTitle = title.c_str();
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = _countof(fileName);
        ofn.lpstrInitialDir = dir.c_str();
        ofn.lpstrFilter = native_filter.c_str();
        ofn.lpstrDefExt = def_ext.c_str();
        ofn.nFilterIndex = filterIndexFromText(filter, def_filter);
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST;

        BOOL res = FALSE;
        switch (mode) {
        case Mode::OpenFiles:
            ofn.Flags |= OFN_ALLOWMULTISELECT;
        case Mode::OpenFile: {
            ofn.Flags |= OFN_FILEMUSTEXIST;
            res = GetOpenFileName(&ofn);
            break;
        }
        case Mode::SaveFile: {
            res = GetSaveFileName(&ofn);
            break;
        }
        default:
            break;
        }
        if (res) {
            std::vector<tstring> paths;
            if (mode == Mode::OpenFiles) {
                paths = parseNullSeparatedList(ofn.lpstrFile);
            } else {
                paths.emplace_back(ofn.lpstrFile);
            }
            if (def_filter) {
                def_filter->assign(filterTextByIndex(filter, ofn.nFilterIndex));
            }
            return paths;
        }
        return {};
#else
        GtkFileChooserAction action = (mode == Mode::OpenFile || mode == Mode::OpenFiles) ? GTK_FILE_CHOOSER_ACTION_OPEN :
                                          mode == Mode::SaveFile ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        GtkWidget *dlg = nullptr;
        dlg = gtk_file_chooser_dialog_new(title.c_str(), parent ? GTK_WINDOW(parent->platformWindow()) : nullptr,
                                          action, g_dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                                          mode == Mode::SaveFile ? g_dgettext("gtk30", "_Save") : g_dgettext("gtk30", "_Open"),
                                          GTK_RESPONSE_ACCEPT, NULL);
        GtkFileChooser *fc = GTK_FILE_CHOOSER(dlg);
        gtk_file_chooser_set_current_folder(fc, dir.c_str());
        if (mode == Mode::SaveFile) {
            gtk_file_chooser_set_current_name(fc, file.c_str());
            gtk_file_chooser_set_do_overwrite_confirmation(fc, TRUE);
        } else {
            gtk_file_chooser_set_select_multiple(fc, mode == Mode::OpenFiles);
        }
        if (mode != Mode::OpenFolder && !filter.empty()) {
            GSList *lst = splitString(filter.c_str(), ";;");
            for (GSList *node = lst; node; node = node->next) {
                const char *name = (const char*)node->data;
                if (!name)
                    continue;

                GtkFileFilter *flt = gtk_file_filter_new();
                gtk_file_filter_set_name(flt, name);

                const char *lbr = strchr(name, '(');
                const char *rbr = strchr(name, ')');
                if (lbr && rbr && lbr < rbr) {
                    std::string inside(lbr + 1, rbr);

                    GSList *pattern_lst = splitString(inside.c_str(), " ");
                    for (GSList *p = pattern_lst; p; p = p->next) {
                        if (const char *pattern = (const char*)p->data)
                            gtk_file_filter_add_pattern(flt, pattern);
                    }
                    g_slist_free_full(pattern_lst, free);
                }
                gtk_file_chooser_add_filter(fc, flt);

                if (def_filter && strcmp(name, def_filter->c_str()) == 0)
                    gtk_file_chooser_set_filter(fc, flt);
            }
            g_slist_free_full(lst, free);
        }

        gint result = gtk_dialog_run(GTK_DIALOG(dlg));
        std::vector<tstring> paths;
        if (result == GTK_RESPONSE_ACCEPT) {
            if (mode == Mode::OpenFiles) {
                GSList *files_lst = gtk_file_chooser_get_filenames(fc);
                for (GSList *f = files_lst; f; f = f->next)
                    paths.emplace_back((const char*)f->data);
                g_slist_free_full(files_lst, free);
            } else {
                paths.emplace_back((const char*)gtk_file_chooser_get_filename(fc));
            }
            if (mode != Mode::OpenFolder) {
                GtkFileFilter *s_filter = gtk_file_chooser_get_filter(fc);
                if (s_filter && def_filter)
                    def_filter->assign(gtk_file_filter_get_name(s_filter));
            }
        }
        gtk_widget_destroy(dlg);
        return paths;
#endif
    }

    UIWidget *parent = nullptr;
    Mode mode = Mode::OpenFile;
    tstring *def_filter = nullptr;
    tstring title,
            file,
            dir,
            def_ext,
            filter;
};

UIFileDialog::UIFileDialog(UIWidget *parent, const tstring &title) :
    pimpl(new UIFileDialogPrivate(parent))
{
    pimpl->title = title;
}

UIFileDialog::~UIFileDialog()
{
    delete pimpl, pimpl = nullptr;
}

void UIFileDialog::setTitle(const tstring &title)
{
    pimpl->title = title;
}

void UIFileDialog::setMode(Mode mode)
{
    pimpl->mode = mode;
}

void UIFileDialog::setFileName(const tstring &file)
{
    pimpl->file = file;
}

void UIFileDialog::setDirectory(const tstring &dir)
{
    pimpl->dir = dir;
#ifdef _WIN32
    std::replace(pimpl->dir.begin(), pimpl->dir.end(), L'/', L'\\');
#endif
}

void UIFileDialog::setDefaultExt(const tstring &ext)
{
    pimpl->def_ext = ext;
}

void UIFileDialog::setFilter(const tstring &filter)
{
    pimpl->filter = filter;
}

void UIFileDialog::setDefaultFilter(tstring *def_filter)
{
    pimpl->def_filter = def_filter;
}

std::vector<tstring> UIFileDialog::exec()
{
    std::vector<tstring> paths;
#ifdef _WIN32
    switch (pimpl->mode) {
    case Mode::OpenFolder: {
        tstring folder = pimpl->selectFolder();
        if (!folder.empty()) {
            size_t last = folder.length() - 1;
            if (folder[last] == L'\\')
                folder[last] = L'\0';
            paths.push_back(std::move(folder));
        }
        break;
    }
    case Mode::OpenFile:
    case Mode::OpenFiles:
    case Mode::SaveFile: {
#endif
        std::vector<tstring> files = pimpl->selectFile();
        if (!files.empty()) {
            paths = std::move(files);
        }
#ifdef _WIN32
        break;
    }
    default:
        break;
    }
#endif
    return paths;
}
