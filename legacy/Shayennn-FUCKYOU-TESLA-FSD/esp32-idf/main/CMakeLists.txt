idf_component_register(
  SRCS "esp32_app.cpp"
       "can_fast_path.cpp"
       "can_logger.cpp"
       "wifi_ap.cpp"
       "auth.cpp"
       "web_server.cpp"
       "ota.cpp"
  INCLUDE_DIRS "."
  REQUIRES nvs_flash esp_driver_gpio esp_driver_twai esp_wifi esp_netif esp_http_server mbedtls app_update esp_timer esp_event
)

target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../shared")
target_compile_options(${COMPONENT_LIB} PRIVATE -O3)
