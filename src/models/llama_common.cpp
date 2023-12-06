#include "../generators.h"
#include "../search.h"
#include "llama_common.h"
#include "debugging.h"
#include <iostream>

namespace Generators {

Llama_Model::Llama_Model(OrtEnv& ort_env, Config& config, OrtSessionOptions& session_options)
    : config_{config} {
  session_decoder_ = OrtSession::Create(ort_env, (config.config_path / config.model_decoder).c_str(), &session_options);
  InitModelParams();
}

void Llama_Model::InitModelParams() {
  // We could use this to determine the vocabulary size and if the logits has a width of 1
  auto logits_type_info = session_decoder_->GetOutputTypeInfo(0);
  auto& logits_tensor_info = logits_type_info->GetTensorTypeAndShapeInfo();
  auto logits_shape = logits_tensor_info.GetShape();
  assert(logits_shape.size() == 3);
  logits_uses_seq_len_ = logits_shape[1] == -1;
  vocab_size_ = static_cast<int>(logits_shape[2]);
  layer_count_ = (static_cast<int>(session_decoder_->GetOutputCount()) - 1) / 2;
  score_type_ = logits_tensor_info.GetElementType();

  auto past_shape = session_decoder_->GetInputTypeInfo(3)->GetTensorTypeAndShapeInfo().GetShape();
  head_count_ = static_cast<int>(past_shape[1]);
  hidden_size_ = static_cast<int>(past_shape[3]);
}

}  // namespace Generators