#include <assert.h>
#include <node_api.h>

#include "pv_recorder.h"

napi_value napi_pv_recorder_init(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    int32_t device_index;
    status = napi_get_value_int32(env, args[0], &device_index);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the device index");
        return NULL;
    }

    int32_t frame_length;
    status = napi_get_value_int32(env, args[1], &frame_length);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the frame length");
        return NULL;
    }

    int32_t buffer_size_msec;
    status = napi_get_value_int32(env, args[2], &buffer_size_msec);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the buffer size");
        return NULL;
    }

    bool log_overflow;
    status = napi_get_value_bool(env, args[3], &log_overflow);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the log overflow flag");
        return NULL;
    }

    bool log_silence;
    status = napi_get_value_bool(env, args[3], &log_silence);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the log silence flag");
        return NULL;
    }

    pv_recorder_t *handle = NULL;
    pv_recorder_status_t pv_recorder_status = pv_recorder_init(device_index, frame_length, buffer_size_msec, log_overflow, log_silence, &handle);
    if (pv_recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        handle = NULL;
    }

    napi_value object_js = NULL;
    napi_value handle_js = NULL;
    napi_value status_js = NULL;
    const char *ERROR_MSG = "Unable to allocate memory for the constructed instance of PvRecorder";

    status = napi_create_object(env, &object_js);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                ERROR_MSG);
        return NULL;
    }

    status = napi_create_bigint_uint64(env, ((uint64_t) handle), &handle_js);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                ERROR_MSG);
        return NULL;
    }
    status = napi_set_named_property(env, object_js, "handle", handle_js);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                ERROR_MSG);
        return NULL;
    }
    status = napi_create_int32(env, pv_recorder_status, &status_js);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                ERROR_MSG);
        return NULL;
    }
    status = napi_set_named_property(env, object_js, "status", status_js);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                ERROR_MSG);
        return NULL;
    }

    return object_js;
}

napi_value napi_pv_recorder_delete(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    uint64_t object_id = 0;
    bool lossless = false;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    if ((status != napi_ok) || !lossless) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get the address of the instance of PvRecorder properly");
        return NULL;
    }

    pv_recorder_delete((pv_recorder_t *)(uintptr_t) object_id);
    return NULL;
}

napi_value napi_pv_recorder_start(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    uint64_t object_id = 0;
    bool lossless = false;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    if ((status != napi_ok) || !lossless) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get the address of the instance of PvRecorder properly");
        return NULL;
    }

    pv_recorder_status_t pv_recorder_status = pv_recorder_start((pv_recorder_t *)(uintptr_t) object_id);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the start result");
        return NULL;
    }

    return result;
}

napi_value napi_pv_recorder_stop(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    uint64_t object_id = 0;
    bool lossless = false;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    if ((status != napi_ok) || !lossless) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get the address of the instance of PvRecorder properly");
        return NULL;
    }

    pv_recorder_status_t pv_recorder_status = pv_recorder_stop((pv_recorder_t *)(uintptr_t) object_id);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the stop result");
        return NULL;
    }

    return result;
}

napi_value napi_pv_recorder_read(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    uint64_t object_id = 0;
    bool lossless = false;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    if ((status != napi_ok) || !lossless) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get the address of the instance of PvRecorder properly");
        return NULL;
    }

    napi_typedarray_type arr_type = -1;
    size_t length = 0;
    void *data = NULL;
    napi_value arr_value = NULL;
    size_t offset = 0;
    status = napi_get_typedarray_info(env, args[1], &arr_type, &length, &data, &arr_value, &offset);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Unable to get the input frame");
        return NULL;
    }
    if (arr_type != napi_int16_array) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Invalid type of input pcm buffer. The input frame has to be 'Int16Array'");
        return NULL;
    }
    if (length == 0) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Invalid frame length");
        return NULL;
    }
    if (offset != 0) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_INVALID_ARGUMENT),
                "Invalid shape of input frame");
        return NULL;
    }


    pv_recorder_status_t pv_recorder_status = pv_recorder_read((pv_recorder_t *)(uintptr_t) object_id, (int16_t *) data);

    napi_value result;
    status = napi_create_int32(env, pv_recorder_status, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the read result");
        return NULL;
    }

    return result;
}

napi_value napi_pv_recorder_get_selected_device(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    uint64_t object_id = 0;
    bool lossless = false;
    status = napi_get_value_bigint_uint64(env, args[0], &object_id, &lossless);
    if ((status != napi_ok) || !lossless) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get the address of the instance of PvRecorder properly");
        return NULL;
    }

    napi_value result;
    status = napi_create_string_utf8(env, pv_recorder_get_selected_device((pv_recorder_t *)(uintptr_t) object_id), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the read result");
        return NULL;
    }

    return result;
}

napi_value napi_pv_recorder_get_audio_devices(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to get input arguments");
        return NULL;
    }

    int32_t count = 0;
    char **devices = NULL;
    pv_recorder_status_t pv_recorder_status = pv_recorder_get_audio_devices(&count, &devices);

    if (pv_recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        return NULL;
    }

    napi_value result;
    status = napi_create_array_with_length(env, count, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the devices result");
        return NULL;
    }

    for (int32_t i = 0; i < count; i++) {
        napi_value device_name;
        status = napi_create_string_utf8(env, devices[i], NAPI_AUTO_LENGTH, &device_name);
        if (status != napi_ok) {
            napi_throw_error(
                    env,
                    pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                    "Unable to allocate memory for the device name");
            return NULL;
        }

        status = napi_set_element(env, result, i, device_name);
        if (status != napi_ok) {
            napi_throw_error(
                    env,
                    pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                    "Unable to copy device name to allocated memory");
            return NULL;
        }
    }

    pv_recorder_free_device_list(count, devices);
    return result;
}

napi_value napi_pv_recorder_version(napi_env env, napi_callback_info info) {
    (void)(info);

    napi_value result;
    napi_status status = napi_create_string_utf8(env, pv_recorder_version(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        napi_throw_error(
                env,
                pv_recorder_status_to_string(PV_RECORDER_STATUS_RUNTIME_ERROR),
                "Unable to allocate memory for the version result");
        return NULL;
    }

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