//
// Created by Kurosu Chan on 2023/12/5.
//
#include "app_nvs.h"

etl::expected<common::nvs::punch_step_t, esp_err_t>
app_nvs::get_punch_step() {
  esp_err_t err = ESP_OK;
  using ue_t    = etl::unexpected<esp_err_t>;
  auto handle   = nvs::open_nvs_handle(common::nvs::PARTITION, NVS_READONLY, &err);
  if (err != ESP_OK) {
    auto ue = ue_t{err};
    return {ue};
  }
  common::nvs::punch_step_t step{};
  err = handle->get_item(common::nvs::PUNCH_STEP_KEY, step);
  if (err != ESP_OK) {
    auto ue = ue_t{err};
    return {ue};
  }
  return {step};
}

esp_err_t app_nvs::set_punch_step(common::nvs::punch_step_t step) {
  esp_err_t err = ESP_OK;
  auto handle   = nvs::open_nvs_handle(common::nvs::PARTITION, NVS_READWRITE, &err);
  if (err != ESP_OK) {
    return err;
  }
  err = handle->set_item(common::nvs::PUNCH_STEP_KEY, step);
  return err;
}
