//
// Created by Kurosu Chan on 2023/12/5.
//

#ifndef PUNCHER_APP_NVS_H
#define PUNCHER_APP_NVS_H

#include <nvs_handle.hpp>
#include <etl/expected.h>
#include "common.h"

namespace app_nvs {
etl::expected<common::nvs::punch_step_t, esp_err_t>
get_punch_step();

esp_err_t set_punch_step(common::nvs::punch_step_t step);
}

#endif // PUNCHER_APP_NVS_H
