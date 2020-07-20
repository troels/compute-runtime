/*
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "level_zero/core/source/device/device.h"
#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/core/test/unit_tests/mock.h"
#include "level_zero/core/test/unit_tests/white_box.h"
#include "level_zero/tools/source/metrics/metric.h"
#include "level_zero/tools/source/metrics/metric_enumeration_imp.h"
#include "level_zero/tools/source/metrics/metric_query_imp.h"

namespace L0 {
namespace ult {

template <>
struct WhiteBox<::L0::MetricGroup> : public ::L0::MetricGroup {
    using BaseClass = ::L0::MetricGroup;
};

template <>
struct WhiteBox<::L0::MetricQuery> : public ::L0::MetricQuery {
    using BaseClass = ::L0::MetricQuery;
};

template <>
struct WhiteBox<::L0::MetricQueryPool> : public ::L0::MetricQueryPool {
    using BaseClass = ::L0::MetricQuery;
};

using MetricGroup = WhiteBox<::L0::MetricGroup>;
using MetricQuery = WhiteBox<::L0::MetricQuery>;
using MetricQueryPool = WhiteBox<::L0::MetricQueryPool>;

using MetricsLibraryApi::ClientData_1_0;
using MetricsLibraryApi::ClientDataLinuxAdapter_1_0;
using MetricsLibraryApi::ClientType_1_0;
using MetricsLibraryApi::CommandBufferData_1_0;
using MetricsLibraryApi::CommandBufferSize_1_0;
using MetricsLibraryApi::ConfigurationActivateData_1_0;
using MetricsLibraryApi::ConfigurationCreateData_1_0;
using MetricsLibraryApi::ConfigurationHandle_1_0;
using MetricsLibraryApi::ContextCreateData_1_0;
using MetricsLibraryApi::ContextHandle_1_0;
using MetricsLibraryApi::GetReportData_1_0;
using MetricsLibraryApi::LinuxAdapterType;
using MetricsLibraryApi::MarkerCreateData_1_0;
using MetricsLibraryApi::MarkerHandle_1_0;
using MetricsLibraryApi::OverrideCreateData_1_0;
using MetricsLibraryApi::OverrideHandle_1_0;
using MetricsLibraryApi::ParameterType;
using MetricsLibraryApi::QueryCreateData_1_0;
using MetricsLibraryApi::QueryHandle_1_0;
using MetricsLibraryApi::StatusCode;
using MetricsLibraryApi::TypedValue_1_0;
using MetricsLibraryApi::ValueType;

struct MockMetricsLibraryApi {

    // Original api functions.
    static StatusCode ML_STDCALL ContextCreate(ClientType_1_0 clientType, ContextCreateData_1_0 *createData, ContextHandle_1_0 *handle);
    static StatusCode ML_STDCALL ContextDelete(const ContextHandle_1_0 handle);
    static StatusCode ML_STDCALL GetParameter(const ParameterType parameter, ValueType *type, TypedValue_1_0 *value);
    static StatusCode ML_STDCALL CommandBufferGet(const CommandBufferData_1_0 *data);
    static StatusCode ML_STDCALL CommandBufferGetSize(const CommandBufferData_1_0 *data, CommandBufferSize_1_0 *size);
    static StatusCode ML_STDCALL QueryCreate(const QueryCreateData_1_0 *createData, QueryHandle_1_0 *handle);
    static StatusCode ML_STDCALL QueryDelete(const QueryHandle_1_0 handle);
    static StatusCode ML_STDCALL OverrideCreate(const OverrideCreateData_1_0 *createData, OverrideHandle_1_0 *handle);
    static StatusCode ML_STDCALL OverrideDelete(const OverrideHandle_1_0 handle);
    static StatusCode ML_STDCALL MarkerCreate(const MarkerCreateData_1_0 *createData, MarkerHandle_1_0 *handle);
    static StatusCode ML_STDCALL MarkerDelete(const MarkerHandle_1_0 handle);
    static StatusCode ML_STDCALL ConfigurationCreate(const ConfigurationCreateData_1_0 *createData, ConfigurationHandle_1_0 *handle);
    static StatusCode ML_STDCALL ConfigurationActivate(const ConfigurationHandle_1_0 handle, const ConfigurationActivateData_1_0 *activateData);
    static StatusCode ML_STDCALL ConfigurationDeactivate(const ConfigurationHandle_1_0 handle);
    static StatusCode ML_STDCALL ConfigurationDelete(const ConfigurationHandle_1_0 handle);
    static StatusCode ML_STDCALL GetData(GetReportData_1_0 *data);

    // Mocked api functions.
    MOCK_METHOD(StatusCode, MockContextCreate, (ClientType_1_0 clientType, ContextCreateData_1_0 *createData, ContextHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockContextDelete, (const ContextHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockGetParameter, (const ParameterType parameter, ValueType *type, TypedValue_1_0 *value));
    MOCK_METHOD(StatusCode, MockCommandBufferGet, (const CommandBufferData_1_0 *data));
    MOCK_METHOD(StatusCode, MockCommandBufferGetSize, (const CommandBufferData_1_0 *data, CommandBufferSize_1_0 *size));
    MOCK_METHOD(StatusCode, MockQueryCreate, (const QueryCreateData_1_0 *createData, QueryHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockQueryDelete, (const QueryHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockOverrideCreate, (const OverrideCreateData_1_0 *createData, OverrideHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockOverrideDelete, (const OverrideHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockMarkerCreate, (const MarkerCreateData_1_0 *createData, MarkerHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockMarkerDelete, (const MarkerHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockConfigurationCreate, (const ConfigurationCreateData_1_0 *createData, ConfigurationHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockConfigurationActivate, (const ConfigurationHandle_1_0 handle, const ConfigurationActivateData_1_0 *activateData));
    MOCK_METHOD(StatusCode, MockConfigurationDeactivate, (const ConfigurationHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockConfigurationDelete, (const ConfigurationHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockGetData, (GetReportData_1_0 * data));
};

template <>
struct Mock<MetricsLibrary> : public MetricsLibrary {

  public:
    Mock(::L0::MetricContext &metricContext);
    ~Mock() override;

    // Api mock enable/disable.
    void setMockedApi(MockMetricsLibraryApi *mockedApi);

    // Mocked metrics library functions.
    MOCK_METHOD(bool, load, (), (override));
    MOCK_METHOD(bool, getContextData, (::L0::Device &, ContextCreateData_1_0 &), (override));
    MOCK_METHOD(bool, getMetricQueryReportSize, (size_t & rawDataSize), (override));

    // Not mocked metrics library functions.
    bool metricsLibraryGetContextData(::L0::Device &device, ContextCreateData_1_0 &contextData) { return MetricsLibrary::getContextData(device, contextData); }

    // Original metrics library implementation used by metric context.
    ::L0::MetricsLibrary *metricsLibrary = nullptr;

    // Mocked metrics library api version.
    // We cannot use a static instance here since the gtest validates memory usage,
    // and mocked functions will stay in memory longer than the test.
    static MockMetricsLibraryApi *g_mockApi;
};

using MetricsDiscovery::IConcurrentGroup_1_5;
using MetricsDiscovery::IInformation_1_0;
using MetricsDiscovery::IMetric_1_0;
using MetricsDiscovery::IMetricsDevice_1_5;
using MetricsDiscovery::IMetricSet_1_0;
using MetricsDiscovery::IMetricSet_1_5;
using MetricsDiscovery::IOverride_1_2;
using MetricsDiscovery::TCompletionCode;
using MetricsDiscovery::TConcurrentGroupParams_1_0;
using MetricsDiscovery::TGlobalSymbol_1_0;
using MetricsDiscovery::TMetricParams_1_0;
using MetricsDiscovery::TMetricsDeviceParams_1_2;
using MetricsDiscovery::TMetricSetParams_1_4;
using MetricsDiscovery::TSamplingType;
using MetricsDiscovery::TTypedValue_1_0;

struct MockMetricsDiscoveryApi {

    // Original api functions.
    static TCompletionCode MD_STDCALL OpenMetricsDevice(IMetricsDevice_1_5 **device);
    static TCompletionCode MD_STDCALL OpenMetricsDeviceFromFile(const char *fileName, void *openParams, IMetricsDevice_1_5 **device);
    static TCompletionCode MD_STDCALL CloseMetricsDevice(IMetricsDevice_1_5 *device);
    static TCompletionCode MD_STDCALL SaveMetricsDeviceToFile(const char *fileName, void *saveParams, IMetricsDevice_1_5 *device);

    // Mocked api functions.
    MOCK_METHOD(TCompletionCode, MockOpenMetricsDevice, (IMetricsDevice_1_5 **));
    MOCK_METHOD(TCompletionCode, MockOpenMetricsDeviceFromFile, (const char *, void *, IMetricsDevice_1_5 **));
    MOCK_METHOD(TCompletionCode, MockCloseMetricsDevice, (IMetricsDevice_1_5 *));
    MOCK_METHOD(TCompletionCode, MockSaveMetricsDeviceToFile, (const char *, void *, IMetricsDevice_1_5 *));
};

template <>
class Mock<IMetricsDevice_1_5> : public IMetricsDevice_1_5 {
  public:
    Mock(){};

    MOCK_METHOD(TMetricsDeviceParams_1_2 *, GetParams, (), (override));
    MOCK_METHOD(IOverride_1_2 *, GetOverride, (uint32_t index), (override));
    MOCK_METHOD(IOverride_1_2 *, GetOverrideByName, (const char *symbolName), (override));
    MOCK_METHOD(IConcurrentGroup_1_5 *, GetConcurrentGroup, (uint32_t index), (override));
    MOCK_METHOD(TGlobalSymbol_1_0 *, GetGlobalSymbol, (uint32_t index), (override));
    MOCK_METHOD(TTypedValue_1_0 *, GetGlobalSymbolValueByName, (const char *name), (override));
    MOCK_METHOD(TCompletionCode, GetLastError, (), (override));
    MOCK_METHOD(TCompletionCode, GetGpuCpuTimestamps, (uint64_t * gpuTimestampNs, uint64_t *cpuTimestampNs, uint32_t *cpuId), (override));
};

template <>
class Mock<IConcurrentGroup_1_5> : public IConcurrentGroup_1_5 {
  public:
    Mock(){};

    MOCK_METHOD(IMetricSet_1_5 *, GetMetricSet, (uint32_t index), (override));
    MOCK_METHOD(TConcurrentGroupParams_1_0 *, GetParams, (), (override));
    MOCK_METHOD(TCompletionCode, OpenIoStream, (IMetricSet_1_0 * metricSet, uint32_t processId, uint32_t *nsTimerPeriod, uint32_t *oaBufferSize), (override));
    MOCK_METHOD(TCompletionCode, ReadIoStream, (uint32_t * reportsCount, char *reportData, uint32_t readFlags), (override));
    MOCK_METHOD(TCompletionCode, CloseIoStream, (), (override));
    MOCK_METHOD(TCompletionCode, WaitForReports, (uint32_t milliseconds), (override));
    MOCK_METHOD(TCompletionCode, SetIoStreamSamplingType, (TSamplingType type), (override));
    MOCK_METHOD(IInformation_1_0 *, GetIoMeasurementInformation, (uint32_t index), (override));
    MOCK_METHOD(IInformation_1_0 *, GetIoGpuContextInformation, (uint32_t index), (override));
};

template <>
class Mock<IMetricSet_1_5> : public IMetricSet_1_5 {
  public:
    Mock(){};

    MOCK_METHOD(TMetricSetParams_1_4 *, GetParams, (), (override));
    MOCK_METHOD(IMetric_1_0 *, GetMetric, (uint32_t index), (override));
    MOCK_METHOD(IInformation_1_0 *, GetInformation, (uint32_t index), (override));
    MOCK_METHOD(TCompletionCode, Activate, (), (override));
    MOCK_METHOD(TCompletionCode, Deactivate, (), (override));
    MOCK_METHOD(TCompletionCode, SetApiFiltering, (uint32_t apiMask), (override));
    MOCK_METHOD(TCompletionCode, CalculateMetrics, (const unsigned char *rawData, uint32_t rawDataSize, TTypedValue_1_0 *out, uint32_t outSize, uint32_t *outReportCount, bool enableContextFiltering), (override));
    MOCK_METHOD(TCompletionCode, CalculateIoMeasurementInformation, (TTypedValue_1_0 * out, uint32_t outSize), (override));
    MOCK_METHOD(IMetricSet_1_5 *, GetComplementaryMetricSet, (uint32_t index), (override));
    MOCK_METHOD(TCompletionCode, CalculateMetrics, (const unsigned char *rawData, uint32_t rawDataSize, TTypedValue_1_0 *out, uint32_t outSize, uint32_t *outReportCount, TTypedValue_1_0 *outMaxValues, uint32_t outMaxValuesSize), (override));
};

template <>
class Mock<IMetric_1_0> : public IMetric_1_0 {
  public:
    Mock(){};

    MOCK_METHOD(TMetricParams_1_0 *, GetParams, (), (override));
};

template <>
struct Mock<MetricEnumeration> : public MetricEnumeration {
    Mock(::L0::MetricContext &metricContext);
    ~Mock() override;

    // Api mock enable/disable.
    void setMockedApi(MockMetricsDiscoveryApi *mockedApi);

    // Mock metric enumeration functions.
    MOCK_METHOD(bool, isInitialized, (), (override));
    MOCK_METHOD(ze_result_t, loadMetricsDiscovery, (), (override));

    // Mock metrics discovery api.
    static MockMetricsDiscoveryApi *g_mockApi;

    // Original metric enumeration obtained from metric context.
    ::L0::MetricEnumeration *metricEnumeration = nullptr;
};

template <>
struct Mock<MetricGroup> : public MetricGroup {
    Mock() {}

    MOCK_METHOD(ze_result_t, getMetric, (uint32_t *, zet_metric_handle_t *), (override));
    MOCK_METHOD(ze_result_t, calculateMetricValues, (size_t, const uint8_t *, uint32_t *, zet_typed_value_t *), (override));
    MOCK_METHOD(ze_result_t, getProperties, (zet_metric_group_properties_t * properties), (override));
    MOCK_METHOD(uint32_t, getRawReportSize, (), (override));
    MOCK_METHOD(bool, activate, (), (override));
    MOCK_METHOD(bool, deactivate, (), (override));
    MOCK_METHOD(ze_result_t, waitForReports, (const uint32_t), (override));
    MOCK_METHOD(ze_result_t, openIoStream, (uint32_t &, uint32_t &), (override));
    MOCK_METHOD(ze_result_t, readIoStream, (uint32_t &, uint8_t &), (override));
    MOCK_METHOD(ze_result_t, closeIoStream, (), (override));
};

template <>
struct Mock<MetricQueryPool> : public MetricQueryPool {
    Mock();
    ~Mock() override;

    MOCK_METHOD(ze_result_t, createMetricQuery, (uint32_t, zet_metric_query_handle_t *), (override));
    MOCK_METHOD(ze_result_t, destroy, (), (override));
};

template <>
struct Mock<MetricQuery> : public MetricQuery {
    Mock();
    ~Mock() override;

    MOCK_METHOD(ze_result_t, getData, (size_t *, uint8_t *), (override));
    MOCK_METHOD(ze_result_t, reset, (), (override));
    MOCK_METHOD(ze_result_t, destroy, (), (override));
};

class MetricDeviceFixture : public DeviceFixture {

  protected:
    void SetUp() override;
    void TearDown() override;

  public:
    // Mocked objects.
    std::unique_ptr<Mock<MetricEnumeration>> mockMetricEnumeration = nullptr;
    std::unique_ptr<Mock<MetricsLibrary>> mockMetricsLibrary = nullptr;

    // Mocked metrics library/discovery APIs.
    MockMetricsLibraryApi mockMetricsLibraryApi = {};
    MockMetricsDiscoveryApi mockMetricsDiscoveryApi = {};

    // Metrics discovery device
    Mock<IMetricsDevice_1_5> metricsDevice;
    MetricsDiscovery::TMetricsDeviceParams_1_2 metricsDeviceParams = {};
};

} // namespace ult
} // namespace L0
