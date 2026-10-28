#pragma once
// widgets_demo 页面顺序的单一入口。main.cpp 按相同顺序创建页面，导航与检查复用这些 ID。
namespace DemoPage {
enum Index { Inputs, Dialogs, Common, Text, Acoustic, Direction, Monitoring, Gauges, Records, Dashboard, Count };
}
