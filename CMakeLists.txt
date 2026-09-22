# SPDX-License-Identifier: MIT

target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_RF_POWER app PRIVATE
  src/behaviors/behavior_rf_power.c
)

# HALMAX_USB_STUDIO_V1_3_1
if(CONFIG_HALMAX_STUDIO_USB_AUTO_SELECT)
  if(NOT CONFIG_ZMK_STUDIO OR NOT CONFIG_ZMK_STUDIO_RPC
      OR NOT CONFIG_ZMK_USB OR NOT CONFIG_ZMK_SPLIT_ROLE_CENTRAL
      OR NOT CONFIG_ZMK_STUDIO_TRANSPORT_UART)
    message(FATAL_ERROR "Halmax: central Studio + USB UART transport is required")
  endif()
  if(CONFIG_ZMK_STUDIO_LOCKING OR CONFIG_ZMK_STUDIO_LOCK_ON_DISCONNECT
      OR CONFIG_ZMK_STUDIO_TRANSPORT_BLE)
    message(FATAL_ERROR "Halmax: Studio must be unlocked and USB-only")
  endif()
  # Select the declared API from the actual checked-out ZMK headers. This
  # handles the endpoint API rename without assuming a header from another
  # revision. Unsupported APIs fail before the application is compiled.
  file(READ "${APPLICATION_SOURCE_DIR}/include/zmk/endpoints.h" _halmax_ep_header)
  if(_halmax_ep_header MATCHES "int[ \t\r\n]+zmk_endpoint_set_preferred_transport[ \t\r\n]*\\(")
    set(_halmax_usb_select zmk_endpoint_set_preferred_transport)
  elseif(_halmax_ep_header MATCHES "int[ \t\r\n]+zmk_endpoints_select_transport[ \t\r\n]*\\(")
    set(_halmax_usb_select zmk_endpoints_select_transport)
  else()
    message(FATAL_ERROR "Halmax: no supported USB endpoint selection API was found")
  endif()
  target_compile_definitions(app PRIVATE HALMAX_SELECT_USB_TRANSPORT=${_halmax_usb_select})
  target_sources(app PRIVATE src/halmax_studio_usb_auto.c)
  message(STATUS "Halmax Studio: USB RPC / locking OFF / auto USB selection ON")
endif()
