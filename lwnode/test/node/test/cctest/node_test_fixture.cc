#include "node_test_fixture.h"

ArrayBufferUniquePtr NodeZeroIsolateTestFixture::allocator{nullptr, nullptr};
uv_loop_t NodeZeroIsolateTestFixture::current_loop;
NodePlatformUniquePtr NodeZeroIsolateTestFixture::platform;
TracingAgentUniquePtr NodeZeroIsolateTestFixture::tracing_agent;
bool NodeZeroIsolateTestFixture::node_initialized = false;
bool NodeZeroIsolateTestFixture::is_trace_call_enabled_ = false;
