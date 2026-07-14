#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/PushButton.h>

int main(int argc, char* argv[])
{
    basewidgets::StyleManagerAdapter::EnableHighDpi();

    QApplication app(argc, argv);
    basewidgets::StyleManagerAdapter::initialize();

    QWidget window;
    window.setWindowTitle("BaseWidgets Package Consumer");

    auto* layout = new QVBoxLayout(&window);
    auto* label = new basewidgets::Label("Loaded from find_package(BaseWidgets)");
    auto* button = new basewidgets::PushButton("Package consumer demo");

    layout->addWidget(label);
    layout->addWidget(button);

    window.resize(420, 160);
    window.show();
    return app.exec();
}
