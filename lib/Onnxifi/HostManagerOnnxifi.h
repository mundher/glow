/**
 * Copyright (c) Glow Contributors. See CONTRIBUTORS file.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef GLOW_ONNXIFI_HOSTMANAGERONNXIFI_H
#define GLOW_ONNXIFI_HOSTMANAGERONNXIFI_H

#include "Base.h"

#include "glow/Runtime/HostManager/HostManager.h"

namespace glow {
namespace onnxifi {

class HostManagerBackend : public Backend {
public:
  /// Create Glow ONNXIFI backend identifier using HostManager with the
  /// given Glow backend \p kindName, whether to use onnx or caffe2 for models
  /// (\p useOnnx).
  HostManagerBackend(std::shared_ptr<runtime::HostManager> hostManager,
                     llvm::StringRef backendName, bool useOnnx)
      : Backend(backendName, useOnnx), hostManager_(hostManager) {}

  void runNetwork(const Graph *graph, std::unique_ptr<ExecutionContext> context,
                  runtime::ResultCBTy callback, uint64_t priority = 0) override;

  onnxStatus addNetwork(std::unique_ptr<Module> module);

  onnxStatus removeNetwork(const Graph *graph) override;

  // \returns a unique_ptr to a new HostManager for the given Backend \p
  // backendName.
  static std::unique_ptr<runtime::HostManager>
  createHostManager(llvm::StringRef backendName);

private:
  std::shared_ptr<runtime::HostManager> hostManager_;
};

class HostManagerGraph : public Graph {
public:
  using Graph::Graph;

  ~HostManagerGraph() override;

  /// \returns a globally unique graph id.
  static size_t makeUniqueGraphId();

  /// Init Glow graph based on the ONNX model \p onnxModel and
  /// static trained weights \p weightDescriptors. Weights can be read in later
  /// by a \p deferedBlobReader.
  onnxStatus initGraph(const void *onnxModel, size_t onnxModelSize,
                       uint32_t weightCount,
                       const onnxTensorDescriptorV1 *weightDescriptors,
                       void *deferedBlobReader) override;

  /// Async run HostManagerGraph with the given ExecutionContext \p ctx then
  /// signal \p outputEvent when done. \p phNameToOnnxTensorOutputs is a mapping
  /// that is generated by the base class Graph and should be used to map
  /// copy output placeholder tensors back to the given onnxifi tensors.
  onnxStatus run(std::unique_ptr<ExecutionContext> ctx, EventPtr outputEvent,
                 onnxTraceEventList *traceEvents) override;

  /// \returns the unique string name of the HostManagerGraph that the
  /// underlying HostManagerGraph uses to identify this network.
  const std::string &getName() const { return netName_; }

private:
  std::string netName_;
};

} // namespace onnxifi
} // namespace glow

#endif // GLOW_ONNXIFI_HOSTMANAGERONNXIFI_H
