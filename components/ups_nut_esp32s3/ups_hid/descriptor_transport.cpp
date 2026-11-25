#include "transport_esp32.h"

#ifdef USE_ESP32
namespace esphome { namespace ups_hid {

uint8_t Esp32UsbTransport::get_device_string_index(uint8_t kind) const {
  const usb_device_desc_t *descriptor = nullptr;
  if (!device_.dev_hdl || usb_host_get_device_descriptor(device_.dev_hdl, &descriptor) != ESP_OK)
    return 0;
  if (kind == 1) return descriptor->iManufacturer;
  if (kind == 2) return descriptor->iProduct;
  if (kind == 3) return descriptor->iSerialNumber;
  return 0;
}

esp_err_t Esp32UsbTransport::get_report_descriptor(std::vector<uint8_t> &result) {
  result.clear();
  const usb_config_desc_t *config = nullptr;
  if (!device_.dev_hdl || usb_host_get_active_config_descriptor(device_.dev_hdl, &config) != ESP_OK)
    return ESP_ERR_INVALID_STATE;
  const auto *raw = reinterpret_cast<const uint8_t *>(config);
  uint16_t length = 0;
  bool selected = false;
  for (size_t pos = 0; pos + 2 <= config->wTotalLength;) {
    uint8_t size = raw[pos], type = raw[pos + 1];
    if (size < 2 || pos + size > config->wTotalLength) return ESP_ERR_INVALID_RESPONSE;
    if (type == 4 && size >= 9) selected = raw[pos + 2] == device_.interface_num;
    if (selected && type == 0x21 && size >= 9) {
      for (unsigned i = 0; i < raw[pos + 5] && 6 + i * 3 + 2 < size; ++i) {
        size_t entry = pos + 6 + i * 3;
        if (raw[entry] == 0x22) length = raw[entry + 1] | uint16_t(raw[entry + 2]) << 8;
      }
    }
    pos += size;
  }
  if (!length || length > 4096) return ESP_ERR_INVALID_SIZE;
  usb_transfer_t *transfer = nullptr;
  esp_err_t ret = usb_host_transfer_alloc(sizeof(usb_setup_packet_t) + length, 0, &transfer);
  if (ret != ESP_OK) return ret;
  struct Context {
    SemaphoreHandle_t done{xSemaphoreCreateBinary()};
    esp_err_t result{ESP_FAIL};
    std::vector<uint8_t> bytes;
    ~Context() { if (done) vSemaphoreDelete(done); }
  };
  auto context = std::make_shared<Context>();
  if (!context->done) { usb_host_transfer_free(transfer); return ESP_ERR_NO_MEM; }
  auto *setup = reinterpret_cast<usb_setup_packet_t *>(transfer->data_buffer);
  *setup = {};
  setup->bmRequestType = 0x81;  // Standard GET_DESCRIPTOR, interface recipient.
  setup->bRequest = 6;
  setup->wValue = 0x2200;
  setup->wIndex = device_.interface_num;
  setup->wLength = length;
  transfer->device_handle = device_.dev_hdl;
  transfer->bEndpointAddress = 0;
  transfer->num_bytes = sizeof(usb_setup_packet_t) + length;
  transfer->timeout_ms = 3000;
  transfer->context = new std::shared_ptr<Context>(context);
  transfer->callback = [](usb_transfer_t *completed) {
    auto *owner = static_cast<std::shared_ptr<Context> *>(completed->context);
    auto context = *owner;
    delete owner;
    if (completed->status == USB_TRANSFER_STATUS_COMPLETED &&
        completed->actual_num_bytes > int(sizeof(usb_setup_packet_t))) {
      context->bytes.assign(completed->data_buffer + sizeof(usb_setup_packet_t),
                            completed->data_buffer + completed->actual_num_bytes);
      context->result = ESP_OK;
    }
    // The transfer and its context stay valid even if the waiting caller times out.
    xSemaphoreGive(context->done);
    usb_host_transfer_free(completed);
  };
  ret = usb_host_transfer_submit_control(device_.client_hdl, transfer);
  if (ret != ESP_OK) {
    delete static_cast<std::shared_ptr<Context> *>(transfer->context);
    usb_host_transfer_free(transfer);
    return ret;
  }
  if (xSemaphoreTake(context->done, pdMS_TO_TICKS(5000)) != pdTRUE) return ESP_ERR_TIMEOUT;
  if (context->result != ESP_OK || context->bytes.size() != length) return ESP_ERR_INVALID_RESPONSE;
  result = context->bytes;
  return ESP_OK;
}

}}
#endif
