#pragma once

#include <Eigen/Dense>
#include <memory>
#include <robot_design/sim.h>
#include <robot_design/torch_utils.h>
#include <robot_design/types.h>
#include <torch/torch.h>
#include <vector>

namespace robot_design {

using Eigen::Ref;

class ValueEstimator {
public:
  virtual ~ValueEstimator() {}
  virtual int getObservationSize() const = 0;
  virtual void getObservation(
      const Simulation &sim, Ref<VectorX> obs) const = 0;
  virtual void estimateValue(
      const MatrixX &obs, Ref<VectorX> value_est) const = 0;
  virtual void train(const MatrixX &obs, const Ref<const VectorX> &value) = 0;
};

class NullValueEstimator : public ValueEstimator {
public:
  NullValueEstimator() {}
  virtual ~NullValueEstimator() {}
  NullValueEstimator(const NullValueEstimator &other) = delete;
  NullValueEstimator &operator=(const NullValueEstimator &other) = delete;
  virtual int getObservationSize() const override { return 0; }
  virtual void getObservation(
      const Simulation &sim, Ref<VectorX> obs) const override {}
  virtual void estimateValue(
      const MatrixX &obs, Ref<VectorX> value_est) const override {
    value_est = VectorX::Zero(obs.cols());
  }
  virtual void train(
      const MatrixX &obs, const Ref<const VectorX> &value) override {}
};

struct FCValueNet : torch::nn::Module {
  FCValueNet(int obs_size, int hidden_layer_count, int hidden_layer_size);
  torch::Tensor forward(torch::Tensor x);

  std::vector<torch::nn::Linear> layers_;
};

class FCValueEstimator : public ValueEstimator {
public:
  FCValueEstimator(const Simulation &sim, Index robot_idx,
                   const torch::Device &device, int batch_size = 32,
                   int epoch_count = 4, int ensemble_size = 6);
  virtual ~FCValueEstimator() {}
  FCValueEstimator(const FCValueEstimator &other) = delete;
  FCValueEstimator &operator=(const FCValueEstimator &other) = delete;
  virtual int getObservationSize() const override;
  virtual void getObservation(
      const Simulation &sim, Ref<VectorX> obs) const override;
  virtual void estimateValue(
      const MatrixX &obs, Ref<VectorX> value_est) const override;
  virtual void train(
      const MatrixX &obs, const Ref<const VectorX> &value) override;

private:
  int robot_idx_;
  torch::Device device_;
  int batch_size_;
  int epoch_count_;
  int dof_count_;
  std::vector<std::shared_ptr<FCValueNet>> nets_;
  std::vector<std::shared_ptr<torch::optim::Adam>> optimizers_;
};

} // namespace robot_design
