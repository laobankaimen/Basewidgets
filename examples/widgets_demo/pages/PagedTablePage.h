#ifndef WIDGETS_DEMO_PAGEDTABLEPAGE_H
#define WIDGETS_DEMO_PAGEDTABLEPAGE_H

#include <basewidgets/Core/StyledWidget.h>

class PagedProxyModel;

class PagedTablePage : public basewidgets::StyledWidget {
    Q_OBJECT
public:
    explicit PagedTablePage(QWidget* parent = nullptr);
};

#endif // WIDGETS_DEMO_PAGEDTABLEPAGE_H