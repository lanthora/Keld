/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_JSON_H
#define KELD_JSON_H

#include "keld/util.h"
#include <nlohmann/json.hpp>
#include <string>

namespace keld {

namespace json {

std::string dump(const nlohmann::json &doc);
nlohmann::json parse(const std::string &msg);

}; // namespace json

}; // namespace keld

#endif