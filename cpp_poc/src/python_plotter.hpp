#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>

namespace py = pybind11;

void create_scatter_plot(const std::vector<double>& x, const std::vector<double>& y, const std::string& title);