#ifndef UIFILEDIALOG_H
#define UIFILEDIALOG_H

#include "uidefines.h"
#include <vector>


class UIWidget;
class DECL_VISUALUI UIFileDialog
{
public:
    explicit UIFileDialog(UIWidget *parent = nullptr, const tstring &title = {});
    ~UIFileDialog();

    enum class Mode { OpenFolder, OpenFile, OpenFiles, SaveFile };

    void setTitle(const tstring &title);
    void setMode(Mode mode);
    void setFileName(const tstring &file);
    void setDirectory(const tstring &dir);
    void setDefaultExt(const tstring &ext);
    void setFilter(const tstring &filter);
    void setDefaultFilter(tstring *def_filter = nullptr);
    std::vector<tstring> exec();

private:
    class UIFileDialogPrivate;
    UIFileDialogPrivate *pimpl;
};

#endif // UIFILEDIALOG_H
