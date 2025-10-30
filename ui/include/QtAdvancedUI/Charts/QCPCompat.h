#pragma once

// Provide global aliases for QCustomPlot types which are defined inside
// the dc_opengl namespace in our bundled QCustomplot.h.
// This keeps existing code that uses un-namespaced types compiling cleanly
// without touching third-party headers.

namespace dc_opengl {
class QCustomPlot;
class QCPGraph;
class QCPBars;
class QCPAxisTickerText;
class QCPPolarAxisAngular;
class QCPPolarAxisRadial;
class QCPPolarGraph;
class QCPScatterStyle;
// Forward-declare nested namespace for aliasing QCP enums/flags
namespace QCP {}
}

using QCustomPlot = dc_opengl::QCustomPlot;
using QCPGraph = dc_opengl::QCPGraph;
using QCPBars = dc_opengl::QCPBars;
using QCPAxisTickerText = dc_opengl::QCPAxisTickerText;
using QCPPolarAxisAngular = dc_opengl::QCPPolarAxisAngular;
using QCPPolarAxisRadial = dc_opengl::QCPPolarAxisRadial;
using QCPPolarGraph = dc_opengl::QCPPolarGraph;
using QCPScatterStyle = dc_opengl::QCPScatterStyle;

// Alias the QCP namespace so code can use QCP::Interaction, PlottingHint, etc.
namespace QCP = dc_opengl::QCP;