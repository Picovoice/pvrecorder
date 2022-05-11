#include <assert.h>
#include <node_api.h>
#include <stdio.h>

#include "pv_recorder.h"

napi_value napi_pv_recorder_init(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    int32_t device_index;
    status = napi_get_value_int32(env, args[0], &device_index);
    assert(status == napi_ok);

    int32_t frame_length;
    status = napi_get_value_int32(env, args[1], &frame_length);
    assert(status == napi_ok);

    int32_t buffer_size_msec;
    status = napi_get_value_int32(env, args[2], &buffer_size_msec);
    assert(status == napi_ok);

    bool log_overflow;
    status = napi_get_value_bool(env, args[3], &log_overflow);
    assert(status == napi_ok);

    bool log_silence;
    status = napi_get_value_bool(env, args[3], &log_silence);
    assert(status == napi_ok);

    pv_recorder_t *handle = NULL;
    pv_recorder_status_t pv_recorder_status = pv_recorder_init(device_index, frame_length, buffer_size_msec, log_overflow, log_silence, &handle);
    if (pv_recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        handle = NULL;
    }

    napi_value result;
    uint64_t object_id_and_status = (((uint64_t)(uintptr_t) handle) * 10) + pv_recorder_status;
    status = napi_create_bigint_uint64(env, (uint64_t) object_id_and_status, &result);
    assert(status == napi_ok);

    return result;
}

napi_value napi_pv_recorder_delete(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    uint64_t object_id;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    assert(status == napi_ok);
    assert(lossless);

    pv_recorder_delete((pv_recorder_t *)(uintptr_t) object_id);

    return NULL;
}

napi_value napi_pv_recorder_start(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    uint64_t object_id;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    assert(status == napi_ok);
    assert(lossless);

    pv_recorder_status_t pv_recorder_status = pv_recorder_start((pv_recorder_t *)(uintptr_t) object_id);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    assert(status == napi_ok);

    return result;
}

napi_value napi_pv_recorder_stop(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    uint64_t object_id;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    assert(status == napi_ok);
    assert(lossless);

    pv_recorder_status_t pv_recorder_status = pv_recorder_stop((pv_recorder_t *)(uintptr_t) object_id);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    assert(status == napi_ok);

    return result;
}

napi_value napi_pv_recorder_read(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    uint64_t object_id;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    assert(status == napi_ok);
    assert(lossless);

    napi_typedarray_type arr_type;
    size_t length;
    void *data;
    napi_value arr_value;
    size_t offset;
    status = napi_get_typedarray_info(env, args[1], &arr_type, &length, &data, &arr_value, &offset);
    assert(status == napi_ok);
    assert(arr_type == napi_int16_array);
    assert(length > 0);
    assert(offset == 0);

    pv_recorder_status_t pv_recorder_status = pv_recorder_read((pv_recorder_t *)(uintptr_t) object_id, (int16_t *) data);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    assert(status == napi_ok);

    return result;
}

napi_value napi_pv_recorder_get_selected_device(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    assert(status == napi_ok);

    uint64_t object_id;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    assert(status == napi_ok);
    assert(lossless);

    napi_value result;
    status = napi_create_string_utf8(env, pv_recorder_get_selected_device((pv_recorder_t *)(uintptr_t) object_id), NAPI_AUTO_LENGTH, &result);
    assert(status == napi_ok);

    return result;
}

napi_value napi_pv_recorder_get_audio_devices(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);
    assert(status == napi_ok);

    int32_t count = 0;
    char **devices = NULL;
    pv_recorder_status_t pv_recorder_status = pv_recorder_get_audio_devices(&count, &devices);
    assert(devices != NULL);

    if (pv_recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        return NULL;
    }

    napi_value result;
    status = napi_create_array_with_length(env, count, &result);
    assert(status == napi_ok);

    for (int32_t i = 0; i < count; i++) {
        napi_value device_name;
        status = napi_create_string_utf8(env, devices[i], NAPI_AUTO_LENGTH, &device_name);
        assert(status == napi_ok);

        status = napi_set_element(env, result, i, device_name);
        assert(status == napi_ok);
    }

    pv_recorder_free_device_list(count, devices);

    return result;
}

napi_value napi_pv_recorder_version(napi_env env, napi_callback_info info) {
    (void)(info);

    napi_value result;
    napi_status status = napi_create_string_utf8(env, pv_recorder_version(), NAPI_AUTO_LENGTH, &result);
    assert(status == napi_ok);

    return result;
}

#define DECLARE_NAPI_METHOD(name, func) (napi_property_descriptor){ name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc = DECLARE_NAPI_METHOD("init", napi_pv_recorder_init);
    napi_status status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("delete", napi_pv_recorder_delete);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("start", napi_pv_recorder_start);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("stop", napi_pv_recorder_stop);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("read", napi_pv_recorder_read);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("get_selected_device", napi_pv_recorder_get_selected_device);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("get_audio_devices", napi_pv_recorder_get_audio_devices);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    desc = DECLARE_NAPI_METHOD("version", napi_pv_recorder_version);
    status = napi_define_properties(env, exports, 1, &desc);
    assert(status == napi_ok);

    return exports;
}

NAPI_MODULE(NODE_GYB_MODULE_NAME, Init)