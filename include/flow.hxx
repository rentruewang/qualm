#include <iostream>
#include "algo.hxx"
#include "q_device.hxx"
#include "qft_mapper.hxx"
#include "qft_topo.hxx"
#include "topo.hxx"
#include "util.hxx"

#pragma once

size_t flow(json& conf, std::vector<size_t> assign, bool io);

size_t device_num(json& conf);

size_t topo_num(json& conf);
