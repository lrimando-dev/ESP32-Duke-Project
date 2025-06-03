#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "utils/CLI/cli_interface.h"
#include "utils/CLI/cli_commands.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Duke Project Debugger");
    
    // Initialize NVS (required for CLI history storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize CLI interface
    cli_config_t cli_config = cli_get_default_config();
    cli_config.history_enabled = true;
    
    cli_status_t status = cli_interface_init(&cli_config);
    if (status != CLI_STATUS_OK) {
        ESP_LOGE(TAG, "Failed to initialize CLI interface");
        return;
    }
    
    // Register utility commands
    cli_register_utility_commands();
    
    // Start the CLI interface
    status = cli_interface_start();
    if (status != CLI_STATUS_OK) {
        ESP_LOGE(TAG, "Failed to start CLI interface");
        return;
    }
    
    ESP_LOGI(TAG, "System initialization complete. CLI interface is running.");
    ESP_LOGI(TAG, "Type 'help' to see available commands.");
}