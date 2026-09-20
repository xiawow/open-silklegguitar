# main 组件
# REQUIRES: bt(Bluedroid) nvs_flash(配置存储) driver(GPIO/I2S) esp_adc(ADC1 DMA)
idf_component_register(
    SRCS
        "app_main.cpp"
        "BinaryBuffer.cpp"
        "JsInterface.cpp"
        "SilkLegGuitar.cpp"
        "UserConfigure.cpp"
        "PickupPlugin.cpp"
        "FFT.cpp"
        "RecordTemporarily.cpp"
        "driver/BluetoothCustomService.cpp"
        "driver/BluetoothManager.cpp"
        "driver/A2DP.cpp"
        "driver/I2S.cpp"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash driver esp_adc
)
